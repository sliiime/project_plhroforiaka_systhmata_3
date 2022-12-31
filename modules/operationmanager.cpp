#include "operationmanager.hpp"
#include "relation.hpp"
#include "utils.hpp"
#include "parser.hpp"
#include "fstream"


long OperationManager::ProjectColumn(const Column &column){
    long result = 0;
    for (int i = 0; i < column.num_tuples; i++){
        result += column.tuples[i][1];
    }
    return result;
}

OperationManager::OperationManager(Vector<Relation *> *relations){
    this->relations = relations;
}

OperationManager::~OperationManager(){
    
}


void OperationManager::Execute(QueryInfo *queryInfo){

    Result *result = new Result();

    //// Add Relations ////

    Vector<unsigned> *relationIds = &(queryInfo->relationIds);
    for (uint i = 0; i < relationIds->get_size(); i++){
        RelationId relationId = relationIds->get(i);
        Relation *relation = relations->get(relationId);
        result->AddRelation(relation);
    }


    if (DEBUG) std::cout << "Relations added to Result" << std::endl;
    if (DEBUG) result->Print();
    
    //// Filter Relations on constants ////

    Vector<FilterInfo> *filters = &(queryInfo->filters);

    for (uint i = 0; i < filters->get_size(); i++){


        FilterInfo filterInfo = filters->get(i);
        uint colId = filterInfo.filterColumn.colId;
        uint relId = filterInfo.filterColumn.binding;
        uint64_t constant = filterInfo.constant;
        FilterInfo::Comparison comparison = filterInfo.comparison;

        if (DEBUG) printf("[Operation] Applying constant filter: %d.%d %c %ld\n", relId, colId, comparison, constant);

        Column column = result->GetColumn(relId, colId);

        IDVector ids = Filter::FilterConstant(column, comparison, constant);

        result->SetVector(relId, ids);
    }


    if (DEBUG) std::cout << "Constant Filters have been applied" << std::endl;
    if (DEBUG) result->Print();

    // result->Print();


    // Get Predicates
    Vector<PredicateInfo> *predicates = &(queryInfo->predicates);

    
    //// Seperate Predicates ////

    Vector<PredicateInfo> *joinPredicates = new Vector<PredicateInfo>();
    Vector<PredicateInfo> *filterPredicates = new Vector<PredicateInfo>();

    for (uint i = 0; i < predicates->get_size(); i++){
        PredicateInfo predicateInfo = predicates->get(i);
        if (predicateInfo.isSelfJoin()){
            filterPredicates->push(predicateInfo);
        } else {
            joinPredicates->push(predicateInfo);
        }
    }


    //// Filter Relations on predicates ////

    for (uint i = 0; i < filterPredicates->get_size(); i++){
        PredicateInfo predicateInfo = filterPredicates->get(i);
        uint leftColId = predicateInfo.left.colId;
        uint leftRelId = predicateInfo.left.binding;
        uint rightColId = predicateInfo.right.colId;
        uint rightRelId = predicateInfo.right.binding;

        if (DEBUG) printf("[Operation] Applying self join filter: %d.%d = %d.%d\n", leftRelId, leftColId, rightRelId, rightColId);

        if (leftRelId != rightRelId){
            printf("[ERROR] Filtering on different relations is not supported");
            exit(1);
        }

        Column leftColumn = result->GetColumn(leftRelId, leftColId);
        Column rightColumn = result->GetColumn(rightRelId, rightColId);

        IDVector ids = Filter::FilterPredicate(leftColumn, rightColumn);

        result->SetVector(leftRelId, ids);
    }

    if (DEBUG) std::cout << "Self joins have been applied" << std::endl;
    if (DEBUG) result->Print();


    //// Join Relations ////

    // Sort joins
    Vector<PredicateInfo *> sortedJoinPredicates;
    
    if (joinPredicates->get_size() > 0)
        sortedJoinPredicates = Utils::findJoinSequence(*joinPredicates);

    for (uint i = 0; i < sortedJoinPredicates.get_size(); i++){
        PredicateInfo predicateInfo = *sortedJoinPredicates.get(i);
        uint leftColId = predicateInfo.left.colId;
        uint leftRelId = predicateInfo.left.binding;
        uint rightColId = predicateInfo.right.colId;
        uint rightRelId = predicateInfo.right.binding;

        if (!result->IsJoined(leftRelId) && result->IsJoined(rightRelId)){
            if (DEBUG) printf("[Operation] Swapping...\n");
            uint temp = leftColId;
            leftColId = rightColId;
            rightColId = temp;
            temp = leftRelId;
            leftRelId = rightRelId;
            rightRelId = temp;
        }


        if (DEBUG) printf("[Operation] Applying join: %d.%d & %d.%d\n", leftRelId, leftColId, rightRelId, rightColId);

        Column leftColumn = result->GetColumn(leftRelId, leftColId);
        Column rightColumn = result->GetColumn(rightRelId, rightColId);

        if (result->IsJoined(leftRelId) && result->IsJoined(rightRelId)){
            if (DEBUG) printf("[Operation] Joining two already joined relations\n");
            IDVector ids = Filter::FilterPredicate(leftColumn, rightColumn);
            result->SetJoint(ids);
        }
        else {
            Joint *joint = new Joint(&leftColumn, &rightColumn);
            if (DEBUG) printf("[Operation] Joint of size %d created\n", joint->GetTupleCount());
            result->SetJoint(leftRelId, rightRelId, joint);
            delete joint;
        }

        if (DEBUG) result->Print();

    }

    if (DEBUG) std::cout << "Joins have been applied" << std::endl;

    delete joinPredicates;
    delete filterPredicates;


    //// Project Columns ////

    Vector<SelectInfo> *selects = &(queryInfo->selections);

    for (uint i = 0; i < selects->get_size(); i++){
        SelectInfo selectInfo = selects->get(i);
        uint colId = selectInfo.colId;
        uint relId = selectInfo.binding;

        Column column = result->GetColumn(relId, colId);

        long sum = ProjectColumn(column);

        if (sum != 0)
            printf("%ld ", sum);
        else
            printf("NULL ");
    }

    printf("\n");
    if (DEBUG) printf("\n");


    delete result;

}

void OperationManager::Execute(QueryInfo *queryInfo,jsch::JobScheduler& jobScheduler){

    Result *result = new Result();

    //// Add Relations ////

    Vector<unsigned> *relationIds = &(queryInfo->relationIds);
    for (uint i = 0; i < relationIds->get_size(); i++){
        RelationId relationId = relationIds->get(i);
        Relation *relation = relations->get(relationId);
        result->AddRelation(relation);
    }


    if (DEBUG) std::cout << "Relations added to Result" << std::endl;
    if (DEBUG) result->Print();
    
    //// Filter Relations on constants ////

    Vector<FilterInfo> *filters = &(queryInfo->filters);

    for (uint i = 0; i < filters->get_size(); i++){


        FilterInfo filterInfo = filters->get(i);
        uint colId = filterInfo.filterColumn.colId;
        uint relId = filterInfo.filterColumn.binding;
        uint64_t constant = filterInfo.constant;
        FilterInfo::Comparison comparison = filterInfo.comparison;

        if (DEBUG) printf("[Operation] Applying constant filter: %d.%d %c %ld\n", relId, colId, comparison, constant);

        Column column = result->GetColumn(relId, colId);

        IDVector ids = Filter::FilterConstant(column, comparison, constant);

        result->SetVector(relId, ids);
    }


    if (DEBUG) std::cout << "Constant Filters have been applied" << std::endl;
    if (DEBUG) result->Print();

    // result->Print();


    // Get Predicates
    Vector<PredicateInfo> *predicates = &(queryInfo->predicates);

    
    //// Seperate Predicates ////

    Vector<PredicateInfo> *joinPredicates = new Vector<PredicateInfo>();
    Vector<PredicateInfo> *filterPredicates = new Vector<PredicateInfo>();

    for (uint i = 0; i < predicates->get_size(); i++){
        PredicateInfo predicateInfo = predicates->get(i);
        if (predicateInfo.isSelfJoin()){
            filterPredicates->push(predicateInfo);
        } else {
            joinPredicates->push(predicateInfo);
        }
    }


    //// Filter Relations on predicates ////

    for (uint i = 0; i < filterPredicates->get_size(); i++){
        PredicateInfo predicateInfo = filterPredicates->get(i);
        uint leftColId = predicateInfo.left.colId;
        uint leftRelId = predicateInfo.left.binding;
        uint rightColId = predicateInfo.right.colId;
        uint rightRelId = predicateInfo.right.binding;

        if (DEBUG) printf("[Operation] Applying self join filter: %d.%d = %d.%d\n", leftRelId, leftColId, rightRelId, rightColId);

        if (leftRelId != rightRelId){
            printf("[ERROR] Filtering on different relations is not supported");
            exit(1);
        }

        Column leftColumn = result->GetColumn(leftRelId, leftColId);
        Column rightColumn = result->GetColumn(rightRelId, rightColId);

        IDVector ids = Filter::FilterPredicate(leftColumn, rightColumn);

        result->SetVector(leftRelId, ids);
    }

    if (DEBUG) std::cout << "Self joins have been applied" << std::endl;
    if (DEBUG) result->Print();


    //// Join Relations ////

    // Sort joins
    Vector<PredicateInfo *> sortedJoinPredicates;
    
    if (joinPredicates->get_size() > 0)
        sortedJoinPredicates = Utils::findJoinSequence(*joinPredicates);

    for (uint i = 0; i < sortedJoinPredicates.get_size(); i++){
        PredicateInfo predicateInfo = *sortedJoinPredicates.get(i);
        uint leftColId = predicateInfo.left.colId;
        uint leftRelId = predicateInfo.left.binding;
        uint rightColId = predicateInfo.right.colId;
        uint rightRelId = predicateInfo.right.binding;

        if (!result->IsJoined(leftRelId) && result->IsJoined(rightRelId)){
            if (DEBUG) printf("[Operation] Swapping...\n");
            uint temp = leftColId;
            leftColId = rightColId;
            rightColId = temp;
            temp = leftRelId;
            leftRelId = rightRelId;
            rightRelId = temp;
        }


        if (DEBUG) printf("[Operation] Applying join: %d.%d & %d.%d\n", leftRelId, leftColId, rightRelId, rightColId);

        Column leftColumn = result->GetColumn(leftRelId, leftColId);
        Column rightColumn = result->GetColumn(rightRelId, rightColId);

        if (result->IsJoined(leftRelId) && result->IsJoined(rightRelId)){
            if (DEBUG) printf("[Operation] Joining two already joined relations\n");
            IDVector ids = Filter::FilterPredicate(leftColumn, rightColumn);
            result->SetJoint(ids);
        }
        else {
            Joint *joint = new Joint(&leftColumn, &rightColumn,jobScheduler);
            if (DEBUG) printf("[Operation] Joint of size %d created\n", joint->GetTupleCount());
            result->SetJoint(leftRelId, rightRelId, joint);
            delete joint;
        }

        if (DEBUG) result->Print();

    }

    if (DEBUG) std::cout << "Joins have been applied" << std::endl;

    delete joinPredicates;
    delete filterPredicates;


    //// Project Columns ////

    Vector<SelectInfo> *selects = &(queryInfo->selections);

    for (uint i = 0; i < selects->get_size(); i++){
        SelectInfo selectInfo = selects->get(i);
        uint colId = selectInfo.colId;
        uint relId = selectInfo.binding;

        Column column = result->GetColumn(relId, colId);

        long sum = ProjectColumn(column);

        if (sum != 0)
            printf("%ld ", sum);
        else
            printf("NULL ");
    }

    printf("\n");
    if (DEBUG) printf("\n");


    delete result;

}
