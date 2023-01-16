#pragma once
#include <cmath>
#include "relation.hpp"
#include "vector.hpp"
#include "config.hpp"
#include "parser.hpp"
#include "list.hpp"

struct ColumnStatistics{
    double lowerBound;
    double upperBound;
    double totalValues;
    double distinctValues;
};

class RelationStatistics{
private:
    Vector<ColumnStatistics> stats;
    Relation *relation;
    int rowCount;
    int columnCount;
    double cost;

public:
    // Constructor that creates statistics for a relation
    RelationStatistics(Relation *relation);

    // Constructor that copies statistics from another relation statistics object
    RelationStatistics(RelationStatistics *rs);

    // Create stats concatenating two other stats
    RelationStatistics(RelationStatistics *rs1, RelationStatistics *rs2);

    // Empty constructor
    RelationStatistics(){};

    double LowerBound(int column);
    double UpperBound(int column);
    double TotalValues(int column);
    double DistinctValues(int column);

    void LowerBound(int column, double value);
    void UpperBound(int column, double value);
    void TotalValues(int column, double value);
    void DistinctValues(int column, double value);

    int RowCount();
    int ColumnCount();
    Relation* GetRelation();
    double GetCost();

    void Print();
};

class BestTree{

private:
    Vector<RelationStatistics*> stats;
    Vector<Vector<uint>> orderedIDs;
    Vector<Vector<PredicateInfo *>> orderedPredicates;
    int hash(Vector<uint> relationIDs);

public:
    ~BestTree();
    Vector<PredicateInfo *> GetPredicateOrder(Vector<uint> orderedIDs);
    void SetBestTree(Vector<uint> joinIDs, Vector<uint> orderedIDs, Vector<PredicateInfo *> *predInfo);
    static Vector<Vector<uint>> GetCombinations(Vector<uint> ids, int n);
    RelationStatistics *GetStats(Vector<uint> relationIDs);
    double GetCost(Vector<uint> relationIDs);
    void SetStats(Vector<uint> joinIDs, RelationStatistics *relStats);
    void Print();

};

class Optimizer{

private:
    Vector<Relation *> *relations;
    Vector<QueryInfo *> *queries;
    Vector<RelationStatistics *> *allRelationStats;

    void OptimizeFilterEqual(RelationStatistics *rs, int column, double value);
    RelationStatistics OptimizeFilterLessGreater(RelationStatistics *rs, int column, double value, bool less, bool inplace);
    void OptimizeSelfJoin(RelationStatistics *rs, int column1, int column2);
    RelationStatistics *OptimizeJoin(RelationStatistics *rs1, RelationStatistics *rs2, int column1, int column2);

public:
    Optimizer(Vector<Relation*> *relations, Vector<QueryInfo *> *queries);
    ~Optimizer();
    void Optimize();
    // void Print(); // TODO [Medium] : Print function for optimizer
};