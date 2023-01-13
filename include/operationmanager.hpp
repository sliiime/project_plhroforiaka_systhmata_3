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
    void Execute(QueryInfo *queryInfo, jsch::JobScheduler& jobScheduler);
    Result* ExecuteAndReturn(QueryInfo *queryInfo, jsch::JobScheduler& jobScheduler,const size_t workersPerQuery);
    void printColumnProjection(Vector<SelectInfo>& selectInfo,Result* result);
};