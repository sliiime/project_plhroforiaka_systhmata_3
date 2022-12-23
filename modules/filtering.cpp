#include "filtering.hpp"
#include <assert.h>

IDVector Filter::FilterConstant(Column &column, FilterInfo::Comparison op, int num) {
    IDVector filteredRowIDs = new Vector<int>;

    int columnLength = column.num_tuples;

    for( int i = 0 ; i < columnLength ; i++ ) {

        int rowID = column[i].key;
        int value = column[i].payload;

        bool compare;

        switch (op) {
            case FilterInfo::Comparison::Less:
                compare = (value < num);
                break;
            case FilterInfo::Comparison::Greater:
                compare = (value > num);
                break;
            case FilterInfo::Comparison::Equal:
                compare = (value == num);
                break;
            default:
                assert(false && "unimplemented comparison type");
        }   

        if (compare) filteredRowIDs->push(rowID);
    }
    
    return filteredRowIDs;
}


IDVector Filter::FilterPredicate(Column &leftColumn, Column &rightColumn) {
    IDVector filteredRowIDs = new Vector<int>;

    int relationLength = leftColumn.num_tuples;

    for( int i = 0 ; i < relationLength ; i++ ) {

        int rowID = leftColumn[i].key;
        
        assert(rowID == rightColumn[i].key && "columns are of different relation");

        int leftColumnValue = leftColumn[i].payload;
        int rightColumnValue = rightColumn[i].payload;

        if (leftColumnValue == rightColumnValue) filteredRowIDs->push(rowID);
    }

    return filteredRowIDs;
}
