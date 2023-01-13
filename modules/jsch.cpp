#include "jsch.hpp"

void* jsch::JobScheduler::work(){
    /*Initialization*/
    pthread_mutex_lock(&waitMutex);
        size_t id = selfSerialId();
    pthread_mutex_unlock(&waitMutex);
    while (!_stopped){
        /* blocks */
        Job* job;
        if (queue.pop(job) == success){

            active[id] = true;
            job->execute();
            completed[id]++;
            active[id] = false;

            delete job;
        }

        if (queue.empty() && _waitEnabled ){
            pthread_mutex_lock(&waitMutex);
                pthread_cond_broadcast(&waitCond);
            pthread_mutex_unlock(&waitMutex);
        } 
    }
}



void jsch::JobScheduler::wait(){
    _waitEnabled = true;
    pthread_mutex_lock(&waitMutex);
        while (!queue.empty() || !onVacation()) pthread_cond_wait(&waitCond,&waitMutex);
    pthread_mutex_unlock(&waitMutex);
}

void jsch::JobScheduler::block(){
    _blocked = true;
}

size_t jsch::JobScheduler::selfSerialId() const {
    for (size_t i = 0; i < total; i++){
        if (pthread_self() == workers[i]) return i;
    }
    assert(false);
}

bool jsch::JobScheduler::onVacation() const {
    for (size_t i = 0; i < total; i++){
        if (active[i]) return false;
    }
    return true;
}

jsch::JobScheduler::JobScheduler(size_t workersCount): queue(ccqueue<Job*>()),total(workersCount)  {
    workers = new pthread_t[workersCount];
    active = new bool[workersCount];
    completed = new size_t[workersCount];

    pthread_mutex_init(&waitMutex,NULL);
    pthread_cond_init(&waitCond,NULL);
        
    /*Not allowing workers to start until proper structure initialization has taken place */
    pthread_mutex_lock(&waitMutex);
    for (size_t i = 0 ; i < workersCount; i++) pthread_create(workers+i,NULL,(workPtr)&JobScheduler::work,this);
    pthread_mutex_unlock(&waitMutex);
}

void jsch::JobScheduler::submitJob(Job* job){ 
    if (!_blocked && !_stopped) queue.push(job);
    else delete job;
}

jsch::JobScheduler::~JobScheduler(){

    /*Workers must terminate*/
    _stopped = true;
    /*Queue does not accept new jobs*/
    queue.close();
    /*Join threads */
    for (size_t i = 0 ; i < total; i++){
        pthread_join(workers[i],NULL);
        // std::cout << "Joined " << i << std::endl; 
    }
    /*Delete workers metadata*/
    delete[] workers;
    delete[] completed;
    delete[] active;
    /*Destroy synchronization resources*/
    pthread_mutex_destroy(&waitMutex);
    pthread_cond_destroy(&waitCond);

}