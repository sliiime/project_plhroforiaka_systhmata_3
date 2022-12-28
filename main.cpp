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

    delete operationManager;

    return 0;

}
