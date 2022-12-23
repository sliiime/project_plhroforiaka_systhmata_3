#include  "entry.hpp"

Entry::Entry() {
    this->bucket = NULL;
}

Entry& Entry::operator=(const Entry& entry){

    this->bucket = entry.bucket;
    this->key = entry.key;

    return *this;
}


Entry::~Entry() {
    if(this->bucket != NULL) delete this->bucket;
}

void Entry::set_key(int32_t key) {
    this->key = key;
}

int32_t Entry::get_key()const {
    return this->key;
}

Vector<uint32_t> *Entry::get_bucket()const {
    return this->bucket;
}

void Entry::set_bucket(Vector<uint32_t>* bucket){
    this->bucket = bucket;
}

uint32_t Entry::get_bucket_size() const {
    return this->bucket == NULL ? 0 : this->bucket->get_size();
}
 
void Entry::insert_value(uint32_t value) {
    if(this->bucket == NULL) this->bucket = new Vector<uint32_t>;
    this->bucket->push(value);
}

bool Entry::is_free() const{
    return bucket == NULL || bucket->get_size() == 0 ;
}

bool Entry::key_is(int32_t key) const{
    return bucket != NULL && this->key == key;
}
