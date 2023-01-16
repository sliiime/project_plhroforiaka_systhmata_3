#ifndef __JSCH_REQ_JOB__
#define __JSCH_REQ_JOB__

#include "job.hpp"

namespace jsch{

    class JobScheduler;
    class Future;
    class LinkedJobWrapper;

    class RequiredJob : public Job {

        public:

            RequiredJob(Job* job, pthread_mutex_t* mutex, size_t* counter,size_t* total,LinkedJobWrapper* dependent,JobScheduler& jobScheduler);
            virtual void* execute() override;
            virtual Future* enableFuture() override;
            virtual void gatherFutures(pthread_mutex_t* mutex,pthread_cond_t* cond,bool* complete,size_t& totalFutures) override;
            virtual Job* makeRequired(pthread_mutex_t* mutex,size_t* executed,size_t* total,LinkedJobWrapper* dependent,JobScheduler& jobScheduler) override;
            virtual~RequiredJob() override;

            private:

                Job* job;

                pthread_mutex_t* mutex;

                /*Counter of required jobs that have been executed*/
                size_t* executed;
                /*Total required jobs that must be executed*/
                size_t* total;
                LinkedJobWrapper* dependent = NULL;

                JobScheduler& jobScheduler;
    };
}

#endif