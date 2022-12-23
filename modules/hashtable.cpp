#include "hashtable.hpp"

HashTable::HashTable(int size){
    this->size = size;
    this->relation = new Vector<Bucket>(size);
    for(int i = 0; i < size; i++){
        this->relation->set(i, new Vector<int>());
    }
}

HashTable::~HashTable(){
    for(int i = 0; i < this->size; i++){
        delete this->relation->get(i);
    }
    delete this->relation;
}

int HashTable::hash(int key){
    return key % this->size;
}

void HashTable::Insert(int key, int value){
    int hash = this->hash(key);
    Bucket bucket = relation->get(hash);
    bucket->push(value);
}

Bucket HashTable::GetBucket(int index){
    return relation->get(index);
}

void HashTable::Print(){
    for(int i = 0; i < this->size; i++){
        printf("%d: ", i);
        GetBucket(i)->print();
    }
}

Bucket HashTable::GetKey(int key){
    int hash = this->hash(key);
    Bucket bucket = GetBucket(hash);
    return bucket;
}

void HashTable::Rehash(int new_size){

    // Delete old relation
    for (int i = 0; i < this->size; i++){
        delete this->relation->get(i);
    }
    delete this->relation;

    // Initiate new relation
    size = new_size;
    this->relation = new Vector<Bucket>(size);
    for(int i = 0; i < size; i++){
        this->relation->set(i, new Vector<int>());
    }

    // TODO [Low] Maybe rehash old value here

}