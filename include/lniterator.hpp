#ifndef __LN_ITERATOR_H__
#define __LN_ITERATOR_H__

#include "iterator.hpp"
#include "list.hpp"

class ListNeighbourhoodIterator : public Iterator {
    
    private :

        ListNode<uint32_t>* node;
    
    public :

        ListNeighbourhoodIterator(ListNode<uint32_t>* node);

        void operator++() override;
        uint32_t operator*() override;
        bool operator!=(Iterator& i) override;

        virtual ~ListNeighbourhoodIterator() override;
};

#endif