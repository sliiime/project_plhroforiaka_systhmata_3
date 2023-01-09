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
                
                /*Jobs completed by workers */
                size_t* completed;
                ccqueue<Job*> queue;
                pthread_t* workers;
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
                

            public:
                
                JobScheduler(size_t workersCount);  
                ~JobScheduler();
                /*The job scheduler does not accept new jobs and waits for workers to finish*/
                void wait();
                void block();
                void submitJob(Job* job);
                size_t workersCount() const;

                
                
    };  
}

#endif