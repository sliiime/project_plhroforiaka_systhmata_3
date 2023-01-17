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
#include "job_plan.hpp"
#include "job_sequence.hpp"
#include "job_spec.hpp"
#include <pthread.h>
#include <chrono>
#include "optimizer.hpp"

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

    size_t TOTAL_WORKERS;
    size_t THREADS_PER_QUERY;
    size_t TOTAL_QUERIES;
    bool debug = 0;
    if (debug){ 
        TOTAL_WORKERS = 20;
        THREADS_PER_QUERY = 3;
        TOTAL_QUERIES = 5;
    }else {
        TOTAL_WORKERS = queries.get_size()*4;
        THREADS_PER_QUERY = queries.get_size()*3;
        TOTAL_QUERIES = queries.get_size();
    }



    jsch::JobScheduler jobScheduler(TOTAL_WORKERS);

    // Create pointers to relations
    Vector<Relation*> relationPtrs = Vector<Relation*>();
    for (uint i = 0 ; i < relations.get_size(); i++){
        relationPtrs.push(&relations[i]);
    }

    // Create pointers to relations
    Vector<QueryInfo*> queryPtrs = Vector<QueryInfo*>();
    for (uint i = 0 ; i < queries.get_size(); i++){
        queryPtrs.push(&queries[i]);
    }

    // Optimize query order

    Optimizer optimizer(&relationPtrs, &queryPtrs);
    optimizer.Optimize();

    // Execute queries
    auto start = chrono::high_resolution_clock::now();
    OperationManager operationManager(&relationPtrs);

    jsch::JobPlan* exec = jobScheduler.makeJobPlan();
    jsch::JobSequence* print = jobScheduler.makeJobSequence();

    Result* results[queries.get_size()];


    for (uint i = 0 ; i < TOTAL_QUERIES; i++){
        //Calculate queries
        exec->addRequiredJob(
            jsch::make_job([&,i]{
                results[i] = operationManager.ExecuteAndReturn(&queries[i],jobScheduler,THREADS_PER_QUERY);
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

    for (uint i = 0; i < TOTAL_QUERIES; i++) delete results[i];
    printf("\n");
    auto end = chrono::high_resolution_clock::now();

    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
    std::cout << "Time taken using JobScheduler with multithreading : " << duration.count() << std::endl;

    return 0;

}
