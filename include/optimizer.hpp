#pragma once
#include "relation.hpp"
#include "vector.hpp"
#include "config.hpp"

struct ColumnStatistics{
    int lowerBound;
    int upperBound;
    int totalValues;
    int distinctValues;
};


class RelationStatistics{
private:
    Vector<ColumnStatistics> stats;
    Relation *relation;
    int rowCount;
    int columnCount;

public:
    RelationStatistics(Relation *relation);
    int LowerBound(int column);
    int UpperBound(int column);
    int TotalValues(int column);
    int DistinctValues(int column);
    void Print();
};

class Optimizer{

    struct Statistics;

private:
    Vector<Relation *> *relations;

public:
    Optimizer(Vector<Relation*> *relations);
    void Optimize();
};