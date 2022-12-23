#ifndef __ITERATOR_H__
#define __ITERATOR_H__

#include <stdint.h>

class Iterator{

    public:
        virtual void operator++() = 0;
        virtual uint32_t operator*() = 0;
        virtual bool operator!=(Iterator&) =0;
        virtual ~Iterator(){};
};

#endif