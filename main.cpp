#include <iostream>
#include "relation.hpp"
#include "parser.hpp"
#include "utils.hpp"
#include "vector.hpp"
#include "config.hpp"
#include "relation.hpp"
#include "column.hpp"
#include "result.hpp"
#include "parser.hpp"
#include "joint.hpp"
#include "operationmanager.hpp"
#include <fstream>
#include <time.h>
#include "filtering.hpp"
#include "jsch.hpp"
#include <pthread.h>
#include <chrono>
#include "optimizer.hpp"

#define BENCHMARK

int main(){

    Vector<Relation> relations;
    Vector<QueryInfo> queries;

    jsch::JobScheduler jobScheduler(10);


    //Read filenames of relations from stdin
    Utils::readRelations(std::cin,relations);
    
	//Reads batch of queries and stores them to queries vector
    std::ifstream in("workloads/small/small.work");
	Utils::readQueryBatch(in,queries);

    // Create pointers to relations
    Vector<Relation*> relationPtrs = Vector<Relation*>();
    for (uint i = 0 ; i < relations.get_size(); i++){
        relationPtrs.push(&relations[i]);
    }

    // Optimize query order
    Optimizer optimizer(&relationPtrs);

    optimizer.Optimize();



    return 0;

    // Execute queries
    auto start = chrono::high_resolution_clock::now();
    OperationManager operationManager(&relationPtrs);
    for (uint i = 0 ; i < queries.get_size(); i++){
        operationManager.Execute(&queries[i],jobScheduler);
    }
    auto end = chrono::high_resolution_clock::now();

    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
    std::cout << "Queries with JobScheduler : " << duration.count() << std::endl;

    #ifdef BENCHMARK
        start = chrono::high_resolution_clock::now();
        OperationManager operationManager2(&relationPtrs);
        for (uint i = 0 ; i < queries.get_size(); i++){
            operationManager2.Execute(&queries[i]);
        }
        end = chrono::high_resolution_clock::now();

        duration = chrono::duration_cast<chrono::microseconds>(end - start);
        std::cout << "Queries without JobScheduler : " << duration.count() << std::endl;
    #endif

    return 0;

}
