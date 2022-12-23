#pragma once
#include "column.hpp"
#include "tuple.hpp"
#include "utils.hpp"


class Histogram {

public:

    Tuple *data;
    int size;

    /*
    Default Constructor
    */
    Histogram();

    /*
    Creates an empty histogram with a given size initialized to 0.
    */
    Histogram(int);

    /*
    Prints a histogram.
    */
    void Print();

    /*
    Prints a histogram with binary representation.
    */
    void PrintBinary();

    /*
    Gets the max value in the histogram.
    */
   int GetMax();

    /*
    Frees the memory allocated for a histogram.
    */
    ~Histogram();

    /*
    Returns the index of a key
    */
    int GetIndex(int key, int start=0);

};