#ifndef __JSCH_JOB_SCHEDULER__
#define __JSCH_JOB_SCHEDULER__

#include <iostream>
#include <fstream>
#include <list>
#include <pthread.h>

#include "ccqueue.hpp"

namespace jsch{

    class Job;
    class JobSequence;
    class JobPlan;
    class Future;

    class JobScheduler{

            private:

                /*Jobs completed by workers */
                size_t* completed;
                ccqueue<Job*> queue;
                pthread_t* workers;
                /*Total workers*/
                size_t total;

                /*Tells workers to quit */
                bool _stopped = false;
                /*No new jobs can be added*/
                void* work();
                /*Mutex and cond used for wait */
                pthread_mutex_t waitMutex;
                pthread_cond_t waitCond;
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
                
                JobSequence* makeJobSequence(Job* job);
                JobSequence* makeJobSequence();
                JobPlan* makeJobPlan();
    };  
            
}
    

#endif