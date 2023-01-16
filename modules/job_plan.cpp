#include "job_plan.hpp"
#include "jsch.hpp"
#include "future.hpp"
#include "linked_job_wrapper.hpp"
#include "job_spec.hpp"

using namespace jsch;

JobPlan::JobPlan(JobScheduler& jobScheduler) : jobScheduler(jobScheduler) { }

JobPlan::~JobPlan(){} 

Future* JobPlan::enableFuture(){


    MultiFuture* multiFuture = new MultiFuture();

    for (LinkedJobWrapper* trav = dependent; trav != NULL; trav = trav->getNext()){
        trav->gatherFutures(multiFuture->mutex,multiFuture->cond,multiFuture->complete,multiFuture->totalFutures);
    }


    return multiFuture;
}

void JobPlan::gatherFutures(pthread_mutex_t* mutex,pthread_cond_t* cond,bool* complete,size_t& totalFutures){
        for (LinkedJobWrapper* trav = dependent; trav != NULL; trav = trav->getNext()){
            //Only reason this will work is because DependentJob is a list node wrapper
            trav->gatherFutures(mutex,cond,complete,totalFutures);
        }
}


void* JobPlan::execute(){

    pthread_mutex_t* mutex = new pthread_mutex_t;
    pthread_mutex_init(mutex,NULL);

    size_t* executed = new size_t;
    *executed = 0;

    size_t* totalRequired = new size_t;
    *totalRequired = 0;

    auto cleanup = make_job(
        [=]{

            //Acquire mutex before destroying it
            pthread_mutex_lock(mutex);
            pthread_mutex_unlock(mutex);

            pthread_mutex_destroy(mutex);

            delete totalRequired;
            delete mutex;
            delete executed;
        }
    );

    depTail->setNext(new LinkedJobWrapper(cleanup));
    depTail = depTail->getNext();

    for (LinkedJobWrapper* trav = required; trav != NULL; trav = trav->getNext()){
        trav->makeRequired(mutex,executed,totalRequired,dependent,jobScheduler);
    }

    LinkedJobWrapper* next = NULL;
    for (LinkedJobWrapper* trav = required; trav != NULL; trav = next){
        next = trav->getNext();
        jobScheduler.submitJob(trav);
    }


    /*Find better return value*/
    return NULL;
}

Job* JobPlan::makeRequired(pthread_mutex_t* mutex,size_t* executed,size_t* total,LinkedJobWrapper* dependent,JobScheduler& jobScheduler){
    for (LinkedJobWrapper* trav = dependent; trav != NULL; trav = trav->getNext()){
        trav->makeRequired(mutex,executed,total,dependent,jobScheduler);
    }

    return this;
}

JobPlan* JobPlan::addRequiredJob(Job* job){
    if (required == NULL){
        required = new LinkedJobWrapper(job);
        reqTail = required;
    }else{
        reqTail->setNext(new LinkedJobWrapper(job));
        reqTail = reqTail->getNext();
    }

    return this; 
}

JobPlan* JobPlan::addDependentJob(Job* job){
    /*Initialize list of dependent jobs */
    if (dependent == NULL) {
        dependent = new LinkedJobWrapper(job);
        depTail = dependent;
    }
    /*or append dependent job to the end of the list*/
    else {
        depTail->setNext(new LinkedJobWrapper(job));
        depTail = depTail->getNext();
    }

    return this; 
}