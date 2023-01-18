#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <sys/mman.h>
#include <sys/stat.h>
#include "relation.hpp"
#include <assert.h>
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
void Relation::storeRelation(const string& fileName)
  // Stores a relation into a binary file
{
  ofstream outFile;
  outFile.open(fileName,ios::out|ios::binary);
  //Column size (total rows)
  outFile.write((char*)&size,sizeof(size));
  //Number of Columns (total columns)
  size_t numColumns=(size_t)columns.get_size();

  outFile.write((char*)&numColumns,sizeof(size_t));
  for (uint i = 0 ; i < columns.get_size(); i++) {
    assert(columns[i].get_size() == size);
    for (uint j = 0 ; j < size; j++) 
      outFile.write((char*)&columns[i][j],sizeof(uint64_t));
  }
  outFile.close();
}
//---------------------------------------------------------------------------
void Relation::loadRelation(const char* fileName)
{
  int fd = open(fileName, O_RDONLY);
  if (fd==-1) {
    cerr << "cannot open " << fileName << endl;
    throw;
  }

  // Obtain file size
  struct stat sb;
  if (fstat(fd,&sb)==-1)
    cerr << "fstat\n";

  auto length=sb.st_size;
  
  char* staddr=static_cast<char*>(mmap(nullptr,length,PROT_READ,MAP_PRIVATE,fd,0u));
  char* addr = staddr;
  if (addr==MAP_FAILED) {
    cerr << "cannot mmap " << fileName << " of length " << length << endl;
    throw;
  }

  if (length<16) {
    cerr << "relation file " << fileName << " does not contain a valid header" << endl;
    throw;
  }

  this->size=*reinterpret_cast<uint64_t*>(addr);
  addr+=sizeof(size);
  auto numColumns=*reinterpret_cast<size_t*>(addr);
  addr+=sizeof(size_t);
  for (unsigned i=0;i<numColumns;++i) {
    RelationColumn column(size);
    for (uint j = 0; j < size; j++){
      column.push(reinterpret_cast<uint64_t*>(addr)[j]);
    }

    addr+=size*sizeof(uint64_t);  
    columns.push(std::move(column));
  }

  munmap(staddr,length);
}
//---------------------------------------------------------------------------
Relation::Relation(const char* fileName) 
{
  loadRelation(fileName);
}
//---------------------------------------------------------------------------

void Relation::PrettyPrint(){
    printf("Relation: %d x %d\n", this->row_count(), this->column_count());
    for (uint i=0; i<row_count(); i++){
        printf("%3d:", i);
        for (uint j=0; j<column_count(); j++){
            printf("%8ld", this->get(i,j));
        }
        printf("\n");
    }
    printf("\n");
}

void Relation::PrettyPrint(int emphasizeColumn){
    printf("Relation: %d x %d\n", this->row_count(), this->column_count());
    for (uint i=0; i<row_count(); i++){
        printf("%3d:", i);
        for (uint j=0; j<column_count(); j++){
            if (j == (uint)emphasizeColumn){
                printf(BOLDYELLOW);
                printf("%4ld", this->get(i,j));
                printf(RESET);
            }
            else{
                printf("%4ld", this->get(i,j));
            }
        }
        printf("\n");
    }
    printf("\n");
}