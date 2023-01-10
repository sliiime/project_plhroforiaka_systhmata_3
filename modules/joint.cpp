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


Joint::Joint(Column *c1, Column *c2, jsch::JobScheduler& jobScheduler){

    // Create Partition Managers
    PartitionManager pm1 = PartitionManager(c1);
    PartitionManager pm2 = PartitionManager(c2);

    bool fitsInMemory = false;


    // Check if the partitions fit in L2 cache
    if (pm1.FitsInMemory(L2_CACHE_BYTES) && pm2.FitsInMemory(L2_CACHE_BYTES)){
        fitsInMemory = true;
    }

    // Pass 1
    if (!fitsInMemory){
        pm1.Reorder(PASS1_BITS,jobScheduler);
        pm2.Reorder(PASS1_BITS,jobScheduler);
    }

    // Check if the partitions fit in memory
    if (!fitsInMemory && pm1.FitsInMemory(L2_CACHE_BYTES) && pm2.FitsInMemory(L2_CACHE_BYTES)){
        fitsInMemory = true;
    }

    // Pass 2
    if (!fitsInMemory){
        pm1.Reorder(PASS2_BITS,jobScheduler);
        pm2.Reorder(PASS2_BITS,jobScheduler);
    }


    Vector<Tuple> workerTuples[pm1.size];
    auto job1 = jsch::make_job(
        [&]{
 // Pick the smaller partition to make the Hash Table
            uint workerId = 0;

            bool swapped = false;
            Column *smaller, *larger;
            Column R = pm1.GetPartition(0);
            Column S = pm2.GetPartition(0);

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
                    workerTuples[workerId].push(t);
                }
            }
            // Set tuples to null so the destructor doesn't delete them
            R.tuples = NULL;
            S.tuples = NULL;
        }
    );

    jsch::JobScheduler::JobSequence* jobSequence = jobScheduler.makeJobSequence(job1);
    // Iterate over the partitions
    for (uint i=1; i<pm1.size; i++){
        jobSequence->then(jsch::make_job([&,i]{
            // Pick the smaller partition to make the Hash Table
            uint workerId = i;

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
                    workerTuples[workerId].push(t);
                }
            }
            // Set tuples to null so the destructor doesn't delete them
            R.tuples = NULL;
            S.tuples = NULL;
        }));   
    }

    auto job2 = jsch::make_job(
        [&]{
            uint totalTuples = 0;
            for (uint i = 0 ; i < pm1.size; i++) totalTuples += workerTuples[i].get_size();

            //Initialize tuples Vector
            this->tuples = new Vector<Tuple>(totalTuples);

            //Merging tuples of each worker
            for (uint i = 0 ; i < pm1.size; i++){
                for (uint j = 0 ; j < workerTuples[i].get_size(); j++) this->AddTuple(workerTuples[i][j]);
            }            
        }
    );

    jobSequence->then(job2);

    //Calculate total number of tuples, so vector doesn't have to resize 
    
    jobScheduler.submitJob(jobSequence);
    jobScheduler.wait();
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
    }

    // Pass 1
    if (!fitsInMemory){
        pm1.Reorder(PASS1_BITS);
        pm2.Reorder(PASS1_BITS);
    }

    // Check if the partitions fit in memory
    if (!fitsInMemory && pm1.FitsInMemory(L2_CACHE_BYTES) && pm2.FitsInMemory(L2_CACHE_BYTES)){
        fitsInMemory = true;
    }

    // Pass 2
    if (!fitsInMemory){
        pm1.Reorder(PASS2_BITS);
        pm2.Reorder(PASS2_BITS);
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