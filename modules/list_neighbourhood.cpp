#include "list_neighbourhood.hpp"
#include "hoodsize.hpp"
#include "lniterator.hpp"
#include <assert.h>

ListNeighbourhood::ListNeighbourhood():neighbourhood(){
    this->capacity = HOOD;
}

void ListNeighbourhood::addNeighbour(uint32_t relativeIndex){
    assert(relativeIndex < HOOD);
    assert(this->neighbourhood.get_size() < HOOD);
    
    this->neighbourhood.insert(relativeIndex);
}

bool ListNeighbourhood::isFull(){
    return this->neighbourhood.get_size() == capacity;
}

bool ListNeighbourhood::removeNeighbour(uint32_t index){
    return neighbourhood.remove(index);
}



uint32_t ListNeighbourhood::replaceNearestNeighbour(uint32_t value){
    
    assert(value < HOOD);

    return neighbourhood.get_size() == 0 ? HOOD : neighbourhood.pop_and_push(value);

}

Iterator* ListNeighbourhood::begin(){
    return new ListNeighbourhoodIterator(neighbourhood.begin());
}

Iterator* ListNeighbourhood::end(){
    return new ListNeighbourhoodIterator(NULL);
}



