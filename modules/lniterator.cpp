#include "lniterator.hpp"
#include "list.hpp"


ListNeighbourhoodIterator::ListNeighbourhoodIterator(ListNode<uint32_t>* node):node(node){}

void ListNeighbourhoodIterator::operator++(){
    this->node = node->get_next();
}

uint32_t ListNeighbourhoodIterator::operator*(){
    if (this->node != NULL) return this->node->get_value();
    else throw std::out_of_range("invalid access");
}

bool ListNeighbourhoodIterator::operator!=(Iterator& i){
    try{
        ListNeighbourhoodIterator& it = dynamic_cast<ListNeighbourhoodIterator&>(i);

        return it.node != this->node;
    }catch(std::bad_cast& e){
        return true;
    }catch(std::exception& e){
        throw e;
    }
}

ListNeighbourhoodIterator::~ListNeighbourhoodIterator(){ }