#pragma once
#include <cstdint>
#include <string>
#include "vector.hpp"

#define RESET   "\033[0m"
#define BOLDYELLOW  "\033[1m\033[33m"

using RelationId = unsigned;

class Relation {
  private:

  using RelationColumn = Vector<uint64_t>;
  /// Loads data from a file
  void loadRelation(const char* fileName);

  public:
  /// The number of tuples
  uint64_t size;
  /// The join column containing the keys
  Vector<RelationColumn> columns;
  /// Stores a relation into a file (binary)
  void storeRelation(const std::string& fileName);
  /// Element getter
  uint64_t get(size_t row,size_t col){ return columns[col][row]; }
  /// Total columns getter
  uint row_count(){ return columns.get_size() == 0 ? 0 : columns[0].get_size();}
  /// Total rows getter
  uint column_count(){ return columns.get_size();}
  /// Constructor without mmap
  Relation(uint64_t size,Vector<RelationColumn>&& columns) : size(size), columns(columns) {}
  /// Constructor using mmap
  Relation(const char* fileName);
  /// Delete copy constructor
  Relation(const Relation& other)=default;

  Relation& operator=(Relation&&  r) = default;

  Relation() = default;
  /// Move constructor
  Relation(Relation&& other)=default;
  /// The destructor
  ~Relation() = default;
  // Prints the relation in a tubular format
  void PrettyPrint();
  // Emphasize a column in the PrettyPrint
  void PrettyPrint(int emphasizeColumn);
};