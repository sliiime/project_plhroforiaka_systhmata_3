#include "req_job.hpp"
#include "jsch.hpp"
#include "linked_job_wrapper.hpp"

using namespace jsch;

RequiredJob::RequiredJob(Job* job, pthread_mutex_t* mutex, size_t* counter,size_t* total,LinkedJobWrapper* dependent,JobScheduler& jobScheduler)
            :job(job),mutex(mutex),executed(counter),total(total),dependent(dependent),jobScheduler(jobScheduler) {}

void* RequiredJob::execute(){
        job->execute();
        delete job;
        pthread_mutex_lock(mutex);
        *executed = *executed + 1;
        if (*executed == *total){
            LinkedJobWrapper* next;
            for (LinkedJobWrapper* trav = dependent; trav != NULL; trav = next) {
                next = trav->getNext();
                jobScheduler.submitJob(trav);
            } 
        }
        pthread_mutex_unlock(mutex);
        /*Find better return value*/
        return NULL;
}

Future* RequiredJob::enableFuture(){
    assert(false);
    /*Avoid warnings*/
    return NULL;
}

void RequiredJob::gatherFutures(pthread_mutex_t* mutex,pthread_cond_t* cond,bool* complete,size_t& totalFutures) {
    /*Should not be called under normal circumstances*/
    assert(false);
}

Job* RequiredJob::makeRequired(pthread_mutex_t* mutex,size_t* executed,size_t* total,LinkedJobWrapper* dependent,JobScheduler& jobScheduler) {
    assert(false && "makeRequired RequiredJob");
}
RequiredJob::~RequiredJob() {}
