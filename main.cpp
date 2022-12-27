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

int main(){

    Vector<Relation> relations;
    Vector<QueryInfo> queries;
    jsch::JobScheduler jobScheduler(15);

    std::ofstream out("out.txt");
    pthread_mutex_t writeMutex;
    pthread_mutex_init(&writeMutex,NULL);


    //Read filenames of relations from stdin
    Utils::readRelations(std::cin,relations);
    
	//Reads batch of queries and stores them to queries vector
    std::ifstream in("workloads/small/custom.work");
	Utils::readQueryBatch(in,queries);

    // Execute queries
    Vector<Relation*> relationPtrs = Vector<Relation*>();
    for (uint i = 0 ; i < relations.get_size(); i++){
        relationPtrs.push(&relations[i]);
    }

    OperationManager *operationManager = new OperationManager(&relationPtrs);
    for (uint i = 0 ; i < queries.get_size(); i++){
        operationManager->Execute(&queries[i]);
    }

        /*Submit 1000 jobs where each of them is submitting another 1000 jobs*/
        for (int j = 0 ; j < 1000; j++){
                jobScheduler.submitJob(jsch::make_job([&]{
                for (int i = 0 ; i < 1000; i++){
                    jobScheduler.submitJob(jsch::make_job([&,i]{
                        pthread_mutex_lock(&writeMutex);
                            out << pthread_self() << " " << i << std::endl;
                        pthread_mutex_unlock(&writeMutex);
                    }));
                }
                }));
        }


    jobScheduler.wait();
    out.close();
    delete operationManager;

    return 0;

}
