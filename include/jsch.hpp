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
                *complete = true;
                pthread_cond_broadcast(cond);
            }

            Future(): mutex(new pthread_mutex_t), cond(new pthread_cond_t), complete(new bool(false)){
                pthread_mutex_init(mutex,NULL);
                pthread_cond_init(cond,NULL);
            }

            Future(const Future& f): cond(f.cond),complete(f.complete),mutex(f.mutex) {}
        
        protected:
            Future(bool* complete) : mutex(new pthread_mutex_t),cond(new pthread_cond_t),complete(complete) {
                pthread_mutex_init(mutex,NULL);
                pthread_cond_init(cond,NULL);

            }

        public:
            ~Future(){ }

            virtual void wait(){
                pthread_mutex_lock(mutex);
                    while (! *complete) pthread_cond_wait(cond,mutex);
                pthread_mutex_unlock(mutex); 
            }
    };


    class MultiFuture : public Future {

        private:

            static const size_t MAX_FUTURES = 256;
            


        public:

            size_t totalFutures = 0;

            MultiFuture(): Future(new bool[256]) {}

            virtual void wait() override {
                pthread_mutex_lock(mutex);
                    while (!checkDone()) pthread_cond_wait(cond,mutex);
                pthread_mutex_unlock(mutex);
            }

            bool checkDone(){
                for (size_t i = 0; i < totalFutures; i++){
                    if (!complete[i]) return false;
                }
                return true;
            }


    };

    class Job{
        public:

            virtual void* execute() = 0;
            virtual ~Job() = default;
            virtual Future* enableFuture() = 0;
            virtual void enableFuture(const Future& future,size_t& totalJobs) = 0;

        protected:
            Future* future;
    };

    template<typename S>
    class JobSpecification : public Job {


        private:

            S spec;
        
        public:
            /*Universal reference*/
            JobSpecification(S&& spec): spec(spec){}

            void* execute() override {
                spec();
                if (future != NULL) future->done();
                /*find a different return value?*/
                return NULL;
            }

            virtual Future* enableFuture() override {
                this->future = new Future();
                return this->future;
            }

            virtual void enableFuture(const Future& future,size_t& totalJobs) override{
                this->future = new Future(future);
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
                /*Someone is waiting for the Scheduler to finish*/
                bool _waitEnabled = false;
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
                void wait();
                void block();
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
                public:

                    virtual void* execute(){
                        job->execute();
                        if( next != NULL) jobScheduler.submitJob(next);
                        else {
                            if (future != NULL) future->done();
                        }
                        /*Find better return value*/
                        return NULL;
                    }

                    virtual Future* enableFuture() override {
                        JobSequence* trav = this;
                        while (trav->next != NULL) trav = trav -> next;

                        this->future = trav->job->enableFuture();
                        
                        return this->future;

                    }

                    virtual void enableFuture(const Future& future,size_t& totalJobs) override {

                        JobSequence* trav = this;
                        while (trav->next != NULL) trav = trav -> next;

                        /*In order not to call JobSequence::enableFuture again*/
                        trav->job->enableFuture(future,totalJobs);
                        
                        
                    }

                    virtual ~JobSequence() override{
                        delete job;
                    }

                    JobSequence* then(Job* job){
                        JobSequence* trav = this;
                        while(trav->next != NULL) trav = trav->next;
                        trav->next = new JobSequence(job,jobScheduler);
                        return this;
                    }
                };


                class MultiJobSequence : public Job {
                    friend JobScheduler;

                    private :

                        class DependentJob : public Job {
                            friend MultiJobSequence;
                            public:
                                virtual void* execute(){
                                    job->execute();
                                    /*Find better return value*/
                                    return NULL;
                                }

                                virtual Future* enableFuture() override{
                                    this->future = new Future();
                                    return this->future;
                                }

                                virtual void enableFuture(const Future& future,size_t& totalJobs) override {
                                    this->job->enableFuture(future,totalJobs);
                                }

                                virtual ~DependentJob() override {
                                    delete job;
                                }

                            private:
                                Job* job;
                                DependentJob* next = NULL;
                                DependentJob(Job* job) : job(job) {}

                        };

                        class RequiredJob : public Job {
                            friend MultiJobSequence;

                            public:
                                virtual void* execute(){
                                    job->execute();
                                    pthread_mutex_lock(mutex);
                                        if (++(*counter) == total){
                                            for (DependentJob* trav = dependent; trav != NULL; trav = trav->next) {
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

                                virtual void enableFuture(const Future& future,size_t& totalJobs) override {
                                    /*Should not be called under normal circumstances*/
                                    assert(false);
                                }

                                ~RequiredJob() override {}
                            private:
                                Job* job;

                                pthread_mutex_t* mutex;
                                size_t* counter;

                                size_t total;
                                DependentJob* dependent = NULL;
                                RequiredJob* next = NULL;

                                JobScheduler& jobScheduler;

                                RequiredJob(Job* job,JobScheduler& jobScheduler): job(job), jobScheduler(jobScheduler){}

                        };



                        size_t reqCount = 0;
                        
                        RequiredJob* required = NULL;
                        RequiredJob* lastReq;

                        DependentJob* dependent = NULL;
                        DependentJob* lastDep;

                        JobScheduler& jobScheduler;

                    public:

                        MultiJobSequence(Job* req,Job* dep,JobScheduler& jobScheduler) : 
                        reqCount(1),required(new RequiredJob(req,jobScheduler)),jobScheduler(jobScheduler) {

                            DependentJob* _dep = new DependentJob(dep);
                            RequiredJob*  _req = new RequiredJob(req,jobScheduler);

                            this->required = _req;
                            this->lastReq = _req;
                            this->dependent = _dep;
                            this->lastDep = _dep;
                        }

                        MultiJobSequence(Job* req,JobScheduler& jobScheduler) : reqCount(1),jobScheduler(jobScheduler){
                            RequiredJob* _req = new RequiredJob(req,jobScheduler);
                            this->required = _req;
                            this->lastReq = _req;
                        }

                        virtual ~MultiJobSequence() override {} ;

                        virtual Future* enableFuture() override {

                            std::cout << "Kale, lolathike to paidi! Hahahhaha" << std::endl;

                            MultiFuture* multiFuture = new MultiFuture();
                            this->future = multiFuture;

                            for (DependentJob* trav = dependent; trav != NULL; trav = trav->next){
                                trav->job->enableFuture(*multiFuture,multiFuture->totalFutures);
                            }

                            return this->future;
                            
                        }

                        virtual void enableFuture(const Future& future,size_t& totalJobs) override{
                                for (DependentJob* trav = dependent; trav != NULL; trav = trav->next){
                                    //Only reason this might work is because DependentJob is a list node wrapper
                                    trav->job->enableFuture(future,totalJobs);
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
                            lastDep->next = new DependentJob(cleanupJob);
                            lastDep = lastDep->next;


                            for (RequiredJob* trav = required; trav != NULL; trav = trav->next){
                                trav->counter = counter;
                                trav->total = reqCount;
                                trav->dependent = dependent;
                                trav->mutex = mutex;
                                jobScheduler.submitJob(trav);
                            }

                            /*Find better return value*/
                            return NULL;
                        }

                        MultiJobSequence* addRequirement(Job* job){
                            reqCount++;
                            lastReq->next = new RequiredJob(job,jobScheduler);
                            lastReq = lastReq->next;
                            
                            return this; 
                        }

                        MultiJobSequence* addDependent(Job* job){
                            /*Initialize list of dependent jobs */
                            if (dependent == NULL) {
                                dependent = new DependentJob(job);
                                lastDep = dependent;
                            }
                            /*or append dependent job to the end of the list*/
                            else {
                                lastDep->next = new DependentJob(job);
                                lastDep = lastDep->next;
                            }

                            return this; 
                        }

                };

                JobSequence* makeJobSequence(Job* job){
                    return new JobSequence(job,*this);
                }

                MultiJobSequence* makeMultiJobSequence(Job* req,Job* dep){
                    return new MultiJobSequence(req,dep,*this);
                }
                MultiJobSequence* makeMultiJobSequence(Job* req){
                    return new MultiJobSequence(req,*this);
                }
    };   

                
                
};
    

#endif