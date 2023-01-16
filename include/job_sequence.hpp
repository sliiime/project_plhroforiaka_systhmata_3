#ifndef __JSCH_JOBSEQUENCE__
#define __JSCH_JOBSEQUENCE__

#include "job.hpp"

namespace jsch{
    
    class LinkedJobWrapper;

    class JobSequence : public Job {

        friend JobScheduler;

        private:
            JobScheduler& jobScheduler;
            LinkedJobWrapper* head = NULL;
            LinkedJobWrapper* tail = NULL;
            size_t size = 0;
        protected:
            JobSequence(Job* job,JobScheduler& jobScheduler );
            JobSequence(JobScheduler& jobScheduler);
        public:

        virtual void* execute()override;

        virtual Future* enableFuture()override;

        virtual void gatherFutures(pthread_mutex_t* mutex,pthread_cond_t* cond,bool* complete,size_t& totalJobs)override;
        virtual Job* makeRequired(pthread_mutex_t* mutex,size_t* executed,size_t* total,LinkedJobWrapper* dependent,JobScheduler& jobScheduler) override;
        virtual ~JobSequence() override;
        JobSequence* then(Job* job);
    };                
}

#endif