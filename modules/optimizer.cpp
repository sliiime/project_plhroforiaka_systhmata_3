#include "optimizer.hpp"


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


Optimizer::Optimizer(Vector<Relation*> *relations){
    this->relations = relations;
}



void Optimizer::Optimize(){

    // Calculate statistics for each relation
    for(uint i = 0; i < relations->get_size(); i++){
        RelationStatistics relationStatistics(relations->get(i));
    }

    


}

