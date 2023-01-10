#ifndef __JSCH__
#define __JSCH__

#include <iostream>
#include <list>
#include "ccqueue.hpp"
#include <pthread.h>
#include <fstream>


namespace jsch{



    class Job{
        public:

            virtual void* execute() = 0;
            virtual ~Job() = default;
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
                /*find a different return value?*/
                return NULL;
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
                        /*Find better return value*/
                        return NULL;
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
                                            for (DependentJob* trav = dependent; trav != NULL; trav = trav->next) jobScheduler.submitJob(trav); 
                                        }
                                    pthread_mutex_unlock(mutex);
                                    /*Find better return value*/
                                    return NULL;


                                }
                                ~RequiredJob() override {}
                            private:
                                Job* job;

                                pthread_mutex_t* mutex;
                                size_t* counter;

                                size_t total;
                                DependentJob* dependent = NULL;
                                RequiredJob* next;

                                JobScheduler& jobScheduler;

                                RequiredJob(Job* job,JobScheduler& jobScheduler): job(job), jobScheduler(jobScheduler){}

                        };



                        size_t reqCount = 0;
                        
                        RequiredJob* required;
                        DependentJob* dependent;
                        JobScheduler& jobScheduler;

                    public:

                        MultiJobSequence(Job* req,Job* dep,JobScheduler& jobScheduler) : 
                        required(new RequiredJob(req,jobScheduler)),dependent(new DependentJob(dep)),jobScheduler(jobScheduler){}

                        virtual void* execute(){

                            pthread_mutex_t* mutex = new pthread_mutex_t;
                            pthread_mutex_init(mutex,NULL);

                            size_t* counter = new size_t;
                            *counter = 0;

                            auto cleanupJob = make_job(
                                [&]{
                                    pthread_mutex_destroy(mutex);
                                    delete counter;
                                }
                            );

                            //Need 2 check if dependent->next == NULL
                            dependent->next = new DependentJob(cleanupJob);

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
                            for (RequiredJob* trav = required; trav->next != NULL; trav = trav->next) trav->next = new RequiredJob(job,jobScheduler);
                            return this; 
                        }

                        MultiJobSequence* addDependent(Job* job){
                            /*Initialize list of dependent jobs */
                            if (dependent == NULL) dependent = new DependentJob(job);
                            /*or append dependent job to the end of the list*/
                            else for (DependentJob* trav = dependent; trav->next != NULL; trav = trav->next) trav->next = new DependentJob(job);

                            return this; 
                        }



                };

                JobSequence* makeJobSequence(Job* job){
                    return new JobSequence(job,*this);
                }

                MultiJobSequence* makeMultiJobSequence(Job* req,Job* dep){
                    return new MultiJobSequence(req,dep,*this);
                }
    };   

                
                
};
    

#endif