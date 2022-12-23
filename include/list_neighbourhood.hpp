#ifndef __LIST_NEIGHBOURHOOD_H__
#define __LIST_NEIGHBOURHOOD_H__

#include "neighbourhood.hpp"
#include "hoodsize.hpp"
#include "iterator.hpp"
#include "queue.hpp"
#include "lniterator.hpp"

class ListNeighbourhood : public Neighbourhood {


    private :
        Queue<uint32_t> neighbourhood;
        uint32_t capacity = HOOD;       //should be changed imo


    public :

    ListNeighbourhood();
    ~ListNeighbourhood() = default;

    void addNeighbour(uint32_t index) override;
    bool isFull() override;
    bool removeNeighbour(uint32_t index) override;
    uint32_t replaceNearestNeighbour(uint32_t value) override;
    Iterator* begin() override;
    Iterator* end() override;
   

};

#endif

