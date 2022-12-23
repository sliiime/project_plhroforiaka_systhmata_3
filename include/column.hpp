#pragma once
#include <stdint.h>
#include <cstdlib>
#include "tuple.hpp"
#include "relation.hpp"


class Column {

public:

    Tuple *tuples;
    int num_tuples;

    /*
    Print the column tuples
    */
    void Print();

    /*
    Frees the memory allocated for a column
    */
    void FreeTuples();

    /*
    Creates a column from a relation
    */
    Column(Relation *relation, uint columnID);

    /*
    Create an empty column of size num_tuples
    */
    Column(uint num_tuples);

    /*
    Create an empty column
    */
    Column();

    ~Column();

    /*
    Print the column with binary
    */
    void PrintBinary();

    /*
    Get the tuple at index i
    */
    Tuple operator[](int i);

    
};
