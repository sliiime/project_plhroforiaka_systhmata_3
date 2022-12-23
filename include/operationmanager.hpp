#include "parser.hpp"
#include "result.hpp"
#include "filtering.hpp"

class OperationManager{
private:
    Vector<Relation *> *relations;

    long ProjectColumn(const Column &column);
    
public:
    OperationManager(Vector<Relation *> *relations);
    ~OperationManager();
    void Execute(QueryInfo *queryInfo);
};