#include "list.hpp"

template <typename T>
class Queue : public List<T>{
public:
    Queue();
    ~Queue();

    void push(T);
    T pop();
    T pop_and_push(T t);
};

template <typename T>
void Queue<T>::push(T value) {
    this->insert_last(value);
}

template <typename T>
T Queue<T>::pop() {

    if (this->size == 0) throw std::__throw_bad_exception;

    T value = this->begin()->get_value();
    ListNode<T>* temp = this->begin();
    this->first = this->begin()->get_next();
    delete temp;

    if (this->first == NULL) {
        this->last = NULL;
    }

    this->size--;


    return value;
}

template <typename T>
Queue<T>::Queue() {
}

template <typename T>
Queue<T>::~Queue() {
}

template <typename T>
T Queue<T>::pop_and_push(T value){

    T oldValue;
    if (this->size == 0) throw std::runtime_error("Queue is empty");

    else if (this->size == 1) {

        ListNode<T>* node = this->first;
        oldValue = node->get_value();
        node->set_value(value);

    }else {

        ListNode<T>* temp = this->first;

        oldValue = temp->get_value();

        temp->set_value(value);

        this->first = this->first->get_next();
        this->last->set_next(temp);
        this->last = temp;
        temp->set_next(NULL);
    }

    return oldValue;
    



}