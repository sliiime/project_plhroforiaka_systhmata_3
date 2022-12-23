#pragma once
#include <stdio.h>
#include <cstdlib>
#include <cstdint>
#include "relation.hpp"
#include "vector.hpp"
#include "parser.hpp"
#include "list.hpp"


#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)+(uint32_t)(((const uint8_t *)(d))[0]) )
#define MIN(a,b) (a) > (b) ? (b) : (a)


class Utils{
public:

    static void PrintBinary(int);
    
    static uint GetLastBits(uint number, uint n_bits);

    static uint Square(int number);

    static uint32_t superFastHash(const char* data,int len); //http://www.azillionmonkeys.com/qed/hash.html
    static uint32_t hopSuperFastHash(int32_t);
    static uint32_t identityHash(int32_t);
    static void readRelations(std::istream& in,Vector<Relation>& relations);
    static void readQueryBatch(std::istream&in,Vector<QueryInfo>& queryBatch);
    static Vector<PredicateInfo*> findJoinSequence(Vector<PredicateInfo>& predicates);


    template<typename T>
    static bool vector_contains(const Vector<T>& v,T value);

    template<typename T>
    static size_t vector_findIndex(const Vector<T>& v ,T value);

    static void print_sortedPredicates(const Vector<PredicateInfo*> predicates);


};
