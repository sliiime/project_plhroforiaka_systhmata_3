#ifndef __JSCH_LINKED_JOB_WRAPPER__
#define __JSCH_LINKED_JOB_WRAPPER__

#include "job.hpp"

namespace jsch {
    
    enum WrapperType{JOB_SEQ,SIMPLE};
    
    class LinkedJobWrapper : public Job {
        private:
            Job* job;
            LinkedJobWrapper* next;
            WrapperType type;

        public:
            LinkedJobWrapper(Job* job,LinkedJobWrapper* next = NULL,WrapperType type = SIMPLE);

            LinkedJobWrapper* getNext();

            void setNext(LinkedJobWrapper* next);

            Job* getJob();

            virtual void* execute() override;

            virtual ~LinkedJobWrapper() override;

            virtual Future* enableFuture() override;

            virtual Job* makeRequired(pthread_mutex_t* mutex,size_t* counter,size_t* totalRequired,LinkedJobWrapper* dependent,JobScheduler& jobScheduler) override;

            virtual void gatherFutures(pthread_mutex_t* mutex, pthread_cond_t* cond, bool* complete, size_t& totalJobs) override;

            WrapperType getType() const;


    };
}

#endif