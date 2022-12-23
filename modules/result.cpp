#include "result.hpp"

void Result::assert_size(){
    // Check if all joined relations are the same size
    for (uint i=0; i<joined->get_size() -1; i++){
        if (joined->get(i)->get_size() != joined->get(i+1)->get_size()){
            throw("[ERROR] Parallel joined relations are not the same size\n");
        }
    }
}

Result::Result(){
    this->nonjoined = new List<IDVector>();
    this->joined = new List<IDVector>();
    this->nonjoined_ids = new List<int>();
    this->joined_ids = new List<int>();
    this->relations = new Vector<Relation *>();
}

Result::~Result(){

    // Delete all IDVectors
    for (uint i=0; i<nonjoined->get_size(); i++){
        delete nonjoined->get(i);
    }
    for (uint i=0; i<joined->get_size(); i++){
        delete joined->get(i);
    }

    delete this->nonjoined;
    delete this->joined;
    delete this->nonjoined_ids;
    delete this->joined_ids;
    delete this->relations;
}

void Result::AddRelation(Relation *relation){
    // Add relation to nonjoined relations
    nonjoined_ids->push(relations->get_size());
    relations->push(relation);

    // Initialize nonjoined id vector
    IDVector vector = new Vector<int>();
    for (uint i=0; i<relation->row_count(); i++){
        vector->push(i);
    }
    nonjoined->push(vector);
}

bool Result::IsJoined(int relationID){
    return joined_ids->contains(relationID);
}


Column Result::GetColumn(int relationID, int columnID){
    IDVector idVector;
    
    // Get the correct id vector
    if (nonjoined_ids->contains(relationID)){
        idVector = nonjoined->get(nonjoined_ids->index_of(relationID));
    }
    else if (joined_ids->contains(relationID)){
        idVector = joined->get(joined_ids->index_of(relationID));
    }
    else{
        printf("[ERROR] Relation %d doesn't exist\n", relationID);
        exit(1);
    }

    // Reference the relation
    Relation *relation = relations->get(relationID);
    
    // Create column
    Column column = Column(idVector->get_size());

    // Set the values to the original column's values
    for (uint i = 0; i < idVector->get_size(); i++){
        int key = i;
        int value = relation->get(idVector->get(i), columnID);
        Tuple t = Tuple(key, value);
        column.tuples[i] = t;
    }

    return column;
}

void Result::SetVector(int relationID, IDVector newVector){
    // Replace the old vector with the new one
    if (nonjoined_ids->contains(relationID)){
        IDVector oldVector = nonjoined->get(nonjoined_ids->index_of(relationID));
        if (DEBUG) printf("[DEBUG] Replaced %p with %p in %d", oldVector, newVector, relationID);

        delete oldVector;
        nonjoined->set(nonjoined_ids->index_of(relationID), newVector);
    } else {
        printf("[ERROR] Relation %d doesn't exist in result\n", relationID);
        exit(1);
    }
}

void Result::Print(){

    printf("\n");
    printf(" ________\n");
    printf("| Result |\n");
    printf("---------------------------------\n");

    printf("| Relation ids:\n");
    printf("|  - Nonjoined (size %d): ", nonjoined_ids->get_size());
    nonjoined_ids->print();
    printf("|  - Joined (size %d): ", joined_ids->get_size());
    joined_ids->print();

    printf("|--------------------------------\n");
    printf("| Nonjoined relations (size %d):\n", nonjoined->get_size());
    for (uint i=0; i<nonjoined->get_size(); i++){
        printf("|   Relation %d (size %d):\n|   ", nonjoined_ids->get(i), nonjoined->get(i)->get_size());
        // nonjoined->get(i)->print();
        printf("%p\n", nonjoined->get(i));
        printf("|\n");
    }

    printf("|--------------------------------\n");
    printf("| Joined relations (size %d):\n", joined->get_size());
    for (uint i=0; i<joined->get_size(); i++){
        printf("|   Relation %d (size %d):\n|   ", joined_ids->get(i), joined->get(i)->get_size());
        IDVector vector = joined->get(i);
        printf("%p\n", vector);
        // vector->print();
        printf("|\n");
    }
    printf("---------------------------------\n\n");
}



void Result::SetJoint(int rel1, int rel2, Joint *joint){

    // Create Vectors
    IDVector vector1 = new Vector<int>();
    IDVector vector2 = new Vector<int>();

    // Add IDs to vectors
    for (int i=0; i<joint->GetTupleCount(); i++){
        Tuple t = joint->GetTuple(i);
        vector1->push(t[0]);
        vector2->push(t[1]);
    }

    //// Both in joined

    if (joined_ids->contains(rel1) && joined_ids->contains(rel2)){
        // We can ignore this case
        if (DEBUG) printf("[DEBUG] Both are in joined\n");

        throw("Cannot join two relations if both of them are in joined\n");

        return;
    }

    //// None in joined

    if (nonjoined_ids->contains(rel1) && nonjoined_ids->contains(rel2)){

        if (DEBUG) printf("[DEBUG] Both are in nonjoined\n");

        if (joined_ids->get_size() > 0){
            throw("Cannot join two relations if none of them are in joined\n");
        }

        int index1 = nonjoined_ids->index_of(rel1);
        int index2 = nonjoined_ids->index_of(rel2);

        // Resolve ids
        for (uint i=0; i<vector1->get_size(); i++){
            vector1->set(i, nonjoined->get(index1)->get(vector1->get(i)));
            vector2->set(i, nonjoined->get(index2)->get(vector2->get(i)));
        }

        if (DEBUG) printf("[DEBUG] Replaced %p with %p in %d\n", nonjoined->get(index1), vector1, rel1);
        if (DEBUG) printf("[DEBUG] Replaced %p with %p in %d\n", nonjoined->get(index2), vector2, rel2);

        // Add vectors to joined
        joined->push(vector1);
        joined->push(vector2);

        // Add vectors to joined
        joined_ids->push(rel1);
        joined_ids->push(rel2);

        // Delete rel1 from nonjoined
        nonjoined_ids->remove_at(index1);
        delete nonjoined->get(index1);
        nonjoined->remove_at(index1);

        // Delete rel2 from nonjoined
        index2 = nonjoined_ids->index_of(rel2);
        nonjoined_ids->remove_at(index2);
        delete nonjoined->get(index2);
        nonjoined->remove_at(index2);

        assert_size();

        return;
    }


    //// One in joined

    if (!joined_ids->contains(rel1) && joined_ids->contains(rel2)){
        if(DEBUG) printf("[DEBUG] Rel2 is in joined\n Swapping them...\n");

        printf("[ERROR] Swapping not implemented\n");
        exit(1);
    }

    if (joined_ids->contains(rel1) && !joined_ids->contains(rel2)){
        
        if(DEBUG) printf("[DEBUG] Rel1 is in joined\n");

        //// Delete relative rows from joined relations
        // Iterate through joined relations
        for(ListNode<IDVector> *l = joined->begin(); l != NULL; l = joined->get_next(l)){
            // Create new vector
            IDVector newVector = new Vector<int>();
            // Get the old vector
            IDVector oldVector = l->get_value();
            if (DEBUG) printf("[DEBUG] Old vector: %p size %d\n", oldVector, oldVector->get_size());

            for (uint j=0; j<vector1->get_size(); j++){
                int id = vector1->get(j);
                newVector->push(oldVector->get(id));
            }
            // Replace the old vector with the new one
            if (DEBUG) printf("[DEBUG] Replaced %p with %p\n", oldVector, newVector);
            l->set_value(newVector);
            delete oldVector;
        }
                
        // Resolve ids
        int index2 = nonjoined_ids->index_of(rel2);
        for (uint i=0; i<vector2->get_size(); i++){
            vector2->set(i, nonjoined->get(index2)->get(vector2->get(i)));
        }

        // Add rel2 to joined
        joined_ids->push(rel2);
        joined->push(vector2);

        // Delete rel2 from nonjoined
        if (DEBUG) printf("[DEBUG] Replaced %p with %p in %d\n", nonjoined->get(index2), vector2, rel2);
        nonjoined_ids->remove_at(index2);
        delete nonjoined->get(index2);
        nonjoined->remove_at(index2);
        
        delete vector1;
    }

    assert_size();


}


void Result::SetJoint(IDVector vector){
    //// Delete relative rows from joined relations
    // Iterate through joined relations
    for(ListNode<IDVector> *l = joined->begin(); l != NULL; l = joined->get_next(l)){
        // Create new vector
        IDVector newVector = new Vector<int>();
        // Get the old vector
        IDVector oldVector = l->get_value();

        for (uint j=0; j<vector->get_size(); j++){
            int id = vector->get(j);
            newVector->push(oldVector->get(id));
        }
        // Replace the old vector with the new one
        l->set_value(newVector);
        delete oldVector;
    }
}