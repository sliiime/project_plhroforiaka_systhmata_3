#include "tuple.hpp"

Tuple::Tuple(){
    this->key = 0;
    this->payload = 0;
}

Tuple::Tuple(int key, int payload){
    this->key = key;
    this->payload = payload;
}

void Tuple::Print(){
    printf("(%d, %d)\n", this->key, this->payload);
}

Tuple::~Tuple(){
}

int Tuple::operator[](int index){
    if (index==0) return this->key;
    else if (index==1) return this->payload;
    throw "Index out of bounds";
    return -1;
}