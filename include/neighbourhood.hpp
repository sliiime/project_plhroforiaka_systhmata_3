#ifndef __NEIGHBOURHOOD_H__
#define __NEIGHBOURHOOD_H__

#include <iostream>
#include "iterator.hpp"

class Neighbourhood{

    public: 
        virtual void addNeighbour(uint32_t index) = 0;
        virtual bool removeNeighbour(uint32_t index) = 0;
        virtual bool isFull() = 0;
        virtual uint32_t replaceNearestNeighbour(uint32_t value) = 0;
        virtual Iterator* begin() = 0;
        virtual Iterator* end() = 0;
        virtual ~Neighbourhood(){}
};


#endif
