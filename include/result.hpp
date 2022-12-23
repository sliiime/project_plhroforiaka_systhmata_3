#pragma once
#include "vector.hpp"
#include "list.hpp"
#include "column.hpp"
#include "relation.hpp"
#include "config.hpp"
#include "joint.hpp"

typedef Vector<int>* IDVector;

class Result{

private:
    void assert_size();
    List<int> *nonjoined_ids;
    List<int> *joined_ids;
    List<IDVector> *nonjoined;
    List<IDVector> *joined;
    Vector<Relation *> *relations;
public:
    /*
    Create a result
    */
    Result();
    /*
    Destructor
    */
    ~Result();

    /*
    Create column using incrementing ids from either the nonjoined or joined vectors
    @param relationID: The relation id ordered by the query
    @param columnID: The column id to create a column from
    @return: A column with incrementing ids
    */
    Column GetColumn(int relationID, int columnID);

    /*
    Replace ID Vector with new one in non-joined and delete the old one
    @param relationID: The relation id ordered by the query
    @param newVector: The new vector to replace the old one
    */
    void SetVector(int relationID, IDVector newVector);

    /*
    Destructure Joint and set or replace rel1 and rel2 ID Vectors.
    if it replaces non-joined relations, delete old ones.
    @param rel1: The first relation id
    @param rel2: The second relation id
    @param joint: The Joint stucture
    */
    void SetJoint(int rel1, int rel2, Joint *joint);

    /*
    Special case of SetJoint where the relations are already joined
    */
    void SetJoint(IDVector vector);

    /*
    Add relation to the result
    @relation: The relation to add
    */
    void AddRelation(Relation *relation);

    /*
    Print the result structure
    */
    void Print();

    /*
    Is the relation joined
    */
    bool IsJoined(int relationID);

};
