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

// #define BENCHMARK

int main(){

    Vector<Relation> relations;
    Vector<QueryInfo> queries;




    //Read filenames of relations from stdin
    Utils::readRelations(std::cin,relations);
    
	//Reads batch of queries and stores them to queries vector
    std::ifstream in("workloads/small/all.work");
    Utils::readQueryBatch(in,queries);
    /* 
    std::ifstream inPublic("workloads/public/public.work");
	Utils::readQueryBatch(inPublic,queries);
    */

    const size_t TOTAL_WORKERS = 6*queries.get_size();
    jsch::JobScheduler jobScheduler(TOTAL_WORKERS);

    // Execute queries
    Vector<Relation*> relationPtrs = Vector<Relation*>();
    for (uint i = 0 ; i < relations.get_size(); i++){
        relationPtrs.push(&relations[i]);
    }

    auto start = chrono::high_resolution_clock::now();
    OperationManager operationManager(&relationPtrs);

    jsch::JobScheduler::JobPlan* exec = jobScheduler.makeJobPlan();
    jsch::JobScheduler::JobSequence* print = jobScheduler.makeJobSequence();

    Result* results[queries.get_size()];



    for (uint i = 0 ; i < queries.get_size(); i++){
        //Calculate queries
        exec->addRequiredJob(
            jsch::make_job([&,i]{
                results[i] = operationManager.ExecuteAndReturn(&queries[i],jobScheduler,5);
            })
        );
        //Print results of each query
        print->then(
            jsch::make_job([&,i]{
                operationManager.printColumnProjection(queries[i].selections,results[i]);
            })
        );
    }

    exec->addDependentJob(print);

    jsch::Future* future = jobScheduler.submitJobWithFuture(exec);
    future->wait();

    for (uint i = 0; i < queries.get_size(); i++) delete results[i];
    printf("\n");
    auto end = chrono::high_resolution_clock::now();

    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
    std::cout << "Time taken using JobScheduler with multithreading : " << duration.count() << std::endl;

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
