#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include "vector.hpp"
#include <sstream>
#include "relation.hpp"


//---------------------------------------------------------------------------
struct SelectInfo {
   /// Relation id
   RelationId relId;
   /// Binding for the relation
   unsigned binding;
   /// Column id
   unsigned colId;

   SelectInfo() = default;
   /// The constructor
   SelectInfo(RelationId relId,unsigned b,unsigned colId) : relId(relId), binding(b), colId(colId) {};
   /// The constructor if relation id does not matter
   SelectInfo(unsigned b,unsigned colId) : SelectInfo(-1,b,colId) {};

   SelectInfo(const SelectInfo& selectInfo) : relId(selectInfo.relId), binding(selectInfo.binding),colId(selectInfo.colId){};

   inline void print() const {
      std::cout << "relId : " << relId << " binding : " << binding << " colId : " << colId << std::endl;
   }
   /// Equality operator
   inline bool operator==(const SelectInfo& o) const {
     return o.relId == relId && o.binding == binding && o.colId == colId;
   }
   /// Less Operator
   inline bool operator<(const SelectInfo& o) const {
     return binding<o.binding||colId<o.colId;
   }

   /// The delimiter used in our text format
   static const char delimiter=' ';
   /// The delimiter used in SQL
   constexpr static const char delimiterSQL[]=", ";
};
//---------------------------------------------------------------------------
struct FilterInfo {
   enum Comparison : char { Less='<', Greater='>', Equal='=' };
   /// Filter Column
   SelectInfo filterColumn;
   /// Constant
   uint64_t constant;
   /// Comparison type
   Comparison comparison;
   /// Dump SQL
   FilterInfo() = default;
   /// The constructor
   FilterInfo(SelectInfo filterColumn,uint64_t constant,Comparison comparison) : filterColumn(filterColumn), constant(constant), comparison(comparison) {};
   /// Copy Constructor
   FilterInfo(const FilterInfo& filterInfo): filterColumn(filterInfo.filterColumn),constant(filterInfo.constant),comparison(filterInfo.comparison){};
   ///
   inline void print() const {
      std::cout << "filterColumn : ";
      filterColumn.print();
      std::cout << "constant : " << constant << " comparison : " << (char)comparison << std::endl; 
   }
   /// The delimiter used in our text format
   static const char delimiter='&';
   /// The delimiter used in SQL
   constexpr static const char delimiterSQL[]=" and ";
};
static const Vector<FilterInfo::Comparison> comparisonTypes { FilterInfo::Comparison::Less, FilterInfo::Comparison::Greater, FilterInfo::Comparison::Equal};
//---------------------------------------------------------------------------
struct PredicateInfo {
   /// Left
   SelectInfo left;
   /// Right
   SelectInfo right;
   /// The constructor
   PredicateInfo(SelectInfo left, SelectInfo right) : left(left), right(right){};
   /// The Default constructor
   PredicateInfo() = default;
   /// The copy constructor
   PredicateInfo(const PredicateInfo& predicateInfo): left(predicateInfo.left), right(predicateInfo.right){}
   /// 
   inline void print() const {
      std:: cout << "[" << left.binding << "." << left.colId << " == " << right.binding << "." << right.colId << "]" << std::endl;
   }
   
   std::string toString() const {
      std::string str;
      std::stringstream sstream(str);

      sstream << "[" << left.binding << "." << left.colId << " == " << right.binding << "." << right.colId << "]";

      return sstream.str();


   }

   bool isSelfJoin() const { return this->left.binding == this->right.binding;}
   /// The delimiter used in our text format
   static const char delimiter='&';
   /// The delimiter used in SQL
   constexpr static const char delimiterSQL[]=" and ";
};
//---------------------------------------------------------------------------
class QueryInfo {
   public:
   /// The relation ids
   Vector<RelationId> relationIds;
   /// The predicates
   Vector<PredicateInfo> predicates;
   /// The filters
   Vector<FilterInfo> filters;
   /// The selections
   Vector<SelectInfo> selections;
   /// Reset query info
   void clear();
   void print() const;
   private:
   /// Parse a single predicate
   void parsePredicate(std::string& rawPredicate);
   /// Resolve bindings of relation ids
   void resolveRelationIds();

   public:
   /// Parse relation ids <r1> <r2> ...
   void parseRelationIds(std::string& rawRelations);
   /// Parse predicates r1.a=r2.b&r1.b=r3.c...
   void parsePredicates(std::string& rawPredicates);
   /// Parse selections r1.a r1.b r3.c...
   void parseSelections(std::string& rawSelections);
   /// Parse selections [RELATIONS]|[PREDICATES]|[SELECTS]
   void parseQuery(std::string& rawQuery);
   /// Dump text format
   /// The empty constructor
   QueryInfo() {}
   ///Copy Constructor
   QueryInfo(const QueryInfo& queryInfo): relationIds(queryInfo.relationIds),predicates(queryInfo.predicates),selections(queryInfo.selections){}
   /// Move Constructor
   QueryInfo(QueryInfo&& queryInfo) = default;
   /// Move assignment operator
   QueryInfo& operator=(QueryInfo&& queryInfo) = default;
   /// The constructor that parses a query
   QueryInfo(std::string rawQuery);
   ~QueryInfo() = default;
};
//---------------------------------------------------------------------------
