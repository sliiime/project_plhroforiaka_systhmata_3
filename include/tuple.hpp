#pragma once
#include <stdint.h>
#include <stdio.h>

class Tuple {

public:

    int key;
    int payload;

Tuple();
Tuple(int, int);
void Print();
~Tuple();
int operator[](int index);

};