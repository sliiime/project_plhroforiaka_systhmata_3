#include <cassert>
#include <iostream>
#include <utility>
#include <sstream>
#include "parser.hpp"
//---------------------------------------------------------------------------
using namespace std;

//---------------------------------------------------------------------------
//some weird definitions that I had to add for the program to compile 
constexpr const char SelectInfo::delimiterSQL[];
constexpr const char PredicateInfo::delimiterSQL[];
constexpr const char FilterInfo::delimiterSQL[];
//---------------------------------------------------------------------------
static void splitString(string& line,Vector<unsigned>& result,const char delimiter)
  // Split a line into numbers
{
  stringstream ss(line);
  string token;
  while (getline(ss,token,delimiter)) {
    result.push(stoul(token));
  }
}
//---------------------------------------------------------------------------
static void splitString(string& line,Vector<string>& result,const char delimiter)
  // Parse a line into strings
{
  stringstream ss(line);
  string token;
  while (getline(ss,token,delimiter)) {
    result.push(token);
  }
}
//---------------------------------------------------------------------------
static void splitPredicates(string& line,Vector<string>& result)
  // Split a line into predicate strings
{
  // Determine predicate type
  for (uint i = 0 ; i < comparisonTypes.get_size(); i++) {
    auto cT = comparisonTypes[i];
    if (line.find(cT)!=string::npos) {
      splitString(line,result,cT);
      break;
    }
  }
}
//---------------------------------------------------------------------------
void QueryInfo::parseRelationIds(string& rawRelations)
  // Parse a string of relation ids
{
  splitString(rawRelations,relationIds,' ');
}
//---------------------------------------------------------------------------
static SelectInfo parseRelColPair(string& raw)
{
  Vector<unsigned> ids;
  splitString(raw,ids,'.');
  return SelectInfo(0,ids[0],ids[1]);
}
//---------------------------------------------------------------------------
inline static bool isConstant(string& raw) { return raw.find('.')==string::npos; }
//---------------------------------------------------------------------------
void QueryInfo::parsePredicate(string& rawPredicate)
  // Parse a single predicate: join "r1Id.col1Id=r2Id.col2Id" or "r1Id.col1Id=constant" filter
{
  Vector<string> relCols;
  splitPredicates(rawPredicate,relCols);
  assert(relCols.get_size()==2);
  assert(!isConstant(relCols[0])&&"left side of a predicate is always a SelectInfo");
  auto leftSelect=parseRelColPair(relCols[0]);
  if (isConstant(relCols[1])) {
    uint64_t constant=stoul(relCols[1]);
    char compType=rawPredicate[relCols[0].size()];
    filters.push(FilterInfo(leftSelect,constant,FilterInfo::Comparison(compType)));
  } else {
    predicates.push(PredicateInfo(leftSelect,parseRelColPair(relCols[1])));
  }
}
//---------------------------------------------------------------------------
void QueryInfo::parsePredicates(string& text)
  // Parse predicates
{
  Vector<string> predicateStrings;
  splitString(text,predicateStrings,'&');
  for (uint i = 0 ; i < predicateStrings.get_size(); i++ ) parsePredicate(predicateStrings[i]);
  
}
//---------------------------------------------------------------------------
void QueryInfo::parseSelections(string& rawSelections)
 // Parse selections
{
  Vector<string> selectionStrings;
  splitString(rawSelections,selectionStrings,' ');
  for (uint i = 0; i < selectionStrings.get_size(); i++) {
    selections.push(SelectInfo(parseRelColPair(selectionStrings[i])));
  }
}
//---------------------------------------------------------------------------
static void resolveIds(Vector<unsigned>& relationIds,SelectInfo& selectInfo)
  // Resolve relation id
{
  selectInfo.relId=relationIds[selectInfo.binding];
}
//---------------------------------------------------------------------------
void QueryInfo::resolveRelationIds()  // Resolve relation ids
{
  // Selections
  for (uint i = 0 ; i < selections.get_size(); i++) resolveIds(relationIds,selections[i]);
  
  // Predicates
  for (uint i = 0 ; i < predicates.get_size(); i++) {
    resolveIds(relationIds,predicates[i].left);
    resolveIds(relationIds,predicates[i].right);
  }
  // Filters
  for (uint i = 0 ; i < filters.get_size(); i++) resolveIds(relationIds,filters[i].filterColumn);
  
}
//---------------------------------------------------------------------------
void QueryInfo::parseQuery(string& rawQuery)
  // Parse query [RELATIONS]|[PREDICATES]|[SELECTS]
{
  clear();
  Vector<string> queryParts;
  splitString(rawQuery,queryParts,'|');
  assert(queryParts.get_size()==3);
  parseRelationIds(queryParts[0]);
  parsePredicates(queryParts[1]);
  parseSelections(queryParts[2]);
  resolveRelationIds();
  
}
//---------------------------------------------------------------------------
void QueryInfo::clear()
  // Reset query info
{
  relationIds.clear();
  predicates.clear();
  filters.clear();
  selections.clear();
}
//---------------------------------------------------------------------------
QueryInfo::QueryInfo(string rawQuery) { parseQuery(rawQuery); }
//---------------------------------------------------------------------------
void QueryInfo::print() const {
      std::cout << "relationIds : " << std::endl;
      for (uint i = 0; i < relationIds.get_size(); i++) std::cout << relationIds[i] << std::endl;
      
      std::cout << "predicates : " << std::endl;
      for (uint i = 0; i < predicates.get_size(); i++) predicates[i].print();

      std::cout << "filters : " << std::endl;
      for (uint i = 0 ;i < filters.get_size(); i++) filters[i].print();

      std::cout << "selections : " << std::endl;
      for (uint i = 0; i < selections.get_size(); i++) selections[i].print();

}
//----------------------------------------------------------------------------
