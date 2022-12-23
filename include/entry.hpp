#include  <iostream>
#include  <cstdint>
#include "vector.hpp"

using namespace std;

class Entry {


    private:
        int32_t key;
        Vector<uint32_t>* bucket;
    public:
        Entry();
        Entry(int32_t);
        Entry& operator=(const Entry& entry);
        ~Entry();

        void set_key(int32_t);

        int32_t get_key()const;

        void insert_value(uint32_t);

        bool is_free() const;
        bool key_is(int32_t key) const;
        uint32_t get_bucket_size() const;
        Vector<uint32_t> *get_bucket()const;
        void set_bucket(Vector<uint32_t>* bucket);

};