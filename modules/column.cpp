#include "column.hpp"
#include "utils.hpp"

void Column::Print(){
    printf("Column of size %d:\n", this->num_tuples);
    for(int i = 0; i < this->num_tuples; i++){
        printf("%d: ", i);
        this->tuples[i].Print();
    }
    printf("\n");
}

Column::Column(Relation *relation, uint columnID){   

    this->num_tuples = relation->row_count();
    this->tuples = new Tuple[this->num_tuples];

    for(uint i = 0; i < relation->row_count(); i++){
        Tuple t = Tuple(i, relation->get(i,columnID));
        this->tuples[i] = t;
    }
}

Column::Column(){}

Column::Column(uint num_tuples){
    this->num_tuples = num_tuples;
    this->tuples = new Tuple[num_tuples];
}

Column::~Column(){
    if (this->tuples != NULL){
        delete[] this->tuples;
    }
}

void Column::PrintBinary(){
    printf("Column of size %d:\n", this->num_tuples);
    for(int i = 0; i < this->num_tuples; i++){
        printf("%d: ", i);
        Utils::PrintBinary(this->tuples[i].payload);
        printf("\n");
    }
    printf("\n");
}

Tuple Column::operator[](int i){
    return this->tuples[i];
}