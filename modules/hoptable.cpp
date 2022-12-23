#include <assert.h>
#include "hoptable.hpp"
#include "hoodsize.hpp"
#include "iterator.hpp"
#include "list_neighbourhood.hpp"
#include <string.h>
#include "utils.hpp"

Hoptable::Hoptable(uint32_t capacity){
    this->capacity = capacity;
    this->size = 0 ;
    this->hashFunction = &Utils::hopSuperFastHash;
    this->entries = new Entry[capacity];
    this->neighbourhoods = new Neighbourhood*[capacity];
    for (uint32_t i = 0 ; i < capacity; i++) this->neighbourhoods[i] = NULL;
    
}


Hoptable::Hoptable(){
    this->capacity = STARTING_CAPACITY;
    this->size = 0;
    this->hashFunction = &Utils::hopSuperFastHash;
    this->entries = new Entry[capacity];
    this->neighbourhoods = new Neighbourhood*[capacity];
    for (uint32_t i = 0 ; i < capacity; i++) this->neighbourhoods[i] = NULL;
}

Hoptable::Hoptable(uint32_t (*hashFunction)(int32_t)){
    this->capacity = STARTING_CAPACITY;
    this->size = 0;
    this->hashFunction = hashFunction;
    this->entries = new Entry[capacity];
    this->neighbourhoods = new Neighbourhood*[capacity];
    for (uint32_t i = 0 ; i < capacity; i++) this->neighbourhoods[i] = NULL;
}

uint32_t Hoptable::get_size() const{
    return this->size;
}

void Hoptable::insert(int32_t key,uint32_t value){
    try{
        
        insert_try(key,value);

    }catch(RehashingException& e){

        rehash();

        try{

            insert_try(key,value);

        }
        catch(RehashingException& e){
            std::cerr << "Insertion failed after rehashing. Exiting..." << '\n';
            throw e;
        }
    }

}

const Vector<uint32_t>* Hoptable::lookup(int32_t key){
    uint32_t hash = hashFunction(key) % capacity;


    for (uint32_t i = 0; i < HOOD; i++){
        uint32_t index = (hash + i) % capacity;
        if (entries[index].key_is(key))return entries[index].get_bucket();
    }

    return NULL;
    
}


void Hoptable::insert_try(int32_t key, uint32_t value){

    try{

        uint32_t hash = hashFunction(key) % capacity;

        initializeNeighbourhood(hash);

        uint32_t pos = getPositionInNeighbourhood(hash,key);

        if (this->entries[pos].get_bucket_size() == 0){     //Entry has not been added as a Neighbour yet
            uint32_t dist = backwardsDistance(pos,hash);
            neighbourhoods[hash]->addNeighbour(dist);
        }

        assert(this->entries[pos].get_key() == key);
        this->entries[pos].insert_value(value);
        this->size++; 

    }catch(RehashingException& e){
        throw e;
    }

}


void Hoptable::insert_try(int32_t key,Vector<uint32_t>* bucket){
    try {


        uint32_t hash = hashFunction(key) % capacity;
        
        initializeNeighbourhood(hash);

        uint32_t pos = getPositionInNeighbourhood(hash,key);
        
        uint32_t dist = backwardsDistance(pos,hash);
        neighbourhoods[hash]->addNeighbour(dist);

        this->entries[pos].set_bucket(bucket);
        this->size += this->entries[pos].get_bucket_size();

    }catch(RehashingException& e){
        throw;
    }
}

uint32_t Hoptable::getPositionInNeighbourhood(uint32_t hash,int32_t key){


    uint32_t pos;

    try {

        BucketSearchStatus result = findNeighbourhoodEntry(hash,key,&pos);

        if (result == BucketSearchStatus::FOUND);
        else if (result == BucketSearchStatus::FREE) this->entries[pos].set_key(key);
        else if (result == BucketSearchStatus::NOT_FOUND){
                pos = findNextFreeEntry(hash+HOOD);
                pos = moveFreeEntryToNeighbourhood(pos,hash);
                this->entries[pos].set_key(key);
        }

    }catch(RehashingException& e){
        throw e;
    }

    return pos;
    
}

BucketSearchStatus Hoptable::findNeighbourhoodEntry(uint32_t index,int32_t key,uint32_t* bucket){


    if (neighbourhoods[index]->isFull()) throw RehashingException();
    
    bool set = false;
    uint32_t free_entry = index + HOOD;
    //If an entry for that key doesn't exist, return the first available entry that exists
    for (uint32_t i = 0; i < HOOD; i++){
            
            uint32_t current_index = (index + i) % this->capacity;
            if (this->entries[current_index].key_is(key)){
                *bucket = current_index;
                return BucketSearchStatus::FOUND;
            }

            if (!set && this->entries[current_index].is_free()) {
                free_entry = current_index; 
                set = true;    
            }
            
    }

    if (set){
        *bucket = free_entry;
         return BucketSearchStatus::FREE;
    }

    return BucketSearchStatus::NOT_FOUND;

}

uint32_t Hoptable::findNextFreeEntry(uint32_t index) const{

    for (uint32_t i = 0 ; i < this->capacity - HOOD; i++){
        if (this->entries[(index + i) % capacity].is_free()) return (index + i)%capacity;
            
    }
    
    throw RehashingException();
}

uint32_t Hoptable::moveFreeEntryToNeighbourhood(uint32_t from,uint32_t to){


    Entry freeEntry = this->entries[from];

    while (backwardsDistance(from,to)){

        uint32_t swapVictim = from;

        for (uint32_t i = 1 ; i < HOOD ; i++){
            uint32_t neighbourhoodIndex = from >= HOOD - i ? from - HOOD + i : this->capacity + from - HOOD + i;

            uint32_t distanceFromNeighbourhood = HOOD - i;
            
            uint32_t neighbourIndex = HOOD;
            if (this->neighbourhoods[neighbourhoodIndex] != NULL)
             neighbourIndex = this->neighbourhoods[neighbourhoodIndex]->replaceNearestNeighbour(distanceFromNeighbourhood); //returns index of the replaced neighbour or Neighbourhood.capacity if neighbour was empty

            if (neighbourIndex == HOOD) continue; //Neighbourhood contains no neighbours
            else {
                swapVictim = (neighbourhoodIndex + neighbourIndex) % this->capacity;
                break;
            }
        }

        if (backwardsDistance(from,swapVictim) > HOOD - 1 || swapVictim == from){
            this->entries[from].set_bucket(freeEntry.get_bucket());
            freeEntry.set_bucket(NULL);
            // this->entries[from] = freeEntry;
            throw RehashingException();
        } //return 0
        else{
            this->entries[from] = this->entries[swapVictim];
            from = swapVictim;
        }
    }

    this->entries[from] = freeEntry;
    freeEntry.set_bucket(NULL);


    return from;
}

void Hoptable::printNeighbourhood(uint32_t i) const{
    if (this->neighbourhoods[i] != NULL){
        
        Iterator* it = this->neighbourhoods[i]->begin();
        Iterator* end = this->neighbourhoods[i]->end();

        while (it->operator!=(*end)){
            std::cout << it->operator*() << std::endl;
            it->operator++();
        }
        
        delete it;
        delete end;
    }
}

uint32_t Hoptable::backwardsDistance(uint32_t from, uint32_t to) const{

    return from >= to ? from - to : from + this->capacity - to;


}

void Hoptable::destroy(){
    if (this->entries != NULL) delete[] this->entries;

    if (this->neighbourhoods != NULL){
        for (uint32_t i = 0; i <capacity;i++)
            if (this->neighbourhoods[i] != NULL) delete this->neighbourhoods[i];

        delete[] this->neighbourhoods;

    }

}

void Hoptable::rehash(){
     Hoptable resized(capacity*2);
     resized.hashFunction = hashFunction;

     for (uint32_t i = 0; i < capacity; i++) {
         if (this->entries[i].get_bucket_size() > 0){
            resized.insert_try(this->entries[i].get_key(),this->entries[i].get_bucket());
            this->entries[i].set_bucket(NULL);
         }
    }

    
    for (uint i = 0; i < capacity; i++){
        if (this->neighbourhoods[i] != NULL) 
            delete this->neighbourhoods[i];
    }

    delete[] this->neighbourhoods;
    delete[] this->entries;

    this->capacity *=2;
    this->entries = resized.entries;
    this->neighbourhoods = resized.neighbourhoods;
    

    //Resized is a stack allocated instance
    resized.entries = NULL;
    resized.neighbourhoods = NULL;


}

void Hoptable::initializeNeighbourhood(uint32_t index){
    if (this->neighbourhoods[index] == NULL) this->neighbourhoods[index] = new ListNeighbourhood();
        
}

Hoptable::~Hoptable(){
    destroy();
}

void Hoptable::print(){
    for (uint i = 0 ; i < capacity ; i++) {
        if (entries[i].get_bucket() != NULL){
            printf("%d: %d ", i, entries[i].get_key());
            entries[i].get_bucket()->print();
        }
    }
}

Hoptable::Hoptable(uint32_t capacity,uint32_t (*hf)(int32_t)){
        this->capacity = capacity;
        this->hashFunction = hf;
}

uint32_t Hoptable::get_capacity(){
    return this->capacity;
}

const Entry& Hoptable::get_entry(uint32_t index){
    if (index >= capacity) throw std::out_of_range("index is out of range");

    return this->entries[index];
}

const Neighbourhood* Hoptable::get_neighbourhood(uint32_t index){
    if (index >= capacity) throw std::out_of_range("index is out of range");

    return this->neighbourhoods[index];

}
