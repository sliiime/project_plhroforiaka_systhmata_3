#include "partitionmanager.hpp"

PartitionManager::PartitionManager(Column *column) {
    this->column = column;
    this->psum = new Histogram(1);
    this->max_bucket_size = column->num_tuples;
    this->n_bits = 0;
    this->size = 1;
}


void PartitionManager::Reorder(uint n_bits) {

    this->size = 1 << n_bits;

    // Create histogram
    Histogram histogram = Histogram(this->size);

    // Count tuples
    for(int i = 0; i < this->column->num_tuples; i++){
        int value = column->tuples[i].payload;
        int index = Utils::GetLastBits(value, n_bits);
        histogram.data[index].payload++;
    }
    
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

    // For each old bucket
    for (int p=0; p < psum->size; p++){
        int bucket = psum->data[p].key;
        int bucket_start = psum->data[p].payload;
        int bucket_size;

        // Calculate bucket size
        if (p == psum->size-1){
            bucket_size = column->num_tuples - bucket_start;
        } else {
            bucket_size = psum->data[p+1].payload - bucket_start;
        }

        // Count how many values are in each sub-bucket
        Histogram bucket_count = Histogram(sub_bucket_count);

        // For each tuple in the sub-bucket
        for (int i=0; i<bucket_size; i++){

            // Explanation in README.md

            int value = column->tuples[bucket_start+i].payload;
            int sub_bucket = Utils::GetLastBits(value, n_bits) >> this->n_bits;
            int bucket_bits = bucket | (sub_bucket << this->n_bits);
            int index = bucket_map[bucket_bits] + bucket_count.data[sub_bucket].payload++;
            
            reordered_tuples[index] = column->tuples[bucket_start+i];
        }
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