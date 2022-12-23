#pragma once
#include "result.hpp"
#include "parser.hpp"
#include "column.hpp"
#include "tuple.hpp"

class Filter {
    public:

        static IDVector FilterConstant(Column &column, FilterInfo::Comparison op, int num);
        static IDVector FilterPredicate(Column &leftColumn, Column &rightColumn);
};