#include "histogram.hpp"

Histogram::Histogram(int size){
    this->size = size;
    this->data = new Tuple[this->size];
    
    for(int i = 0; i < this->size; i++){
        this->data[i] = Tuple(i, 0);
    }
}

Histogram::Histogram(){
    this->size = 0;
    this->data = NULL;
}


void Histogram::Print(){
    printf("Histogram of size %d:\n", this->size);
    for(int i = 0; i < this->size; i++){
        printf("%d: ", i);
        data[i].Print();
    }
    printf("\n");
}

void Histogram::PrintBinary(){
    printf("Histogram of size %d:\n", this->size);
    for(int i = 0; i < this->size; i++){
        // printf("%d: %d\n", i, h->data[i]);
        // Utils::PrintBinary(i);
        // printf(": %d\n", this->data[i]);
    }
    printf("\n");
}

Histogram::~Histogram(){
    delete[] this->data;
}

int Histogram::GetMax(){
    int max = 0;
    for(int i = 0; i < this->size; i++){
        if(this->data[i].payload > max){
            max = this->data[i].payload;
        }
    }
    return max;
}

int Histogram::GetIndex(int key, int start){
    for(int i = start; i < this->size; i++){
        if(this->data[i].key == key){
            return i;
        }
    }
    return -1;
}