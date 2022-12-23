#pragma once
#include <iostream>
#include <stdint.h>
#include "entry.hpp"
#include "list_neighbourhood.hpp"

#define STARTING_CAPACITY 1024

enum class BucketSearchStatus{FOUND,NOT_FOUND,FREE};


class Hoptable {

    private:

        uint32_t size;
        uint32_t capacity;
        uint32_t (*hashFunction)(int32_t);
        Entry* entries;
        Neighbourhood** neighbourhoods;


        /**
         * @brief Tries to find an entry for the requested key inside the neighbourhood of the index          
         * 
         * @param index Starting point of the search [index,index + HOOD).
         * @param key   Key to find entry for.
         * @param bucket In case of finding a suited entry for this key, the entry is stored here.
         * @return BucketSearchStatus::FOUND if an entry for values with the requested key already exists,
         *         BucketSearchStatus::FREE if there doesn't already exist an entry for values with this key,but a free entry was found in the Neighbourhood
         *         BucketSeartStatus::NOT_FOUND if there weren't any available entries in the neighbourhood to store the key.
         */
        BucketSearchStatus findNeighbourhoodEntry(uint32_t index,int32_t key,uint32_t* bucket);







        /**
         * @brief Allocates space for the Neighbourhood instance of the given index.
         *
         * 
         * @param index 
         */
        void initializeNeighbourhood(uint32_t index);







        /**
         * @brief Tries to find a free entry, throws RehashingException if no free entries exist in the table.
         * 
         * @param index Starting point of the search.
         * @return uint32_t 
         */
        uint32_t findNextFreeEntry(uint32_t index) const;







        /**
         * @brief Tries to move 'from' in to the neighbourhood of 'to', throws RehashingException if such action is not possible.
         * 
         * @param from starting point
         * @param to   destination
         * @return uint32_t 
         */
        uint32_t moveFreeEntryToNeighbourhood(uint32_t from, uint32_t to);
        






        /**
         * @brief Tries to resize the table and rehash the values, throws an exception if rehashing is not possible.
         * 
         */
        void rehash();







        /**
         * @brief Returns the distance between two indexes of the table where only moving backwards is allowed,
         * eg: Relation size 10 (indexing starts at 0): backwardsDistance(3,9) = 3, backwardsDistance(9,3) = 6 
         *   
         * @param from 
         * @param to 
         * @return uint32_t 
         */
        uint32_t backwardsDistance(uint32_t from, uint32_t to) const; // distance(x,y) != distance(y,x)






        /**
         * @brief Used for deallocating resources of a Hoptable
         * 
         */
        void destroy();
        





        /**
         * @brief Returns index of the entry that corresponds to the key,hash pair
         * 
         * @param hash 
         * @param key 
         * @return uint32_t 
         */
        uint32_t getPositionInNeighbourhood(uint32_t hash,int32_t key);
        






        /**
         * @brief Tries to insert a key,value pair into the hoptable, throws rehashing exception if it failed.
         * 
         * @param key 
         * @param value 
         */
        void insert_try(int32_t key,uint32_t value);







        /**
         * @brief Tries to insert a vector of values with the same key into the hoptable, throws rehashing exception if it failed. 
         * 
         * @param key 
         * @param values 
         */
        // void insert_try(int32_t key,Vector<uint32_t>& values);






        /**
         * @brief Used for inserting the already existent buckets of the previous table into the entries of the new table during rehashing.
         * 
         * @param key 
         * @param bucket 
         */
        void insert_try(int32_t key,Vector<uint32_t>* bucket);
        
        Hoptable(uint32_t capacity); //Used for rehashing

    public:
        Hoptable();
        Hoptable(uint32_t (*hashFunction)(int32_t));
        /**
         * @brief Inserts key value pair into the hoptable.
         * 
         * @param key 
         * @param value 
         */
        void insert(int32_t key, uint32_t value);
        const Vector<uint32_t>* lookup(int32_t key);

        /**
         * @brief Returns the amount of the total inserted elements.
         * 
         * @return uint32_t 
         */
        uint32_t get_size() const;

        virtual ~Hoptable();

        void printNeighbourhood(uint32_t i) const;
        void print();

    protected: //Used for testing purposes only

        uint32_t get_capacity();
        const Entry& get_entry(uint32_t index);
        const Neighbourhood* get_neighbourhood(uint32_t index);
        Hoptable(uint32_t capacity,uint32_t (*hf)(int32_t));
};


class RehashingException : std::exception{};
