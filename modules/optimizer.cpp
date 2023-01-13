#include "optimizer.hpp"


// ======= Relation Statistics ======= //

RelationStatistics::RelationStatistics(Relation* relation){
    this->relation = relation;
    rowCount = relation->row_count();
    columnCount = relation->column_count();

    // Calculate statistics for each column
    for(int col = 0; col < columnCount; col++){
        ColumnStatistics columnStatistics;

        columnStatistics.lowerBound = relation->get(0,col);
        columnStatistics.upperBound = relation->get(0,col);
        columnStatistics.distinctValues = 0;
        columnStatistics.totalValues = 0;

        // Iterate through all rows
        for (int i=0; i<rowCount; i++){
            int value = relation->get(i,col);

            // Update lower and upper bounds
            if (value < columnStatistics.lowerBound) columnStatistics.lowerBound = value;
            if (value > columnStatistics.upperBound) columnStatistics.upperBound = value;

        }

        // Update total values (count)
        columnStatistics.totalValues = rowCount;

        // Update distinct values

        int max_array_length = MAX_BOOL_ARRAY_LENGTH;
        int array_length = std::min(max_array_length, int(columnStatistics.upperBound - columnStatistics.lowerBound + 1));

        Vector<bool> distinctValues = Vector<bool>(array_length);

        // Initialize array
        for (int i=0; i<array_length; i++){
            distinctValues.set(i, false);
        }

        // Iterate through all rows
        for (int i=0; i<rowCount; i++){
            int value = relation->get(i,col);
            int index = int(value - columnStatistics.lowerBound) % array_length;
            distinctValues.set(index, true);
        }

        // Count distinct values
        for (int i=0; i<array_length; i++){
            if (distinctValues[i]) columnStatistics.distinctValues++;
        }

        // Append to relation statistics
        stats.push(columnStatistics);
    }

    this->cost = 0;

    
}

RelationStatistics::RelationStatistics(RelationStatistics *rs){
    this->relation = rs->relation;
    this->rowCount = rs->rowCount;
    this->columnCount = rs->columnCount;
    this->stats = rs->stats;
    this->cost = rs->cost;
}

RelationStatistics::RelationStatistics(RelationStatistics *rs1, RelationStatistics *rs2){
    this->relation = NULL;
    this->rowCount = rs1->rowCount;
    this->columnCount = rs1->columnCount + rs2->columnCount;
    this->stats = rs1->stats;
    for (int i=0; i<rs2->columnCount; i++){
        this->stats.push(rs2->stats[i]);
    }
    this->cost = rs1->cost + rs2->cost + rs1->TotalValues(0) + rs2->TotalValues(0);
}


double RelationStatistics::LowerBound(int column){
    return stats[column].lowerBound;
}
double RelationStatistics::UpperBound(int column){
    return stats[column].upperBound;
}
double RelationStatistics::DistinctValues(int column){
    return stats[column].distinctValues;
}
double RelationStatistics::TotalValues(int column){
    return stats[column].totalValues;
}
void RelationStatistics::LowerBound(int column, double value){
    stats[column].lowerBound = value;
}
void RelationStatistics::UpperBound(int column, double value){
    stats[column].upperBound = value;
}
void RelationStatistics::DistinctValues(int column, double value){
    stats[column].distinctValues = std::max(0.01, value);
}
void RelationStatistics::TotalValues(int column, double value){
    stats[column].totalValues = std::max(0.01, value);
}
int RelationStatistics::RowCount(){
    return rowCount;
}
int RelationStatistics::ColumnCount(){
    return columnCount;
}
double RelationStatistics::GetCost(){
    return cost;
}
void RelationStatistics::Print(){
    printf("Relation Statistics (%dx%d):\n", rowCount, columnCount);
    printf("  %-7s %-15s %-15s %-15s %-15s\n", "Column", "Lower bound", "Upper bound", "Distinct values", "Total values");
    for (int i=0; i<columnCount; i++){
        printf("  %-7d %-15.2f %-15.2f %-15.2f %-15.2f\n", i, stats[i].lowerBound, stats[i].upperBound, stats[i].distinctValues, stats[i].totalValues);
    }
    std::cout << std::endl;
}
Relation* RelationStatistics::GetRelation(){
    return relation;
}










// ======= BestTree ======= //

int BestTree::hash(Vector<uint> joinIDs){
    int hash = -1;
    for (uint i=0; i<joinIDs.get_size(); i++){
        hash += pow(2, joinIDs[i]);
    }
    return hash;
}


RelationStatistics *BestTree::GetStats(Vector<uint> joinIDs){
    int hash = BestTree::hash(joinIDs);
    if (hash >= (int)stats.get_size() || stats[hash] == NULL) return NULL;
    return stats[hash];
}

Vector<uint> BestTree::GetBestTree(Vector<uint> ids){
    int hash = BestTree::hash(ids);
    if (hash >= (int)this->orderedIDs.get_size() || hash < 0){
        ids.print();
        printf("[ERROR] BestTree::GetBestTree: Hash %d out of bounds. Size of vector: %u\n", hash, this->orderedIDs.get_size());
        exit(1);
    }
    return this->orderedIDs[hash];
}

void BestTree::SetBestTree(Vector<uint> joinIDs, Vector<uint> orderedIDs){
    int hash = BestTree::hash(joinIDs);
    if (hash >= (int)this->orderedIDs.get_size() || hash < 0){
        printf("[ERROR] BestTree::SetBestTree: Hash %d out of bounds. Size of vector: %u\n", hash, this->orderedIDs.get_size());
        exit(1);
    }
    this->orderedIDs[hash] = orderedIDs;
}

void BestTree::SetStats(Vector<uint> joinIDs, RelationStatistics *relStats){
    int index = hash(joinIDs);


    // Resize Vector if necessary
    while (index >= (int)stats.get_size()){
        stats.push(NULL);
        orderedIDs.push(Vector<uint>());
    }


    if (stats.get_size() != orderedIDs.get_size()){
        printf("[ERROR] stats and orderedIDs have different sizes (%d vs %d)", stats.get_size(), orderedIDs.get_size());
        exit(1);
    }

    stats[index] = relStats;
}

Vector<Vector<uint>> BestTree::GetCombinations(Vector<uint> ids, int n){
    
    // Create combinations of size n
    Vector<Vector<uint>> combinations;
    if (n == 1){
        for (uint i=0; i<ids.get_size(); i++){
            Vector<uint> combination;
            combination.push(ids[i]);
            combinations.push(combination);
        }
    } else {
        for (uint i=0; i<ids.get_size(); i++){
            Vector<uint> newIDs;
            for (uint j=i+1; j<ids.get_size(); j++){
                newIDs.push(ids[j]);
            }
            Vector<Vector<uint>> newCombinations = GetCombinations(newIDs, n-1);
            for (uint j=0; j<newCombinations.get_size(); j++){
                newCombinations[j].push(ids[i]);
                combinations.push(newCombinations[j]);
            }
        }
    }
    return combinations;
}

double BestTree::GetCost(Vector<uint> joinIDs){
    int index = hash(joinIDs);
    if (index >= (int)stats.get_size() || stats[index] == NULL) return -1;
    double cost = stats[index]->GetCost();
    return cost;
}

void BestTree::Print(){
    printf("\nBestTree:\n");
    for (uint i=0; i<stats.get_size(); i++){
        if (stats[i] == NULL) continue;
        printf("Index %d:\n", i);
        printf("  BestTree: ");
        orderedIDs[i].print();
        printf("  Cost: %f\n", stats[i]->TotalValues(0));
    }
}







// ======= Optimizer ======= //


void Optimizer::OptimizeFilterEqual(RelationStatistics *rs, int column, double value){

    // Calculate new statistics for column
    double A_lowerBound = value;
    double A_upperBound = value;
    double A_distinctValues = 1;
    double A_totalValues;


    if (rs->GetRelation()->columns[column].contains(value)){
        A_totalValues = rs->TotalValues(column) / rs->DistinctValues(column);
    } else {
        A_totalValues = 0;
    }



    // Caclulate new statistics for other columns
    for (int i=0; i< rs->ColumnCount(); i++){
        if (i == column) continue;

        // int dA = rs->DistinctValues(column);
        double dC = rs->DistinctValues(i);
        double fA = rs->TotalValues(column);
        double fC = rs->TotalValues(i);
        double fA_new = A_totalValues;

        double C_distinctValues = dC * (1 - pow((1 - fA_new / fA), fC/dC) );
        double C_totalValues = fA_new;

        rs->DistinctValues(i, C_distinctValues);
        rs->TotalValues(i, C_totalValues);
    }


    
    // Update statistics of column
    rs->LowerBound(column, A_lowerBound);
    rs->UpperBound(column, A_upperBound);
    rs->DistinctValues(column, A_distinctValues);
    rs->TotalValues(column, A_totalValues);

}

RelationStatistics Optimizer::OptimizeFilterLessGreater(RelationStatistics *rs, int column, double value, bool less, bool inplace=true){

    RelationStatistics new_rs; 

    if (!inplace){
        new_rs = *rs;
        rs = &new_rs;
    }

    // Calculate new statistics for column
    double A_lowerBound;
    double A_upperBound;
    double A_distinctValues;
    double A_totalValues;

    double k1;
    double k2;

    if (less){
        A_lowerBound = rs->LowerBound(column);
        A_upperBound = value;
    } else {
        A_lowerBound = value;
        A_upperBound = rs->UpperBound(column);
    }

    if (less){
        k1 = A_lowerBound;
        k2 = std::min(A_upperBound, value);
    }else{
        k1 = std::max(A_lowerBound, value);
        k2 = A_upperBound;
    }

    A_distinctValues = (k2 - k1) / (rs->UpperBound(column) - rs->LowerBound(column) ) * rs->DistinctValues(column);
    A_totalValues = (k2 - k1) / (rs->UpperBound(column) - rs->LowerBound(column) ) * rs->TotalValues(column);


    // Caclulate new statistics for other columns
    for (int i=0; i< rs->ColumnCount(); i++){
        if (i == column) continue;

        
        // double dA = rs->DistinctValues(column);
        double dC = rs->DistinctValues(i);
        double fA = rs->TotalValues(column);
        double fC = rs->TotalValues(i);
        double fA_new = A_totalValues;

        double C_distinctValues = dC * (1 - pow((1 - fA_new / fA), fC/dC) );
        double C_totalValues = fA_new;

        rs->DistinctValues(i, C_distinctValues);
        rs->TotalValues(i, C_totalValues);

    }

    // Update statistics of column
    rs->LowerBound(column, A_lowerBound);
    rs->UpperBound(column, A_upperBound);
    rs->DistinctValues(column, A_distinctValues);
    rs->TotalValues(column, A_totalValues);

    return *rs;

}

void Optimizer::OptimizeSelfJoin(RelationStatistics *rs, int column1, int column2){
    
    // Calculate new statistics for columns
    double A_lowerBound;
    double A_upperBound;
    double A_distinctValues;
    double A_totalValues;


    A_lowerBound = std::max(rs->LowerBound(column1), rs->LowerBound(column2));
    A_upperBound = std::min(rs->UpperBound(column1), rs->UpperBound(column2));

    double n = A_upperBound - A_lowerBound + 1;

    A_totalValues = rs->TotalValues(column1) / n;
    A_distinctValues = rs->DistinctValues(column1) * (1 -  pow ( (1 - A_totalValues/rs->TotalValues(column1)), rs->TotalValues(column1)/rs->DistinctValues(column1)));

    // Caclulate new statistics for other columns
    for (int i=0; i< rs->ColumnCount(); i++){
        if (i == column1 || i == column2) continue;

        double dC = rs->DistinctValues(i);
        double fA = rs->TotalValues(column1);
        double fC = rs->TotalValues(i);
        double fA_new = A_totalValues;

        double C_distinctValues = dC * (1 - pow((1 - fA_new / fA), fC/dC) );
        double C_totalValues = fA_new;

        rs->DistinctValues(i, C_distinctValues);
        rs->TotalValues(i, C_totalValues);
    }

    // Update statistics of column1
    rs->LowerBound(column1, A_lowerBound);
    rs->UpperBound(column1, A_upperBound);
    rs->DistinctValues(column1, A_distinctValues);
    rs->TotalValues(column1, A_totalValues);

    // Update statistics of column2
    rs->LowerBound(column2, A_lowerBound);
    rs->UpperBound(column2, A_upperBound);
    rs->DistinctValues(column2, A_distinctValues);
    rs->TotalValues(column2, A_totalValues);
}




RelationStatistics *Optimizer::OptimizeJoin(RelationStatistics *rs1, RelationStatistics *rs2, int column1, int column2){

    // Create new tuples for each relation

    double l = std::max(rs1->LowerBound(column1), rs2->LowerBound(column2));
    double u = std::min(rs1->UpperBound(column1), rs2->UpperBound(column2));
    double n = u - l + 1;

    RelationStatistics A = OptimizeFilterLessGreater(rs1, column1, u, true, false);
    RelationStatistics B = OptimizeFilterLessGreater(rs2, column2, u, true, false);

    OptimizeFilterLessGreater(&A, column1, l, false, true);
    OptimizeFilterLessGreater(&B, column2, l, false, true);

    double A_lowerBound;
    double A_upperBound;
    double A_distinctValues;
    double A_totalValues;

    double B_lowerBound;
    double B_upperBound;
    double B_distinctValues;
    double B_totalValues;

    A_lowerBound = B_lowerBound = l;
    A_upperBound = B_upperBound = u;
    A_totalValues = B_totalValues = A.TotalValues(column1) * B.TotalValues(column2) / n;
    A_distinctValues = B_distinctValues = A.DistinctValues(column1) * B.DistinctValues(column2) / n;


    // Calculate statistics for other columns
    for (int i=0; i< rs1->ColumnCount(); i++){
        if (i == column1) continue;
        double C_lowerBound = A.LowerBound(i);
        double C_upperBound = A.UpperBound(i);
        double C_totalValues = A_totalValues;

        double dC = A.DistinctValues(i);
        double dA_new = A_distinctValues;
        double dA = A.DistinctValues(column1);
        double fC = A.TotalValues(i);
        double C_distinctValues = dC * (1 - pow((1 - dA_new / dA), fC/dC) );

        // Apply new statistics
        A.LowerBound(i, C_lowerBound);
        A.UpperBound(i, C_upperBound);
        A.DistinctValues(i, C_distinctValues);
        A.TotalValues(i, C_totalValues);

    }
    for (int i=0; i< rs2->ColumnCount(); i++){
        if (i == column2) continue;
        double C_lowerBound = B.LowerBound(i);
        double C_upperBound = B.UpperBound(i);
        double C_totalValues = B_totalValues;

        double dC = B.DistinctValues(i);
        double dB_new = B_distinctValues;
        double dB = B.DistinctValues(column2);
        double fC = B.TotalValues(i);
        double C_distinctValues = dC * (1 - pow((1 - dB_new / dB), fC/dC) );

        // Apply new statistics
        B.LowerBound(i, C_lowerBound);
        B.UpperBound(i, C_upperBound);
        B.DistinctValues(i, C_distinctValues);
        B.TotalValues(i, C_totalValues);
    }

    // Update statistics of A
    A.LowerBound(column1, A_lowerBound);
    A.UpperBound(column1, A_upperBound);
    A.DistinctValues(column1, A_distinctValues);
    A.TotalValues(column1, A_totalValues);

    // Update statistics of B
    B.LowerBound(column2, B_lowerBound);
    B.UpperBound(column2, B_upperBound);
    B.DistinctValues(column2, B_distinctValues);
    B.TotalValues(column2, B_totalValues);

    // Concatenate A and B
    RelationStatistics *rs = new RelationStatistics(&A, &B);

    return rs;

}

Optimizer::Optimizer(Vector<Relation *> *relations, Vector<QueryInfo *> *queries){
    this->relations = relations;
    this->queries = queries;
    this->allRelationStats = new Vector<RelationStatistics *>();
    this->allQueryStats = new Vector<Vector<RelationStatistics *> *>();
}

Optimizer::~Optimizer(){
    // Free memory
    for (uint i=0; i<allRelationStats->get_size(); i++){
        delete allRelationStats->get(i);
    }
    delete allRelationStats;
    for (uint i=0; i<allQueryStats->get_size(); i++){
        Vector<RelationStatistics *> *qs = allQueryStats->get(i);
        for (uint j=0; j<qs->get_size(); j++){
            delete qs->get(j);
        }
        delete qs;
    }
    delete allQueryStats;
}












void Optimizer::Optimize(){

    //// Calculate statistics for each relation
    for(uint i = 0; i < relations->get_size(); i++){
        RelationStatistics *rs = new RelationStatistics(relations->get(i));
        allRelationStats->push(rs);
    }


    //// Initialize query statistics
    for (uint i=0; i<queries->get_size(); i++){
        Vector<RelationStatistics *> *qs = new Vector<RelationStatistics *>();

        // Get query
        QueryInfo *queryInfo = queries->get(i);
        
        // Add relation statistics to query
        Vector<unsigned> *relationIds = &(queryInfo->relationIds);
        for (uint i = 0; i < relationIds->get_size(); i++){
            RelationId relationId = relationIds->get(i);
            RelationStatistics *rs = new RelationStatistics(allRelationStats->get(relationId));
            qs->push(rs);
        }
        
        // Add query statistics to vector
        allQueryStats->push(qs);

    }

    //// Optimize each query
    for (uint i=0; i<queries->get_size(); i++){
        QueryInfo *queryInfo = queries->get(i);
        Vector<RelationStatistics *> *qs = allQueryStats->get(i);

        // Optimize each filter
        for (uint j=0; j<queryInfo->filters.get_size(); j++){
            
            // Get filter
            FilterInfo filterInfo = queryInfo->filters[j];

            // Get relation statistics
            RelationStatistics *rs = qs->get(filterInfo.filterColumn.binding);

            // Optimize filter A = k
            if (filterInfo.comparison == FilterInfo::Equal){
                OptimizeFilterEqual(rs, filterInfo.filterColumn.colId, filterInfo.constant);
            }
            // Optimize filter A < k
            else if (filterInfo.comparison == FilterInfo::Less){
                OptimizeFilterLessGreater(rs, filterInfo.filterColumn.colId, filterInfo.constant, true);
            }
            // Optimize filter A > k
            else if (filterInfo.comparison == FilterInfo::Greater){
                OptimizeFilterLessGreater(rs, filterInfo.filterColumn.colId, filterInfo.constant, false);
            }
        }
    }

    //// Optimize self joins

    // For each query
    for (uint i=0; i<queries->get_size(); i++){
        QueryInfo *queryInfo = queries->get(i);
        Vector<RelationStatistics *> *qs = allQueryStats->get(i);

        // Optimize each join
        for (uint j=0; j<queryInfo->predicates.get_size(); j++){

            // Get predicate
            PredicateInfo predicateInfo = queryInfo->predicates[j];

            // Get relation statistics
            RelationStatistics *rs1 = qs->get(predicateInfo.left.binding);

            // Optimize self join
            if (predicateInfo.left.binding == predicateInfo.right.binding){
                OptimizeSelfJoin(rs1, predicateInfo.left.colId, predicateInfo.right.colId);
            }
        
        }

    }


    //// Calculate cost tree for each query ////


    // For each query
    for (uint i=0; i<queries->get_size(); i++){
        QueryInfo *queryInfo = queries->get(i);
        Vector<RelationStatistics *> *qs = allQueryStats->get(i);

        BestTree bestTree;

        for (uint j=0; j<queryInfo->relationIds.get_size(); j++){
            // Get relation statistics
            RelationStatistics *rs = qs->get(j);
            Vector<uint> v;
            v.push(j);
            bestTree.SetStats(v, rs);
            bestTree.SetBestTree(v, v);
        }

        Vector<PredicateInfo> all_predicates = queryInfo->predicates;
        Vector<PredicateInfo> predicates;
        Vector<uint> joinedIds;

        for (uint p=0; p<all_predicates.get_size(); p++){
            PredicateInfo predicate = all_predicates[p];
            if (!predicate.isSelfJoin()){
                predicates.push(predicate);
                if (!joinedIds.contains(predicate.left.binding))
                    joinedIds.push(predicate.left.binding);
                if (!joinedIds.contains(predicate.right.binding))
                    joinedIds.push(predicate.right.binding);
            }
        }


        // Create dummy vector for relation bindings
        Vector<uint> relationBindings;
        for (uint j=0; j<queryInfo->relationIds.get_size(); j++){
            relationBindings.push(j);
        }

        // for j cardinality of join
        for (uint j=1; j<queryInfo->relationIds.get_size(); j++){
            // for each combination of size j

            Vector<Vector<uint>> combinations = BestTree::GetCombinations(relationBindings, j);

            // printf("Combinations of size %d:\n", j);
            // for (int i=0; i<combinations.get_size(); i++){
            //     combinations[i].print();
            // }
            // return;

            for (uint c=0; c<combinations.get_size(); c++){
                Vector<uint> combination = combinations[c];

                if (bestTree.GetStats(combination) == NULL) continue;

                // for each relation not in combination
                for (uint k=0; k<queryInfo->relationIds.get_size(); k++){
                    if (combination.contains(k)) continue;


                    // if relation doesn't join with current combination then skip
                    bool connected = false;
                    int combinationColumn;
                    int relationColumn;
                    for (uint p=0; p<predicates.get_size(); p++){
                        PredicateInfo predicate = predicates[p];

                        // printf("Predicate:");
                        // predicate.print();
                        // printf("\n");

                        if (combination.contains(predicate.left.binding) && k == predicate.right.binding){
                            connected = true;
                            combinationColumn = predicate.left.colId;
                            relationColumn = predicate.right.colId;
                            break;
                        }
                        else if (combination.contains(predicate.right.binding) && k == predicate.left.binding){
                            connected = true;
                            combinationColumn = predicate.right.colId;
                            relationColumn = predicate.left.colId;
                            break;
                        }
                    }
                    if (!connected) continue;

                    // printf("Relation %d joins with combination ", k);
                    // combination.print();

                    // Get current tree
                    Vector<uint> currTree = bestTree.GetBestTree(combination);
                    currTree.push(k);

                    // printf("Current tree: ");
                    // currTree.print();
                    
                    Vector<uint> currCombination = combination;
                    currCombination.push(k);

                    // printf("Current combination: ");
                    // currCombination.print();

                    // Calculate cost of new tree
                    RelationStatistics *rs1 = bestTree.GetStats(combination);
                    Vector <uint> v;
                    v.push(k);
                    RelationStatistics *rs2 = bestTree.GetStats(v);

                    // printf("Combination stats:");
                    // combination.print();
                    // rs1->Print();
                    // printf("Relation stats:");
                    // v.print();
                    // rs2->Print();


                    RelationStatistics *rs3 = OptimizeJoin(rs1, rs2, combinationColumn, relationColumn);

                    // printf("New stats: ");
                    // currCombination.print();
                    // rs3->Print();

                    // Get cost of current tree
                    double currCost = bestTree.GetCost(currTree);

                    // Get cost of new tree
                    double newCost = rs3->GetCost();

                    // rs3->Print();

                    // printf("Cost of combination ");
                    // currCombination.print();
                    // printf(" is %f\n", newCost);

                    // printf("Cost of current tree ");
                    // currTree.print();
                    // printf(" is %f\n", currCost);

                    // If new tree is better then update best tree
                    if (newCost < currCost || currCost == -1){
                        bestTree.SetStats(currCombination, rs3);
                        bestTree.SetBestTree(currCombination, currTree);
                    }
                    // bestTree.Print();
                }
            }
        }

        // Change query order to match best tree

        Vector<uint> order = bestTree.GetBestTree(joinedIds);
        Vector<PredicateInfo> newPredicates;

        // Override querries
        for (uint i=0; i<predicates.get_size(); i++){
            PredicateInfo predicate = predicates[i];
            predicate.left.binding = order[i];
            predicate.right.binding = order[i+1];
            newPredicates.push(predicate);
        }

        queryInfo->predicates = newPredicates;

    }


}

