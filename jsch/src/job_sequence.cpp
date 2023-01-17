#include "job_sequence.hpp"
#include "linked_job_wrapper.hpp"
#include "job_spec.hpp"
#include "job_scheduler.hpp"

using namespace jsch;

JobSequence::JobSequence(Job* job,JobScheduler& jobScheduler ) : jobScheduler(jobScheduler),head(new LinkedJobWrapper(job)),tail(head), size(1) {}
JobSequence::JobSequence(JobScheduler& jobScheduler) : jobScheduler(jobScheduler) {}

void* JobSequence::execute(){
    pthread_mutex_t* mutexes = new pthread_mutex_t[size];
    size_t* executed = new size_t[size];
    size_t* total = new size_t[size];
    
    for (size_t i = 0; i < size; i++){
        pthread_mutex_init(mutexes+i,NULL);
        executed[i] = 0;
        total[i] = 0;
    }

    size_t i = 0;
    size_t sSize = size;

    Job* cleanup = jsch::make_job(
            [=]() mutable {
                for (size_t j = 0; j < sSize; j++){

                    pthread_mutex_lock(mutexes+j);
                    pthread_mutex_unlock(mutexes+j);

                    pthread_mutex_destroy(mutexes+j);
                }

                delete[] mutexes;
                delete[] executed;
                delete[] total;
            });

    assert(tail != NULL && head != NULL);

    tail->setNext(new LinkedJobWrapper(cleanup));
    tail = tail->getNext();                        
    size++;

    for (LinkedJobWrapper* trav = head; trav != tail; trav = trav->getNext()){
        trav->makeRequired(mutexes+i,executed+i,total+i,trav->getNext(),jobScheduler);
        i++;
    }

    assert(head != NULL);

    jobScheduler.submitJob(head);

    return NULL;
}

Future* JobSequence::enableFuture() {
    return tail->enableFuture();                        
}

void JobSequence::gatherFutures(pthread_mutex_t* mutex,pthread_cond_t* cond,bool* complete,size_t& totalJobs) {
    tail->gatherFutures(mutex,cond,complete,totalJobs);
}

Job* JobSequence::makeRequired(pthread_mutex_t* mutex,size_t* executed,size_t* total,LinkedJobWrapper* dependent,JobScheduler& jobScheduler) {
        tail->makeRequired(mutex,executed,total,dependent,jobScheduler);
        return this;
}
JobSequence::~JobSequence(){ }

JobSequence* JobSequence::then(Job* job){
    size++;

    if (head == NULL) {
        head = new LinkedJobWrapper(job,NULL,JOB_SEQ);
        tail = head;
    }else{
        tail->setNext(new LinkedJobWrapper(job,NULL,JOB_SEQ));
        tail = tail->getNext();
    }

    return this;
}



