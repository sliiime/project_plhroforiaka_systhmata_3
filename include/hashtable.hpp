#include <vector.hpp>

typedef Vector<int>* Bucket;

class HashTable{
private:
    int size;
    Vector<Bucket> *relation;
    int hash(int key);

public:
    HashTable(int size);
    ~HashTable();

    /*
    Insert a value into the hash table according to the key
    */
    void Insert(int key, int value);

    /*
    Get the bucket at the given index
    */
    Bucket GetBucket(int index);

    /*
    Print the hash table
    */
    void Print();

    /*
    Get the bucket of a key
    */
    Bucket GetKey(int key);

    /*
    Resizes the hash table
    */
    void Rehash(int new_size);
};