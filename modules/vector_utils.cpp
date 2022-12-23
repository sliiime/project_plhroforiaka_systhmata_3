#include "vector_utils.hpp"
#include <cstdint>
#include <cstdlib>

uint pow2Ceil(uint num){
    uint max = 0x80000000;
    if (num > max) throw "pow2Ceil: overflow";

    uint i = 1 ;
    while (i < num) i*=2;
    return i;

}