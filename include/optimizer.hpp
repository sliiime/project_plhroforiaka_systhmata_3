#pragma once
#include <cmath>
#include "relation.hpp"
#include "vector.hpp"
#include "config.hpp"
#include "parser.hpp"

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
    // Constructor that creates statistics for a relation
    RelationStatistics(Relation *relation);

    // Constructor that copies statistics from another relation statistics object
    RelationStatistics(RelationStatistics *rs);

    // Empty constructor
    RelationStatistics(){};

    int LowerBound(int column);
    int UpperBound(int column);
    int TotalValues(int column);
    int DistinctValues(int column);

    void LowerBound(int column, int value);
    void UpperBound(int column, int value);
    void TotalValues(int column, int value);
    void DistinctValues(int column, int value);

    int RowCount();
    int ColumnCount();
    Relation* GetRelation();

    void Print();
};

class Optimizer{

private:
    Vector<Relation *> *relations;
    Vector<QueryInfo *> *queries;
    Vector<RelationStatistics> allRelationStats;
    Vector< Vector<RelationStatistics> > allQueryStats;

    void OptimizeFilterEqual(RelationStatistics &rs, int column, int value);
    void OptimizeFilterLessGreater(RelationStatistics &rs, int column, int value, bool less);

public:
    Optimizer(Vector<Relation*> *relations, Vector<QueryInfo *> *queries);
    ~Optimizer();
    void Optimize();
    // void Print(); // TODO [Medium] : Print function for optimizer
};