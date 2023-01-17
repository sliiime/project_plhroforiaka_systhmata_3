#ifndef __JSCH_SUPERVISOR__
#define __JSCH_SUPERVISOR__

#include <pthread.h>

namespace jsch{

    class Supervisor {
        private:
            pthread_mutex_t* mutex;
            pthread_cond_t* cond;
            bool* complete;
        public:
            Supervisor();
            Supervisor(pthread_mutex_t* mutex,pthread_cond_t* cond,bool* complete);
            Supervisor(const Supervisor& s);
            void checkOut();
    };
}

#endif