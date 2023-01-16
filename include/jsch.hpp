#ifndef __JSCH__
#define __JSCH__

#include <iostream>
#include <list>
#include "ccqueue.hpp"
#include <pthread.h>
#include <fstream>


namespace jsch{



    class Future {


        public:

            pthread_mutex_t* mutex;
            pthread_cond_t* cond;
            bool* complete;

            void done(){
                pthread_mutex_lock(mutex);
                    *complete = true;
                    pthread_cond_broadcast(cond);
                pthread_mutex_unlock(mutex);
            }

            Future(): mutex(new pthread_mutex_t), cond(new pthread_cond_t), complete(new bool(false)){
                pthread_mutex_init(mutex,NULL);
                pthread_cond_init(cond,NULL);
            }

            Future(pthread_mutex_t* mutex,pthread_cond_t* cond,bool* complete) : mutex(mutex),cond(cond) , complete(complete)  {}
        
        protected:

            Future(bool* complete) : mutex(new pthread_mutex_t),cond(new pthread_cond_t),complete(complete) {
                pthread_mutex_init(mutex,NULL);
                pthread_cond_init(cond,NULL);

            }

            virtual bool checkComplete() {
                return *complete;
            }


        public:

            virtual ~Future(){
                pthread_mutex_destroy(mutex);
                pthread_cond_destroy(cond);

                delete mutex;
                delete cond;
                delete complete;
             }

            virtual void wait(){
                pthread_mutex_lock(mutex);
                    while (!checkComplete()) pthread_cond_wait(cond,mutex);
                pthread_mutex_unlock(mutex); 

                delete this;
            }
    };

    class MultiFuture : public Future {

        private:

            static const size_t MAX_FUTURES = 256;
            
        public:

            size_t totalFutures = 0;

            MultiFuture(): Future(new bool[MAX_FUTURES]) {
                for (size_t i = 0 ; i < MAX_FUTURES; i++) complete[i] = false;
            }

            virtual ~MultiFuture() override{
                delete[] complete;
                this->complete = NULL;
            }

        protected:
            virtual bool checkComplete() override{
                for (size_t i = 0; i < totalFutures; i++){
                    if (!complete[i]) return false;
                }
                return true;
            }


    };

    class Supervisor {
        private:
            pthread_mutex_t* mutex;
            pthread_cond_t* cond;
            bool* complete;
        public:
            Supervisor() : mutex(NULL),cond(NULL),complete(NULL) {}
            Supervisor(pthread_mutex_t* mutex,pthread_cond_t* cond,bool* complete) : mutex(mutex),cond(cond),complete(complete) {}
            Supervisor(const Supervisor& s): mutex(s.mutex),cond(s.cond),complete(s.complete) {}

            void checkOut() {

                if (mutex == NULL) return;

                pthread_mutex_lock(mutex);
                    *complete = true;
                    pthread_cond_broadcast(cond);
                pthread_mutex_unlock(mutex);
            }
    };



    class Job{
        public:

            virtual void* execute() = 0;
            virtual ~Job() = default;

            virtual Future* enableFuture() = 0;
            virtual void gatherFutures(pthread_mutex_t* mutex,pthread_cond_t* cond,bool* complete,size_t& totalJobs) = 0;


    };

    class LinkedJobWrapper : public Job {
        private:
            Job* job;
            LinkedJobWrapper* next;
        public:
            LinkedJobWrapper(Job* job,LinkedJobWrapper* next = NULL) : job(job),next(next) {}

            LinkedJobWrapper* getNext() {
                return next;
            }

            void setNext(LinkedJobWrapper* next){
                this->next = next;
            }

            virtual void* execute() override {
                job->execute();
                return NULL;
            }

            virtual ~LinkedJobWrapper() override{
                delete job;
            }

            virtual Future* enableFuture() override {
                assert(false && "enableFuture LinkedJobWrapper");
            }

            virtual void gatherFutures(pthread_mutex_t* mutex, pthread_cond_t* cond, bool* complete, size_t& totalJobs){
                job->gatherFutures(mutex,cond,complete,totalJobs);
            }

    };

    template<typename S>
    class JobSpecification : public Job {


        private:

            S spec;
            Supervisor supervisor;
        
        public:
            /*Universal reference*/
            JobSpecification(S&& spec): spec(spec){}

            void* execute() override {
                spec();
                supervisor.checkOut();

                return NULL;
            }

            virtual Future* enableFuture() override {
                Future* future = new Future();
                this->supervisor = Supervisor(future->mutex,future->cond,future->complete);
                return future;
            }

            virtual void gatherFutures(pthread_mutex_t* mutex,pthread_cond_t* cond,bool* complete,size_t& totalFutures) override{
                this->supervisor = Supervisor(mutex,cond,complete + totalFutures++);
            }



            virtual ~JobSpecification() override = default;

    };

    template<typename T>
    JobSpecification<T>* make_job(T&& spec){
        return new JobSpecification<T>(std::forward<T>(spec)); 
    }

    
    typedef void* (*workPtr)(void*);


    class JobScheduler{

            private:
                
                size_t submitted;
                /*Jobs completed by workers */
                size_t* completed;
                ccqueue<Job*> queue;
                pthread_t* workers;
                /*Total workers*/
                size_t total;

                /*Workers that are currently executing a job*/
                bool* active;
                /*Tells workers to quit */
                bool _stopped = false;
                /*No new jobs can be added*/
                bool _blocked = false;
                /*Method used by workers */
                void* work();
                /*Mutex and cond used for wait */
                pthread_mutex_t waitMutex;
                pthread_cond_t waitCond;
                /*Returns whether there are currently active workers*/
                bool onVacation() const;
                /*Used by worker to find its serialId*/
                size_t selfSerialId() const;
                size_t jobsCompleted() const;
                

            public:
                
                JobScheduler(size_t workersCount);  
                ~JobScheduler();
                /*The job scheduler does not accept new jobs and waits for workers to finish*/
                void submitJob(Job* job);
                Future* submitJobWithFuture(Job* job);
                size_t workersCount() const;
            
            public :


                class JobSequence : public Job {

                friend JobScheduler;

                private:
                    Job* job;
                    JobScheduler& jobScheduler;
                    JobSequence* next = NULL;
                protected:
                    JobSequence(Job* job,JobScheduler& jobScheduler ) : job(job), jobScheduler(jobScheduler) {}
                    JobSequence(JobScheduler& jobScheduler) : job(NULL),jobScheduler(jobScheduler) {}
                public:

                    virtual void* execute(){
                        job->execute();
                        if( next != NULL) jobScheduler.submitJob(next);

                        /*Find better return value*/
                        return NULL;
                    }

                    virtual Future* enableFuture() override {
                        JobSequence* trav = this;
                        while (trav->next != NULL) trav = trav -> next;

                        return trav->job->enableFuture();
                        
                        

                    }

                    virtual void gatherFutures(pthread_mutex_t* mutex,pthread_cond_t* cond,bool* complete,size_t& totalJobs) override {

                        JobSequence* trav = this;
                        while (trav->next != NULL) trav = trav -> next;

                        /*In order not to call JobSequence::enableFuture again*/ /*Stupid rich*/
                        trav->job->gatherFutures(mutex,cond,complete,totalJobs);
                        
                        
                    }

                    virtual ~JobSequence() override{
                        delete job;
                    }

                    JobSequence* then(Job* job){
                        if (this->job == NULL) this->job = job;
                        else{
                            JobSequence* trav = this;
                            while(trav->next != NULL) trav = trav->next;
                            trav->next = new JobSequence(job,jobScheduler);
                        }
                        return this;
                    }
                };


                class JobPlan : public Job {

                    friend JobScheduler;

                    private :

                        class RequiredJob : public Job {

                            friend JobPlan;

                            public:
                                virtual void* execute(){
                                    job->execute();
                                    delete job;
                                    //Vary mutex 
                                    pthread_mutex_lock(mutex);
                                        *counter = *counter + 1;
                                        if (*counter == total){

                                            LinkedJobWrapper* next;
                                            for (LinkedJobWrapper* trav = dependent; trav != NULL; trav = next) {
                                                next = trav->getNext();
                                                jobScheduler.submitJob(trav);
                                            } 
                                        }
                                    pthread_mutex_unlock(mutex);
                                    /*Find better return value*/
                                    return NULL;


                                }

                                virtual Future* enableFuture() override {
                                    assert(false);
                                    /*Avoid warnings*/
                                    return NULL;
                                }

                                virtual void gatherFutures(pthread_mutex_t* mutex,pthread_cond_t* cond,bool* complete,size_t& totalFutures) override {
                                    /*Should not be called under normal circumstances*/
                                    assert(false);
                                }

                                ~RequiredJob() override {}

                            private:
                                Job* job;

                                pthread_mutex_t* mutex;

                                /*Counter of required jobs that have been executed*/
                                size_t* counter;

                                /*Total required jobs that must be executed*/
                                size_t total;
                                LinkedJobWrapper* dependent = NULL;
                                RequiredJob* next = NULL;

                                JobScheduler& jobScheduler;

                                RequiredJob(Job* job,JobScheduler& jobScheduler): job(job), jobScheduler(jobScheduler){}

                        };



                        size_t totalRequired = 0;
                        
                        RequiredJob* required = NULL;
                        RequiredJob* reqTail = NULL;

                        LinkedJobWrapper* dependent = NULL;
                        LinkedJobWrapper* depTail = NULL;

                        JobScheduler& jobScheduler;

                    public:

                        JobPlan(JobScheduler& jobScheduler) : totalRequired(0),jobScheduler(jobScheduler) { }

                        virtual ~JobPlan() override {} 

                        virtual Future* enableFuture() override {


                            MultiFuture* multiFuture = new MultiFuture();

                            for (LinkedJobWrapper* trav = dependent; trav != NULL; trav = trav->getNext()){
                                trav->gatherFutures(multiFuture->mutex,multiFuture->cond,multiFuture->complete,multiFuture->totalFutures);
                            }


                            return multiFuture;
                            
                        }

                        virtual void gatherFutures(pthread_mutex_t* mutex,pthread_cond_t* cond,bool* complete,size_t& totalFutures) override{
                                for (LinkedJobWrapper* trav = dependent; trav != NULL; trav = trav->getNext()){
                                    //Only reason this will work is because DependentJob is a list node wrapper
                                    trav->gatherFutures(mutex,cond,complete,totalFutures);
                                }
                        }


                        virtual void* execute(){

                            pthread_mutex_t* mutex = new pthread_mutex_t;
                            pthread_mutex_init(mutex,NULL);

                            size_t* counter = new size_t;
                            *counter = 0;

                            auto cleanupJob = make_job(
                                [=]{

                                    //Acquire mutex before destroying it
                                    pthread_mutex_lock(mutex);
                                    pthread_mutex_unlock(mutex);

                                    pthread_mutex_destroy(mutex);

                                    delete counter;
                                    delete mutex;
                                }
                            );

                            //Need 2 check if dependent->next == NULL
                            depTail->setNext(new LinkedJobWrapper(cleanupJob));
                            depTail = depTail->getNext();

                            RequiredJob* next = NULL;
                            for (RequiredJob* trav = required; trav != NULL; trav = next){
                                //This is needed in case trav get deleted before the loop restarts
                                next = trav->next;

                                trav->counter = counter;
                                trav->total = totalRequired;
                                trav->dependent = dependent;
                                trav->mutex = mutex;
                                jobScheduler.submitJob(trav);
                            }


                            /*Find better return value*/
                            return NULL;
                        }

                        JobPlan* addRequiredJob(Job* job){
                            totalRequired++;

                            if (required == NULL){
                                required = new RequiredJob(job,jobScheduler);
                                reqTail = required;
                            }else{
                                reqTail->next = new RequiredJob(job,jobScheduler);
                                reqTail = reqTail->next;
                            }

                            return this; 
                        }

                        JobPlan* addDependentJob(Job* job){
                            /*Initialize list of dependent jobs */
                            if (dependent == NULL) {
                                dependent = new LinkedJobWrapper(job);
                                depTail = dependent;
                            }
                            /*or append dependent job to the end of the list*/
                            else {
                                depTail->setNext(new LinkedJobWrapper(job));
                                depTail = depTail->getNext();
                            }

                            return this; 
                        }

                };

                JobSequence* makeJobSequence(Job* job){
                    return new JobSequence(job,*this);
                }

                JobSequence* makeJobSequence(){
                    return new JobSequence(*this);
                }

                JobPlan* makeJobPlan(){
                    return new JobPlan(*this);
                }
    };   

                
                
};
    

#endif