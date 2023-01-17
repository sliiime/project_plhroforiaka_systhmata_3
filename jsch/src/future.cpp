#include <pthread.h>

#include "future.hpp"

using namespace jsch;

    
void Future::done(){
    pthread_mutex_lock(mutex);
        *complete = true;
        pthread_cond_broadcast(cond);
    pthread_mutex_unlock(mutex);
}

Future::Future(): mutex(new pthread_mutex_t), cond(new pthread_cond_t), complete(new bool(false)){
    pthread_mutex_init(mutex,NULL);
    pthread_cond_init(cond,NULL);
}


Future::Future(pthread_mutex_t* mutex,pthread_cond_t* cond,bool* complete) : mutex(mutex),cond(cond) , complete(complete)  {}


Future::Future(bool* complete) : mutex(new pthread_mutex_t),cond(new pthread_cond_t),complete(complete) {
    pthread_mutex_init(mutex,NULL);
    pthread_cond_init(cond,NULL);

}
bool Future::checkComplete() {
    return *complete;
}


Future::~Future(){
    pthread_mutex_destroy(mutex);
    pthread_cond_destroy(cond);

    delete mutex;
    delete cond;
    delete complete;
}

void Future::wait(){
    pthread_mutex_lock(mutex);
        while (!checkComplete()) pthread_cond_wait(cond,mutex);
    pthread_mutex_unlock(mutex); 

    delete this;
}



MultiFuture::MultiFuture(): Future(new bool[MAX_FUTURES]) {
    for (size_t i = 0 ; i < MAX_FUTURES; i++) complete[i] = false;
}

MultiFuture::~MultiFuture() {
    delete[] complete;
    this->complete = NULL;
}

bool MultiFuture::checkComplete() {
    for (size_t i = 0; i < totalFutures; i++){
        if (!complete[i]) return false;
    }
    return true;
}

