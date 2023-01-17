#ifndef __JSCH_CCQUEUE__
#define __JSCH_CCQUEUE__

#include <iostream>
#include <pthread.h>
#include <assert.h>


namespace jsch{

    enum op_status{success,closed};

    template<typename T>
    class ccqueue{

        public:

        private:

            class Node{
                friend class ccqueue;
                private: 
                    T value;
                    Node* next;
                    Node(T value,Node* next = NULL): value(value),next(next){}
            };
        
        private:
            pthread_mutex_t mutex;
            pthread_cond_t cond;

            Node* head = NULL;
            Node* tail = NULL;

            uint32_t size = 0;
            bool _closed = false;

        public:

            ccqueue();
            ~ccqueue();
            op_status push(const T& t);
            bool empty() const;
            op_status pop(T& holder);
            void close();
    };

    template<typename T>
    op_status ccqueue<T>::push(const T& t){

        if (_closed) return op_status::closed; 

        Node* node = new Node(t);

        pthread_mutex_lock(&mutex);
            if (size == 0 ) {
                head = node;
                tail = node;
            }else{
                tail->next = node;
                tail = tail->next;
            }
            size++;
            pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);

        return op_status::success;
    }

    template<typename T>
    ccqueue<T>::ccqueue(){ 
        pthread_mutex_init(&mutex,NULL);
        pthread_cond_init(&cond,NULL); 
    }

    template<typename T>
    ccqueue<T>::~ccqueue(){

        assert(empty());

        for (Node* node = head; node != NULL;) {
            Node* d = node;
            node = d->next;
            delete d;
        }

        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }

    template<typename T>
    bool ccqueue<T>::empty() const {
        return size == 0;
    }

    template<typename T>
    op_status  ccqueue<T>::pop(T& holder){
        pthread_mutex_lock(&mutex);

            /*Blocks if queue is empty */
            while (size == 0 && !_closed) pthread_cond_wait(&cond,&mutex);

            if (_closed){
                pthread_mutex_unlock(&mutex);
                return op_status::closed;
            }
            
            Node* popped = head;

            /*Not sure if good practice*/
            holder = std::move(head->value);

            head = head->next;
            size--;
            if (size == 0) tail = NULL;

        pthread_mutex_unlock(&mutex);
        
        delete popped;

        return op_status::success;
        
    }

    template<typename T>
    void  ccqueue<T>::close(){ 
        _closed = true;
        pthread_mutex_lock(&mutex);
            pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mutex);
    }
}

#endif