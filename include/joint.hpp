#pragma once
#include "vector.hpp"
#include "tuple.hpp"
#include "column.hpp"
#include "partitionmanager.hpp"
#include "hoptable.hpp"

class Joint{

private:
    Vector<Tuple> *tuples;
    void AddTuple(Tuple t);
public:
    Joint(Column *c1, Column *c2);
    ~Joint();
    Tuple GetTuple(int i);
    int GetTupleCount();
    void Print();
};