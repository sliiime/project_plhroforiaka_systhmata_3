#include "linked_job_wrapper.hpp"

using namespace jsch;

LinkedJobWrapper::LinkedJobWrapper(Job* job,LinkedJobWrapper* next ) : job(job),next(next) {}

LinkedJobWrapper* LinkedJobWrapper::getNext() {
    return next;
}

void LinkedJobWrapper::setNext(LinkedJobWrapper* next){
    this->next = next;
}

Job* LinkedJobWrapper::getJob(){
    return job;
}

void* LinkedJobWrapper::execute(){
    job->execute();
    return NULL;
}

LinkedJobWrapper::~LinkedJobWrapper(){
    delete job;
}

Future* LinkedJobWrapper::enableFuture(){
    assert(false && "enableFuture LinkedJobWrapper");
}

Job* LinkedJobWrapper::makeRequired(pthread_mutex_t* mutex,size_t* counter,size_t* totalRequired,LinkedJobWrapper* dependent,JobScheduler& jobScheduler){
        this->job = job->makeRequired(mutex,counter,totalRequired,dependent,jobScheduler);
        return this;
}

void LinkedJobWrapper::gatherFutures(pthread_mutex_t* mutex, pthread_cond_t* cond, bool* complete, size_t& totalJobs){
    job->gatherFutures(mutex,cond,complete,totalJobs);
}
