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
        int array_length = std::min(max_array_length, columnStatistics.upperBound - columnStatistics.lowerBound + 1);

        Vector<bool> distinctValues = Vector<bool>(array_length);

        // Initialize array
        for (int i=0; i<array_length; i++){
            distinctValues.set(i, false);
        }

        // Iterate through all rows
        for (int i=0; i<rowCount; i++){
            int value = relation->get(i,col);
            int index = (value - columnStatistics.lowerBound) % array_length;
            distinctValues.set(index, true);
        }

        // Count distinct values
        for (int i=0; i<array_length; i++){
            if (distinctValues[i]) columnStatistics.distinctValues++;
        }

        // Append to relation statistics
        stats.push(columnStatistics);
    }

    
}

RelationStatistics::RelationStatistics(RelationStatistics *rs){
    this->relation = rs->relation;
    this->rowCount = rs->rowCount;
    this->columnCount = rs->columnCount;
    this->stats = rs->stats;
}

int RelationStatistics::LowerBound(int column){
    return stats[column].lowerBound;
}
int RelationStatistics::UpperBound(int column){
    return stats[column].upperBound;
}
int RelationStatistics::DistinctValues(int column){
    return stats[column].distinctValues;
}
int RelationStatistics::TotalValues(int column){
    return stats[column].totalValues;
}
void RelationStatistics::LowerBound(int column, int value){
    stats[column].lowerBound = value;
}
void RelationStatistics::UpperBound(int column, int value){
    stats[column].upperBound = value;
}
void RelationStatistics::DistinctValues(int column, int value){
    stats[column].distinctValues = value;
}
void RelationStatistics::TotalValues(int column, int value){
    stats[column].totalValues = value;
}
int RelationStatistics::RowCount(){
    return rowCount;
}
int RelationStatistics::ColumnCount(){
    return columnCount;
}
void RelationStatistics::Print(){
    std::cout << "Relation Statistics" << std::endl;
    std::cout << "Row count: " << rowCount << std::endl;
    std::cout << "Column count: " << columnCount << std::endl;
    std::cout << "Column statistics: " << std::endl;
    for (int i=0; i<columnCount; i++){
        std::cout << "-- Column [" << i  << "] --" << std::endl;
        std::cout << "  Lower bound: " << stats[i].lowerBound << std::endl;
        std::cout << "  Upper bound: " << stats[i].upperBound << std::endl;
        std::cout << "  Distinct values: " << stats[i].distinctValues << std::endl;
        std::cout << "  Total values: " << stats[i].totalValues << std::endl;
    }
    std::cout << std::endl;
}
Relation* RelationStatistics::GetRelation(){
    return relation;
}


// ======= Optimizer ======= //


void Optimizer::OptimizeFilterEqual(RelationStatistics &rs, int column, int value){

    // Calculate new statistics for column
    int A_lowerBound = value;
    int A_upperBound = value;
    int A_distinctValues = 1;
    int A_totalValues;


    if (rs.GetRelation()->columns[column].contains(value)){
        A_totalValues = rs.TotalValues(column) / rs.DistinctValues(column);
    } else {
        A_totalValues = 0;
    }



    // Caclulate new statistics for other columns
    for (int i=0; i< rs.ColumnCount(); i++){
        if (i == column) continue;

        // int dA = rs.DistinctValues(column);
        int dC = rs.DistinctValues(i);
        int fA = rs.TotalValues(column);
        int fC = rs.TotalValues(i);
        int fA_new = A_totalValues;

        int C_distinctValues = dC * (1 - pow((1 - fA_new / fA), fC/dC) );
        int C_totalValues = fA_new;

        rs.DistinctValues(i, C_distinctValues);
        rs.TotalValues(i, C_totalValues);
    }


    
    // Update statistics of column
    rs.LowerBound(column, A_lowerBound);
    rs.UpperBound(column, A_upperBound);
    rs.DistinctValues(column, A_distinctValues);
    rs.TotalValues(column, A_totalValues);

}

void Optimizer::OptimizeFilterLessGreater(RelationStatistics &rs, int column, int value, bool less){

    // Calculate new statistics for column
    int A_lowerBound;
    int A_upperBound;
    int A_distinctValues;
    int A_totalValues;

    int k1;
    int k2;

    if (less){
        A_lowerBound = rs.LowerBound(column);
        A_upperBound = value;
    } else {
        A_lowerBound = value;
        A_upperBound = rs.UpperBound(column);
    }

    if (less){
        k1 = A_lowerBound;
        k2 = std::min(A_upperBound, value);
    }else{
        k1 = std::max(A_lowerBound, value);
        k2 = A_upperBound;
    }

    A_distinctValues = (k2 - k1) / (rs.UpperBound(column) - rs.LowerBound(column) ) * rs.DistinctValues(column);
    A_totalValues = (k2 - k1) / (rs.UpperBound(column) - rs.LowerBound(column) ) * rs.TotalValues(column);


    // Caclulate new statistics for other columns
    for (int i=0; i< rs.ColumnCount(); i++){
        if (i == column) continue;

        
        // int dA = rs.DistinctValues(column);
        int dC = rs.DistinctValues(i);
        int fA = rs.TotalValues(column);
        int fC = rs.TotalValues(i);
        int fA_new = A_totalValues;

        int C_distinctValues = dC * (1 - pow((1 - fA_new / fA), fC/dC) );
        int C_totalValues = fA_new;

        rs.DistinctValues(i, C_distinctValues);
        rs.TotalValues(i, C_totalValues);
    }

    // Update statistics of column
    rs.LowerBound(column, A_lowerBound);
    rs.UpperBound(column, A_upperBound);
    rs.DistinctValues(column, A_distinctValues);
    rs.TotalValues(column, A_totalValues);

}



Optimizer::Optimizer(Vector<Relation *> *relations, Vector<QueryInfo *> *queries){
    this->relations = relations;
    this->queries = queries;
}

Optimizer::~Optimizer(){
    // TODO [Critical] Free memory
}


void Optimizer::Optimize(){

    //// Calculate statistics for each relation
    for(uint i = 0; i < relations->get_size(); i++){
        RelationStatistics rs = RelationStatistics(relations->get(i));
        allRelationStats.push(rs);
    }


    //// Initialize query statistics
    for (uint i=0; i<queries->get_size(); i++){
        Vector<RelationStatistics> qs;

        // Get query
        QueryInfo *queryInfo = queries->get(i);
        
        // Add relation statistics to query
        Vector<unsigned> *relationIds = &(queryInfo->relationIds);
        for (uint i = 0; i < relationIds->get_size(); i++){
            RelationId relationId = relationIds->get(i);
            RelationStatistics rs = RelationStatistics(allRelationStats.get(relationId));
            qs.push(rs);
        }
        
        // Add query statistics to vector
        allQueryStats.push(qs);

    }


    //// Optimize each query
    for (uint i=0; i<queries->get_size(); i++){
        QueryInfo *queryInfo = queries->get(i);
        Vector<RelationStatistics> qs = allQueryStats.get(i);

        // Optimize each filter
        for (uint j=0; j<queryInfo->filters.get_size(); j++){
            
            // Get filter
            FilterInfo filterInfo = queryInfo->filters.get(j);

            // Get relation statistics
            RelationStatistics rs = qs.get(filterInfo.filterColumn.binding);

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

            
            // Update relation statistics
            qs.set(filterInfo.filterColumn.binding, rs);

        }

        // Update query statistics
        allQueryStats.set(i, qs);
    }

}

