#include "utils.hpp"
#include <assert.h>
#include <stdio.h>
#include <cstdint>
#include "relation.hpp"
#include "parser.hpp"
#include "vector.hpp"
#include "list.hpp"
#include "queue.hpp"

Vector<PredicateInfo*> coverAllGraph(const Vector<unsigned int>& nodes,Vector<PredicateInfo>& edges);

uint32_t Utils::superFastHash (const char * data, int len) {
uint32_t hash = len, tmp;
int rem;

    if (len <= 0 || data == NULL) return 0;

    rem = len & 3;
    len >>= 2;

    /* Main loop */
    for (;len > 0; len--) {
        hash  += get16bits (data);
        tmp    = (get16bits (data+2) << 11) ^ hash;
        hash   = (hash << 16) ^ tmp;
        data  += 2*sizeof (uint16_t);
        hash  += hash >> 11;
    }

    /* Handle end cases */
    switch (rem) {
        case 3: hash += get16bits (data);
                hash ^= hash << 16;
                hash ^= ((signed char)data[sizeof (uint16_t)]) << 18;
                hash += hash >> 11;
                break;
        case 2: hash += get16bits (data);
                hash ^= hash << 11;
                hash += hash >> 17;
                break;
        case 1: hash += (signed char)*data;
                hash ^= hash << 10;
                hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}

uint32_t Utils::hopSuperFastHash(int32_t key){
    return superFastHash((char*)(&key),sizeof(int32_t));
}

uint32_t Utils::identityHash(int32_t key){
        return (uint32_t)key;
}



void Utils::PrintBinary(int n){
    int i;
    for (i = 31; i >= 0; i--)
        (n & (1 << i)) ? printf("1") : printf("0");
    printf("\t");
}

uint Utils::GetLastBits(uint number, uint n_bits) {
    return number & ((1 << n_bits) - 1);
}

uint Utils::Square(int number) {
    return 1 << number;
}

void Utils::readRelations(std::istream& in,Vector<Relation>& relations){


    std::string fileName;

    for (std::getline(in,fileName); fileName != "Done"; std::getline(in,fileName)) 
        relations.push(Relation(fileName.c_str()));
    
}

bool Utils::readQueryBatch(std::istream& in,Vector<QueryInfo>& queryBatch){
        
        bool flag = false;
        std::string rawQuery;
        while (std::getline(in,rawQuery)){
            flag = true;
            if (rawQuery == "F") break;
            queryBatch.push(QueryInfo(rawQuery));
        }

        return flag;
        
}

Vector<PredicateInfo*> Utils::findJoinSequence(Vector<PredicateInfo>& predicates){
    Vector<unsigned int> nodes;

    //Create a vector that contains all the relations used in the predicates (unique values)
    for (uint i = 0 ; i < predicates.get_size(); i++){

        if (!vector_contains(nodes,predicates[i].left.binding)) nodes.push(predicates[i].left.binding);
        if (!vector_contains(nodes,predicates[i].right.binding)) nodes.push(predicates[i].right.binding);
    }


    //Find a sequence to apply the joins in such a way that only one new relation is added per join
    Vector<PredicateInfo*> sortedPredicates = coverAllGraph(nodes,predicates);


    //Adding self joins to the first positions of the vector
    // if (sortedPredicates.get_size() != 0){
    //     for (uint i = 0; i < predicates.get_size();i++){
    //         if (predicates[i].isSelfJoin()) mergedSortedPredicates.push(&predicates[i]);
    //     }
    // }

    //Adding sorted non self joins with self joins
    


    return sortedPredicates;


}



Vector<PredicateInfo*> coverAllGraph(const Vector<unsigned int>& nodes,Vector<PredicateInfo>& edges){
    
    const size_t TOTAL_NODES = nodes.get_size();

    bool visited[TOTAL_NODES] = {0};
    List<PredicateInfo*> activeEdges[TOTAL_NODES][TOTAL_NODES];

    
    for (uint i = 0 ; i < edges.get_size(); i++){
        //TODO [High] Handle case where no index is returned
        uint left  = Utils::vector_findIndex(nodes,edges[i].left.binding);
        uint right = Utils::vector_findIndex(nodes,edges[i].right.binding);

        activeEdges[left][right].push(&edges[i]);
        activeEdges[right][left].push(&edges[i]);
    }


    Queue<uint> toVisit;

    if (nodes.get_size() > 0) toVisit.push(0);

    Vector<PredicateInfo*> sortedPredicates;


    while ( toVisit.get_size() > 0){

        uint currentNode = toVisit.pop();
        visited[currentNode] = 1;

        for (uint i = 0; i < TOTAL_NODES; i++){
            
                uint totalEdges = activeEdges[currentNode][i].get_size();
                for (uint j = 0; j < totalEdges; j++){
                    
                    PredicateInfo* predicateInfo = activeEdges[currentNode][i].get(0);
                    //Self joins must not be taken into consideration
                    if (!predicateInfo->isSelfJoin()) sortedPredicates.push(predicateInfo);
                    //In case of self two edges are removed from the current activeEdges List
                    else totalEdges--;

                    //Must not use this predicate again
                    activeEdges[currentNode][i].remove(predicateInfo);
                    activeEdges[i][currentNode].remove(predicateInfo);
                    
                    //Relation that "initiates" the join must always be on the left
                    if (predicateInfo->left.binding != currentNode){
                        SelectInfo temp = predicateInfo->left;
                        predicateInfo->left = predicateInfo->right;
                        predicateInfo->right = temp;
                    }

                    if (!visited[i]) toVisit.push(i);
                    
                }
        }
    }

    bool allVisited = true;
    for (size_t i = 0; i < TOTAL_NODES; i++){
        if (visited[i] == 0){
            allVisited = false;
            break;
        }
    }

    if (allVisited) return sortedPredicates;
    
    sortedPredicates.clear();
    return sortedPredicates;

    


}


template<typename T>
bool Utils::vector_contains(const Vector<T>& v,T value){

    for (uint i = 0; i < v.get_size(); i++) 
        if (v[i] == value) 
            return true;
    
    return false;

}

template<typename T>
size_t Utils::vector_findIndex(const Vector<T>& v, T value){
    for (uint i = 0; i < v.get_size(); i++){
        if (v[i] == value)
            return i;
    }

   assert(false && "vector_findIndex failed");
   return std::string::npos;

}
void Utils::print_sortedPredicates(const Vector<PredicateInfo*> sortedPredicates){

    if (sortedPredicates.get_size() > 0) std::cout << sortedPredicates[0]->toString();
    else std::cout << "[EMPTY]";
    for (uint i = 1; i < sortedPredicates.get_size(); i++){
        std::cout << " -> " << sortedPredicates[i]->toString();
    }
    std::cout << std::endl;
}



