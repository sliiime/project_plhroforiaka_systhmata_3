#include "supervisor.hpp"

using namespace jsch;

Supervisor::Supervisor() : mutex(NULL),cond(NULL),complete(NULL) {}
Supervisor::Supervisor(pthread_mutex_t* mutex,pthread_cond_t* cond,bool* complete) : mutex(mutex),cond(cond),complete(complete) {}
Supervisor::Supervisor(const Supervisor& s): mutex(s.mutex),cond(s.cond),complete(s.complete) {}

void Supervisor::checkOut() {

    if (mutex == NULL) return;

    pthread_mutex_lock(mutex);
        *complete = true;
        pthread_cond_broadcast(cond);
    pthread_mutex_unlock(mutex);
}