#pragma once
#include <pthread.h>
#include "histogram.hpp"
#include "column.hpp"
#include "config.hpp"
#include "jsch.hpp"

class PartitionManager {

public:

    Histogram *psum;
    Column *column;
    uint max_bucket_size;
    uint n_bits;
    uint size;

    /*
    Creates a partition table with given column
    */
    PartitionManager(Column *);

    /*
    Reorders the column according to the given number of bits and destroys the old column
    @param n_bits Number of bits to use for partitioning
    @return a pointer to the reordered column
    */
    void Reorder(uint);

    void Reorder(uint,jsch::JobScheduler&,const size_t);

    /*
    Frees the memory allocated for a partition
    */
    ~PartitionManager();

    /*
    Get a pointer to a new column with the tuples in the given bucket
    @param bucket Bucket to get the column from
    @return Pointer to a column with the tuples in the given bucket
    */
    Column GetPartition(int);

    /*
    Get if the partitions fit in L2 cache
    */
   bool FitsInMemory(uint memory_size);


};
