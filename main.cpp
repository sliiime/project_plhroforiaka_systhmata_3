#include <iostream>
#include <fstream>
#include <time.h>
#include <pthread.h>
#include <chrono>

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
#include "filtering.hpp"
#include "jsch.hpp"
#include "optimizer.hpp"

int main(){

    Vector<Relation> relations;

	//Reads batch of queries and stores them to queries vector
    std::string workloadPath;
    std::getline(std::cin,workloadPath);    
    std::ifstream in(workloadPath);

    //Read filenames of relations from stdin
    Utils::readRelations(std::cin,relations);
    

    Vector<Relation*> relationPtrs = Vector<Relation*>();
    for (uint i = 0 ; i < relations.get_size(); i++){
        relationPtrs.push(&relations[i]);
    }

    jsch::JobScheduler jobScheduler(50);
    OperationManager operationManager(&relationPtrs);
    const size_t THREADS_PER_QUERY = 4;

    jsch::JobPlan* main = jobScheduler.makeJobPlan();
    jsch::JobSequence* print = NULL;
    jsch::JobPlan* current = main;

    Vector<QueryInfo> queries;

    while (Utils::readQueryBatch(in,queries)){
        

        if (print != NULL){
            current = jobScheduler.makeJobPlan();
            print->then(current);
        }

        const size_t TOTAL_QUERIES = queries.get_size();

        Vector<QueryInfo*> queryPtrs = Vector<QueryInfo*>();
        for (uint i = 0 ; i < queries.get_size(); i++){
            queryPtrs.push(&queries[i]);
        }

        Optimizer optimizer(&relationPtrs, &queryPtrs);
        optimizer.Optimize();

        Result** results = new Result*[TOTAL_QUERIES];

        print = jobScheduler.makeJobSequence(); 


        for (uint i = 0; i < TOTAL_QUERIES; i++){
            
            current->addRequiredJob( jsch::make_job(
                [=,&jobScheduler,&operationManager]() mutable {
                    results[i] = operationManager.ExecuteAndReturn(&queries[i],jobScheduler,THREADS_PER_QUERY);
                }
            ));

            print->then( jsch::make_job(
                [=,&operationManager]() mutable {
                    operationManager.printColumnProjection(queries[i].selections,results[i]);
                }

            ));

        }

        print->then (
            jsch::make_job(
                [=]() mutable {
                    for (int i = 0 ; i < TOTAL_QUERIES; i++) delete results[i];
                    
                    delete[] results;
                }
            )
        );

        current->addDependentJob(print);

        queries.clear();

    }

    auto start = chrono::high_resolution_clock::now();
    jsch::Future* future = jobScheduler.submitJobWithFuture(main);
    future->wait();

    auto end = chrono::high_resolution_clock::now();

    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
    std::cout << "Queries calculated in : " << duration.count() << " μ/s" << std::endl;

    return 0;

}
