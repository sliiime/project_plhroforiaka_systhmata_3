#include "joint.hpp"

Joint::~Joint(){
    delete tuples;
}

void Joint::AddTuple(Tuple t){
    tuples->push(t);
}

Tuple Joint::GetTuple(int i){
    return tuples->get(i);
}

int Joint::GetTupleCount(){
    return tuples->get_size();
}

void Joint::Print(){
    printf("Joint:\n");
    for(uint i = 0; i < tuples->get_size(); i++){
        tuples->get(i).Print();
    }
    printf("\n");
}

Joint::Joint(Column *c1, Column *c2){

    tuples = new Vector<Tuple>;

    // Create Partition Managers
    PartitionManager pm1 = PartitionManager(c1);
    PartitionManager pm2 = PartitionManager(c2);

    bool fitsInMemory = false;


    // Check if the partitions fit in L2 cache
    if (pm1.FitsInMemory(L2_CACHE_BYTES) && pm2.FitsInMemory(L2_CACHE_BYTES)){
        fitsInMemory = true;
        // printf("Fits in memory\n");
    }

    // Pass 1
    if (!fitsInMemory){
        pm1.Reorder(PASS1_BITS);
        pm2.Reorder(PASS1_BITS);
    }

    // Check if the partitions fit in memory
    if (!fitsInMemory && pm1.FitsInMemory(L2_CACHE_BYTES) && pm2.FitsInMemory(L2_CACHE_BYTES)){
        fitsInMemory = true;
        // printf("Fits in memory after 1 pass\n");
    }

    // Pass 2
    if (!fitsInMemory){
        pm1.Reorder(PASS2_BITS);
        pm2.Reorder(PASS2_BITS);
        // printf("Partitioned two times\n");
    }

    // Iterate over the partitions

    for (uint i=0; i<pm1.size; i++){

        // Pick the smaller partition to make the Hash Table
        bool swapped = false;
        Column *smaller, *larger;
        Column R = pm1.GetPartition(i);
        Column S = pm2.GetPartition(i);

        if (R.num_tuples < S.num_tuples){
            smaller = &R;
            larger = &S;
        } else {
            smaller = &S;
            larger = &R;
            swapped = true;
        }


        // Create a hop table
        Hoptable ht = Hoptable();
        for (int i = 0; i < smaller->num_tuples; i++){
            ht.insert(smaller->tuples[i].payload, smaller->tuples[i].key);
        }
        
        // Iterate over the larger partition
        for (int i = 0; i < larger->num_tuples; i++){

            // Lookup and create the tuples
            int key = larger->tuples[i].key;
            int payload = larger->tuples[i].payload;

            const Vector<uint32_t> *values = ht.lookup(payload);
            if (values == NULL) continue;

            for (uint j = 0; j < values->get_size(); j++){
                // Add tuples to result
                Tuple t;
                if (swapped) t = Tuple(key, values->get(j));
                else         t = Tuple(values->get(j), key);
                this->AddTuple(t);
            }
        }


        // Set tuples to null so the destructor doesn't delete them
        R.tuples = NULL;
        S.tuples = NULL;

    }
}