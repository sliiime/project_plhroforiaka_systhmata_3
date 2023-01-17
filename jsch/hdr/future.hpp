#ifndef __JSCH_FUTURE__
#define __JSCH_FUTURE__


#include <pthread.h>

namespace jsch{

    class Future {


            public:

                pthread_mutex_t* mutex;
                pthread_cond_t* cond;
                bool* complete;

                void done();
                Future();
                Future(pthread_mutex_t* mutex,pthread_cond_t* cond,bool* complete);
                virtual ~Future();
                virtual void wait();
            
            protected:
                Future(bool* complete);
                virtual bool checkComplete();
                
            public:

    };

    class MultiFuture : public Future {

        private:

            static const size_t MAX_FUTURES = 256;
            
        public:

            size_t totalFutures = 0;

            MultiFuture();
            virtual ~MultiFuture() override;

        protected:
            virtual bool checkComplete() override;

    };
    
}

#endif