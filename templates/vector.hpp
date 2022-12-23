#pragma once
#include <cstdlib>
#include <iostream>
#include <string.h>
#include <string>
#include "vector_utils.hpp"

#define VECTOR_STARTING_CAPACITY 32

template <typename T>
class Vector{
private:
    uint capacity = VECTOR_STARTING_CAPACITY;
    uint size;
    T *data;

public:
    

    Vector();
    Vector(uint size);
    Vector(std::initializer_list<T> l);
    Vector(const Vector& vector);
    ~Vector();

    void push(const T& value);
    void push(T&& value);
    T get(uint) const;
    T& operator[](uint index);
    const T& operator[](uint index) const;
    Vector<T>& operator=(const Vector& v);
    Vector<T>& operator=(Vector&& v);

    /*
    Print vector with a space for separator as default
    */
    void print(std::string sep=" ") const;
    void set(uint, T);
    void resize(uint);
    void resize_and_empty(uint);
    uint get_size() const;
    void clear();
    inline uint get_capacity() const{return this->capacity;}



    /*
    Sort the vector using the selection sort algorithm
    */
    void sort();

    /*
    Does a vector contain a value
    */
    bool contains(T value);

    /*
    Get index of value
    */
    int index_of(T value);
    
};

template <typename T>
Vector<T>::Vector(){
    this->size = 0;
    this->data = new T[capacity];
}

template <typename T>
Vector<T>::Vector(uint size){
    this->size = 0;
    this->capacity = pow2Ceil(size);
    this->data = new T[capacity];
}

template <typename T>
Vector<T>::~Vector(){
    if (this->data != NULL)
        delete[] this->data;    
}

template <typename T>
void Vector<T>::push(const T& value){
    if (size == capacity){
        T *temp = new T[this->capacity*2];
        for(uint i = 0; i < this->size; i++){
            temp[i] = std::move(this->data[i]);
        }
        capacity*=2;

        delete[] this->data;
        this->data = temp;
    }
    
    data[size++] = value;
}

template <typename T>
void Vector<T>::push(T&& value){
    if (size == capacity){
        T *temp = new T[this->capacity*2];
        for(uint i = 0; i < this->size; i++){
            temp[i] = std::move(this->data[i]);
        }
        capacity*=2;

        delete[] this->data;
        this->data = temp;
    }
    data[size++] = std::move(value);
}

template <typename T>
T Vector<T>::get(uint index) const{
    if (index >= this->size){
        printf("[ERROR] Index out of bounds: %d, size: %d\n", index, this->size);
        throw std::out_of_range("Index out of bounds");
    }
    return this->data[index];
}

template <typename T>
T& Vector<T>::operator[](uint index){
    return this->data[index];
}
template <typename T>
const T& Vector<T>::operator[](uint index) const{
    return this->data[index];
}

template <typename T>
void Vector<T>::print(std::string sep) const {
    printf("[");
    for(uint i = 0; i < this->size; i++){
        std::cout << sep << this->data[i];
    }
    printf(" ]\n");
}

template <typename T>
void Vector<T>::set(uint index, T value){
    this->data[index] = value;
}

template <typename T>
void Vector<T>::resize(uint size){
    this->capacity = pow2Ceil(size);
    T *temp = new T[capacity];
    for(uint i = 0; i < size; i++){
        temp[i] = this->data[i];
    }
    delete[] this->data;
    this->data = temp;
}

template <typename T>
void Vector<T>::resize_and_empty(uint size){
    delete[] this->data;
    this->capacity = pow2Ceil(size);
    this->data = new T[capacity];
    this->size = 0;
}

template <typename T>
uint Vector<T>::get_size() const {
    return this->size;
}


template <typename T>
void Vector<T>::sort(){
    for(uint i = 0; i < this->size; i++){
        uint min_index = i;
        for(uint j = i + 1; j < this->size; j++){
            if(this->data[j] < this->data[min_index]){
                min_index = j;
            }
        }
        T temp = this->data[i];
        this->data[i] = this->data[min_index];
        this->data[min_index] = temp;
    }
}

template <typename T>
Vector<T>& Vector<T>::operator=(const Vector& v){
    this->size = v.size;
    this->capacity = v.capacity;
    this->data = new T[v.capacity];

    for (uint i = 0 ; i < size; i++) this->data[i] = v.data[i];

    return *(this);
}

template <typename T>
Vector<T>& Vector<T>::operator=(Vector&& v){

    delete[] this->data;
    
    this->size = v.size;
    this->capacity = v.capacity;
    this->data = v.data;

    v.data = NULL;

    return (*this);
}

template <typename T>
void Vector<T>::clear(){
    delete[] this->data;
    this->capacity = VECTOR_STARTING_CAPACITY;
    this->data = new T[capacity];
    this->size = 0;
}

template <typename T>
bool Vector<T>::contains(T value){
    for(uint i = 0; i < this->size; i++){
        if(this->data[i] == value){
            return true;
        }
    }
    return false;
}

template <typename T>
int Vector<T>::index_of(T value){
    for(uint i = 0; i < this->size; i++){
        if(this->data[i] == value){
            return i;
        }
    }
    return -1;
}

template <typename T>
Vector<T>::Vector(const Vector& vector){
    this->size = vector.size;
    this->capacity = vector.capacity;
    this->data = new T[vector.capacity];

    for (uint i = 0 ; i < size; i++) this->data[i] = vector.data[i]; 

}

template <typename T>
Vector<T>::Vector(std::initializer_list<T> l){
    this->size = l.size();
    this->capacity = pow2Ceil(size) > VECTOR_STARTING_CAPACITY ? pow2Ceil(size) : VECTOR_STARTING_CAPACITY;
    this->data = new T[capacity];
    uint itr = 0;
    for (auto item : l) this->data[itr++] = item; 
}

