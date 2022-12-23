#pragma once
#include <iostream> 
#include <cstdint>

template <typename T>
class ListNode {
    private:
        T value;
        ListNode<T> *next;
    public:
        ListNode<T>(T, ListNode<T> *);
        ~ListNode<T>();

        void set_value(T);
        void set_next(ListNode<T> *);

        T get_value()const;
        ListNode<T> *get_next()const;
};

template <typename T>
class List {
    protected:
        ListNode<T> *first;
        ListNode<T> *last;
        uint32_t size;
    public:
        List();
        ~List();

        void change_node_value(T, T);

        ListNode<T> *begin()const;
        ListNode<T> *end()const;
        T get_node_value(ListNode<T> *)const;
        ListNode<T> *get_next(ListNode<T> *)const;

        uint32_t get_size()const;

        void insert(T);
        void insert_last(T);
        bool remove(T);
        void push(T);
        bool contains(T);
        T get(int index);
        void remove_at(int index);
        void print();
        int index_of(T);
        void set(int, T);
};

/* ListNode */

template <typename T>
ListNode<T>::ListNode(T value, ListNode *next) {
    this->value = value;
    this->next = next;
}

template <typename T>
ListNode<T>::~ListNode() { 

}

template <typename T>
void ListNode<T>::set_value(T value) {
    this->value = value;
}

template <typename T>
void ListNode<T>::set_next(ListNode *node) {
    this->next = node;
}


template <typename T>
ListNode<T> *ListNode<T>::get_next()const {
    return this->next;
}

template <typename T>
T ListNode<T>::get_value()const {
    return this->value;
}

/* List */

template <typename T>
List<T>::List() {
    this->first = NULL;
    this->last = NULL;
    this->size = 0;
}

template <typename T>
List<T>::~List() {
    ListNode<T> *node = this->first;
	while (node != NULL) {			
		ListNode<T> *temp = node;	

        node = node->get_next();

		delete temp;
	}
}

template <typename T>
void List<T>::change_node_value(T old_value, T new_value) {
    for(ListNode<T> *node = this->first ; node != NULL ; node = node->get_next()) {
        if(node->get_value() == old_value) {
            node->set_value(new_value);
            break;
        }
    }
}

template <typename T>
ListNode<T> *List<T>::begin()const {
    return first;
}

template <typename T>
ListNode<T> *List<T>::end()const {
    return last;
}

template <typename T>
T List<T>::get_node_value(ListNode<T> *node)const {
    return node->get_value();
}

template <typename T>
ListNode<T> *List<T>::get_next(ListNode<T> *node)const {
    return node->get_next();
}

template <typename T>
uint32_t List<T>::get_size()const {
    return size;
}

template <typename T>
void List<T>::insert(T value) {
    ListNode<T> *newnode = new ListNode<T>(value, this->first);
    this->first = newnode;

    this->size++;

    if(this->size == 1) {
        this->last = newnode;
    }
}

template <typename T>
bool List<T>::remove(T value) {
    
    ListNode<T> *temp;
    for(ListNode<T> *node = this->first ; node != NULL ; node = node->get_next()) {
        if(node->get_value() == value) {
            if(node == this->first) {       // node is first of list
                this->first = node->get_next();

                if(node == this->last) this->last = NULL;

                delete node;

                size--;

                return 1;
            }
            // node is any other node
            temp->set_next(node->get_next());

            if(node == this->last) this->last = temp;

            delete node;

            size--;

            return 1;
        }
        temp = node;
    }
    return 0;
}

template <typename T>
void List<T>::insert_last(T value) {

    if (this->size == 0) {
        this->insert(value);
    } else {
        ListNode<T> *newnode = new ListNode<T>(value, NULL);
        this->last->set_next(newnode);
        this->last = newnode;

        this->size++;
    }
}

template <typename T>
void List<T>::push(T value) {
    this->insert_last(value);
}

template <typename T>
void List<T>::print() {
    printf("[ ");
    for(ListNode<T> *node = this->first ; node != NULL ; node = node->get_next()) {
        std::cout << node->get_value() << " ";
    }
    printf("]\n");
}

template <typename T>
bool List<T>::contains(T value) {
    for(ListNode<T> *node = this->first ; node != NULL ; node = node->get_next()) {
        if(node->get_value() == value) {
            return 1;
        }
    }
    return 0;
}

template <typename T>
T List<T>::get(int index) {
    int i = 0;
    for(ListNode<T> *node = this->first ; node != NULL ; node = node->get_next()) {
        if(i == index) {
            return node->get_value();
        }
        i++;
    }

    // if index is out of range
    printf("[ERROR] List index [%d] out of range\n", index);
    exit(1);
}

template <typename T>
void List<T>::remove_at(int index) {
    int i = 0;
    ListNode<T> *temp;
    for(ListNode<T> *node = this->first ; node != NULL ; node = node->get_next()) {
        if(i == index) {
            if(node == this->first) {       // node is first of list
                this->first = node->get_next();
                if(node == this->last) this->last = NULL;
                delete node;
                size--;
                return;
            }
            // node is any other node
            temp->set_next(node->get_next());

            if(node == this->last) this->last = temp;
            delete node;
            size--;
            return;
        }
        temp = node;
        i++;
    }
}

template <typename T>
int List<T>::index_of(T value) {
    int i = 0;
    for(ListNode<T> *node = this->first ; node != NULL ; node = node->get_next()) {
        if(node->get_value() == value) {
            return i;
        }
        i++;
    }
    return -1;
}

template <typename T>
void List<T>::set(int index, T value){
    int i = 0;
    for(ListNode<T> *node = this->first ; node != NULL ; node = node->get_next()) {
        if(i == index) {
            node->set_value(value);
            return;
        }
        i++;
    }
    // if index is out of range
    printf("[ERROR] List index [%d] out of range\n", index);
    exit(1);
}