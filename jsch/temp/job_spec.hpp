#ifndef __JSCH_JOB_SPEC__
#define __JSCH_JOB_SPEC__

#include <utility>

#include "supervisor.hpp"
#include "future.hpp"
#include "req_job.hpp"

namespace jsch{

    typedef void* (*workPtr)(void*);
    class LinkedJobWrapper;
    class JobScheduler;

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

            virtual Job* makeRequired(pthread_mutex_t* mutex,size_t* executed,size_t* total,LinkedJobWrapper* dependent,JobScheduler& jobScheduler){
                    (*total)++;
                    return new RequiredJob(this,mutex,executed,total,dependent,jobScheduler);
            }


            virtual ~JobSpecification() override = default;

    };

    template<typename T>
    JobSpecification<T>* make_job(T&& spec){
        return new JobSpecification<T>(std::forward<T>(spec)); 
    }
}

#endif