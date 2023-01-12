#include "partitionmanager.hpp"
#include "jsch.hpp"

PartitionManager::PartitionManager(Column *column) {
    this->column = column;
    this->psum = new Histogram(1);
    this->max_bucket_size = column->num_tuples;
    this->n_bits = 0;
    this->size = 1;
}

struct HistogramArgs {
    Column *column;
    uint n_bits;
    int start;
    int size;
    Histogram *histogram;

};

void CreateHistogram(HistogramArgs* args){    
    // Count tuples
    for(int i = args->start; i < args->start+args->size; i++){
        int value = args->column->tuples[i].payload;
        int index = Utils::GetLastBits(value, args->n_bits);
        args->histogram->data[index].payload++;
    }
}


struct CopyArgs{
    Column *column;
    uint old_n_bits;
    uint new_n_bits;
    int bucket_size;
    Histogram *bucket_count;
    int *bucket_map;
    Tuple *reordered_tuples;
    int start;
    int size;
    pthread_mutex_t *locks;
};

void CopyTuples(CopyArgs* args){

    // For each tuple in the sub-bucket
    for (int i=args->start; i<args->start + args->size; i++){

        // Explanation in README.md

        int value = args->column->tuples[i].payload;
        int old_bucket = Utils::GetLastBits(value, args->old_n_bits);
        int sub_bucket = Utils::GetLastBits(value, args->new_n_bits) >> args->old_n_bits;
        int bucket_bits = old_bucket | (sub_bucket << args->old_n_bits);


        // Lock the partition
        pthread_mutex_lock(&args->locks[bucket_bits]);
        int index = args->bucket_map[bucket_bits] + args->bucket_count->data[bucket_bits].payload++;
        pthread_mutex_unlock(&args->locks[bucket_bits]);
        args->reordered_tuples[index] = args->column->tuples[i];
    }
}



void PartitionManager::Reorder(uint n_bits) {

    this->size = 1 << n_bits;

    //// Histogram Jobs ////

    // Devide column into THREADS threads
    int num_threads = THREADS;
    int num_tuples_per_thread = this->column->num_tuples / num_threads;
    int num_tuples_last_thread = this->column->num_tuples - (num_tuples_per_thread * (num_threads-1));

    // Create threads
    Histogram *histograms[num_threads];
    for (int i=0; i<num_threads; i++){
        histograms[i] = new Histogram(this->size);
    }

    // Create arguments
    HistogramArgs args[num_threads];
    for (int i=0; i<num_threads; i++){
        args[i].column = this->column;              // [R] Column to read from
        args[i].n_bits = n_bits;                    // [R] Number of bits to use
        args[i].start = i*num_tuples_per_thread;    // [R] Start of the part that the thread should read
        args[i].size = num_tuples_per_thread;       // [R] Number of tuples that the thread should read
        args[i].histogram = histograms[i];          // [W] Histogram to write to
    }
    
    // Set last thread to have the remaining tuples
    args[num_threads-1].size = num_tuples_last_thread;

    // Single Threaded Version
    for (int i=0; i<num_threads; i++) CreateHistogram(args+i);
    


    // Merge histograms
    Histogram histogram = Histogram(this->size);
    for (int i=0; i<num_threads; i++){
        for (int j=0; j<histogram.size; j++){
            histogram.data[j].payload += histograms[i]->data[j].payload;
        }
        delete histograms[i];
    }

    // Create new psum
    
    Histogram *new_psum = new Histogram(histogram.size);
    Tuple *reordered_tuples = new Tuple[column->num_tuples];

    int sub_bucket_count = 1 << (n_bits - this->n_bits);
    
    // ========== Create new psum by dividing bucket ========== //

    // Key (last n_bits)
    for (int p=0; p < psum->size; p++){
        int bucket = psum->data[p].key;

        for (int i=0; i < sub_bucket_count; i++){
            int new_key = bucket | (i << this->n_bits);
            int index = p*sub_bucket_count+i;

            new_psum->data[index].key = new_key;
        }
    }

    max_bucket_size = histogram.GetMax();

    // Value (pointer to the first tuple in the bucket)
    for (int i=0; i < new_psum->size; i++){
            if (i == 0){
                new_psum->data[i].payload = 0;
            } else {
                int prev_key = new_psum->data[i-1].key;
                new_psum->data[i].payload = histogram.data[prev_key].payload + new_psum->data[i-1].payload;
            }
    }

    // Create new bucket map
    int *bucket_map = new int[new_psum->size];
    for (int i=0; i < new_psum->size; i++){
        bucket_map[i] = new_psum->data[new_psum->data[i].key].payload;
    }


    // ========== Reorder tuples ========== //
    
    //// Partition Job ////

    // Count how many values are in each bucket
    Histogram bucket_count = Histogram(this->size);

    pthread_mutex_t locks[this->size];
    for (uint i=0; i<this->size; i++){
        pthread_mutex_init(&locks[i], NULL);
    }

    CopyArgs copy_args[num_threads];
    for(int i=0; i<num_threads; i++){
        copy_args[i].column = this->column;                  // [R]  The fill column
        copy_args[i].bucket_count = &bucket_count;           // [R]  The count of tuples moved to each sub-bucket
        copy_args[i].new_n_bits = n_bits;                    // [R]  The new number of bits
        copy_args[i].old_n_bits = this->n_bits;              // [R]  The old number of bits
        copy_args[i].bucket_map = bucket_map;                // [RW] The map of where each sub-bucket starts
        copy_args[i].reordered_tuples = reordered_tuples;    // [W]  The reordered tuples 
        copy_args[i].start = i*num_tuples_per_thread;        // [R]  The start of the part that this thread should work on
        copy_args[i].size = num_tuples_per_thread;           // [R]  The size of the part that this thread should work on
        copy_args[i].locks = locks;                          // [RW] The locks for each sub-bucket
    }
    // Set last thread to have the remaining tuples
    copy_args[num_threads-1].size = num_tuples_last_thread;


    // Create threads
    for (int i=0; i<num_threads; i++) CopyTuples(copy_args);

    // Destroy locks
    for (uint i=0; i<this->size; i++){
        pthread_mutex_destroy(&locks[i]);
    }




    // Free memory
    delete psum;
    delete[] bucket_map;

    // Update partition manager
    this->psum = new_psum;
    this->n_bits = n_bits;
    
    // Update column
    delete[] column->tuples;
    column->tuples = reordered_tuples;
    
}

void PartitionManager::Reorder(uint n_bits,jsch::JobScheduler& jobScheduler) {

    this->size = 1 << n_bits;

    //// Histogram Jobs ////

    // Devide column into THREADS threads
    int num_threads = jobScheduler.workersCount();
    int num_tuples_per_thread = this->column->num_tuples / num_threads;
    int num_tuples_last_thread = this->column->num_tuples - (num_tuples_per_thread * (num_threads-1));

    // Create threads
    Histogram *histograms[num_threads];
    for (int i=0; i < num_threads; i++){
        histograms[i] = new Histogram(this->size);
    }
    
    // Create arguments
    HistogramArgs args[num_threads];
    for (int i=0; i<num_threads; i++){
        args[i].column = this->column;              // [R] Column to read from
        args[i].n_bits = n_bits;                    // [R] Number of bits to use
        args[i].start = i*num_tuples_per_thread;    // [R] Start of the part that the thread should read
        args[i].size = num_tuples_per_thread;       // [R] Number of tuples that the thread should read
        args[i].histogram = histograms[i];          // [W] Histogram to write to
    }
    
    //Set last thread to have the remaining tuples
    args[num_threads-1].size = num_tuples_last_thread;

    jsch::JobScheduler::JobPlan* mainPlan = jobScheduler.makeJobPlan();

    for (int i = 0 ; i < num_threads; i++){
        mainPlan->addRequiredJob(jsch::make_job(
            [&,i] {
                CreateHistogram(&args[i]);
            }
        ));
    }
            
    //Create new psum
    
    Histogram histogram = Histogram(this->size);
    Histogram *new_psum;
    Tuple *reordered_tuples;
    int *bucket_map;
    Histogram bucket_count = Histogram(this->size);
    pthread_mutex_t locks[this->size];
    int sub_bucket_count;
    CopyArgs copy_args[num_threads];






    jsch::JobScheduler::JobPlan* sidePlan = jobScheduler.makeJobPlan();  

    sidePlan->addRequiredJob(
        jsch::make_job(
            [&]{
            // Merge histograms
            for (int i=0; i<num_threads; i++){
                for (int j=0; j<histogram.size; j++){
                    histogram.data[j].payload += histograms[i]->data[j].payload;
                }
                delete histograms[i];
            }

            new_psum = new Histogram(histogram.size);
            reordered_tuples = new Tuple[column->num_tuples];
            sub_bucket_count = 1 << (n_bits - this->n_bits);
            // ========== Create new psum by dividing bucket ========== //

            // Key (last n_bits)
            for (int p=0; p < psum->size; p++){
                int bucket = psum->data[p].key;

                for (int i=0; i < sub_bucket_count; i++){
                    int new_key = bucket | (i << this->n_bits);
                    int index = p*sub_bucket_count+i;

                    new_psum->data[index].key = new_key;
                }
            }

            max_bucket_size = histogram.GetMax();

            // Value (pointer to the first tuple in the bucket)
            for (int i=0; i < new_psum->size; i++){
                    if (i == 0){
                        new_psum->data[i].payload = 0;
                    } else {
                        int prev_key = new_psum->data[i-1].key;
                        new_psum->data[i].payload = histogram.data[prev_key].payload + new_psum->data[i-1].payload;
                    }
            }

            // Create new bucket map
            bucket_map = new int[new_psum->size];

            for (int i=0; i < new_psum->size; i++){
                bucket_map[i] = new_psum->data[new_psum->data[i].key].payload;
            }
            for (uint i=0; i<this->size; i++){
                pthread_mutex_init(&locks[i], NULL);
            }
            
            for(int i=0; i<num_threads; i++){
                copy_args[i].column = this->column;                  // [R]  The fill column
                copy_args[i].bucket_count = &bucket_count;           // [R]  The count of tuples moved to each sub-bucket
                copy_args[i].new_n_bits = n_bits;                    // [R]  The new number of bits
                copy_args[i].old_n_bits = this->n_bits;              // [R]  The old number of bits
                copy_args[i].bucket_map = bucket_map;                // [RW] The map of where each sub-bucket starts
                copy_args[i].reordered_tuples = reordered_tuples;    // [W]  The reordered tuples 
                copy_args[i].start = i*num_tuples_per_thread;        // [R]  The start of the part that this thread should work on
                copy_args[i].size = num_tuples_per_thread;           // [R]  The size of the part that this thread should work on
                copy_args[i].locks = locks;                          // [RW] The locks for each sub-bucket
            }
            // Set last thread to have the remaining tuples
            copy_args[num_threads-1].size = num_tuples_last_thread;
        })
    );

    // Add dependent jobs
    for (int i=0; i<num_threads; i++){
        sidePlan->addDependentJob(jsch::make_job(
            [&,i]{
                CopyTuples(&copy_args[i]);
            }
        ));
    }

    mainPlan->addDependentJob(sidePlan);
    jsch::Future* future = jobScheduler.submitJobWithFuture(mainPlan);

    future->wait();


    // Destroy locks
    for (uint i=0; i<this->size; i++){
        pthread_mutex_destroy(&locks[i]);
    }




    // Free memory
    delete psum;
    delete[] bucket_map;

    // Update partition manager
    this->psum = new_psum;
    this->n_bits = n_bits;
    
    // Update column
    delete[] column->tuples;
    column->tuples = reordered_tuples;
    
    
}


PartitionManager::~PartitionManager(){
    delete this->psum;
}

Column PartitionManager::GetPartition(int bucket){

    // Start index of partition
    int partition_start = psum->data[bucket].payload;

    // Get partition size by subtracting the start index of the next partition
    int partition_size;
    if (bucket == psum->size - 1){
        partition_size = column->num_tuples - partition_start;
    } else {
        partition_size = psum->data[bucket+1].payload - partition_start;
    }

    // Create a new column object
    Column c = Column();
    c.num_tuples = partition_size;
    c.tuples = column->tuples + partition_start;

    return c;
}

bool PartitionManager::FitsInMemory(uint memory_size){
    return this->max_bucket_size * sizeof(this->column->tuples->payload) <= memory_size;
}