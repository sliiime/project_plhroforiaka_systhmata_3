#include "job.hpp"

namespace jsch{

    class JobScheduler;
    class LinkedJobWrapper;
    
    class JobPlan : public Job {

        friend JobScheduler;

            private :
                
                LinkedJobWrapper* required = NULL;
                LinkedJobWrapper* reqTail = NULL;

                LinkedJobWrapper* dependent = NULL;
                LinkedJobWrapper* depTail = NULL;

                JobScheduler& jobScheduler;

            public:

                JobPlan(JobScheduler& jobScheduler);
                virtual ~JobPlan() override; 
                virtual Future* enableFuture() override;
                virtual void gatherFutures(pthread_mutex_t* mutex,pthread_cond_t* cond,bool* complete,size_t& totalFutures)override;
                virtual void* execute() override;
                virtual Job* makeRequired(pthread_mutex_t* mutex,size_t* executed,size_t* total,LinkedJobWrapper* dependent,JobScheduler& jobScheduler) override;
                JobPlan* addRequiredJob(Job* job);
                JobPlan* addDependentJob(Job* job);
    };
}