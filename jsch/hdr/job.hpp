#ifndef __JSCH_JOB__
#define __JSCH_JOB__

#include <pthread.h>
#include <assert.h>

namespace jsch {

    class JobScheduler;
    class LinkedJobWrapper;
    class Future;

    class Job{

        public:

            virtual void* execute() = 0;
            virtual ~Job() = default;

            virtual Future* enableFuture() = 0;
            virtual void gatherFutures(pthread_mutex_t* mutex,pthread_cond_t* cond,bool* complete,size_t& totalJobs) = 0;

            virtual Job* makeRequired(pthread_mutex_t* mutex,size_t* counter,size_t* totalRequired,LinkedJobWrapper* dependent,JobScheduler& jobScheduler){
                assert(false);
            }
    };
}

#endif