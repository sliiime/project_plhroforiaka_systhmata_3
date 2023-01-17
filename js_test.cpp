#include <iostream>
#include "jsch.hpp"

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define GRAY    "\033[1;30m"    /* Gray */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

using namespace std;

enum Test {PASS, FAIL, PROBLEM_WITH_TEST};

void result(Test test, string name){
    // Print the result
    cout << GRAY << name << ": ";
    switch (test){
        case PASS:
            cout << BOLDGREEN << "PASS" << RESET << endl;
            break;
        case FAIL:
            cout << BOLDRED << "FAIL" << RESET << endl;
            break;
        case PROBLEM_WITH_TEST:
            cout << BOLDCYAN << "PROBLEM_WITH_TEST" << RESET << endl;
            break;
    }
}

void operationsDoneSequentially() {
    jsch::JobScheduler jobScheduler(10);

    pthread_mutex_t mutex;

    pthread_mutex_init(&mutex, NULL);

    int i = 10;

    jsch::JobScheduler::JobSequence* jobSequence = jobScheduler.makeJobSequence();
    jobSequence->then(jsch::make_job(
        [&]{
            pthread_mutex_lock(&mutex);
            i += 20;
           // printf("%d\n", i);
            pthread_mutex_unlock(&mutex);
        }
    ));

    jobSequence->then(jsch::make_job(
        [&]{
            pthread_mutex_lock(&mutex);
            i *= 3;
           // printf("%d\n", i);
            pthread_mutex_unlock(&mutex);
        }
    ));

    jobSequence->then(jsch::make_job(
        [&]{
            pthread_mutex_lock(&mutex);
            i += 10;
            //printf("%d\n", i);
            pthread_mutex_unlock(&mutex);
        }
    ));

    jsch::Future* future = jobScheduler.submitJobWithFuture(jobSequence);

    future->wait();

    if(i == 100) {
        result(PASS, "Operations done sequentially");
    } else result(FAIL, "Operations done sequentially");
}



void storingDoneSequentially() {
    jsch::JobScheduler jobScheduler(10);

    pthread_mutex_t mutex;

    pthread_mutex_init(&mutex, NULL);

    int array[80];
    int index = 0;

    jsch::JobScheduler::JobSequence* jobSequence = jobScheduler.makeJobSequence();

    for(int i = 20 ; i < 100 ; i++) jobSequence->then(jsch::make_job(
        [&, i]()mutable {
            pthread_mutex_lock(&mutex);
            array[index++] = i;
            pthread_mutex_unlock(&mutex);
        }
    ));

    jsch::Future* future = jobScheduler.submitJobWithFuture(jobSequence);

    future->wait();

    int max = 0;
    for(int i = 0 ; i < 80 ; i++) {
        if(array[i] < max) {
            result(FAIL, "Storing done sequentially");
            return;
        } else max = array[i];
    }
    
    result(PASS, "Storing done sequentially");
}


void firstEvensThenOdds() {
    jsch::JobScheduler jobScheduler(10);

    pthread_mutex_t mutex;

    pthread_mutex_init(&mutex, NULL);

    int array[51];
    int index = 0;

    jsch::JobScheduler::JobPlan* jobPlan1 = jobScheduler.makeJobPlan();

    jsch::JobScheduler::JobPlan* jobPlan2 = jobScheduler.makeJobPlan();

    for (int i = 1 ; i <= 50; i++){
        jobPlan1->addRequiredJob(jsch::make_job(
        [&, i]()mutable {
            if (i % 2 == 0){
                pthread_mutex_lock(&mutex);
                array[index++] = i;
                pthread_mutex_unlock(&mutex);
            }
        }));
    }

    jobPlan2->addRequiredJob(
        jsch::make_job(
            [&] {
                array[index++] = 0;
            }
        )
    );

    for (int i = 1 ; i <= 50; i++){
        jobPlan2->addDependentJob(jsch::make_job(
        [&, i]()mutable {
            if (i % 2){
                pthread_mutex_lock(&mutex);
                array[index++] = i;
                pthread_mutex_unlock(&mutex);
            }
        }));
    }

    jobPlan1->addDependentJob(jobPlan2);

    jsch::Future* future = jobScheduler.submitJobWithFuture(jobPlan1);

    future->wait();


    bool passedZero = false;
    for(int i = 0 ; i < 50 ; i++) {
        if(array[i] == 0) {
            passedZero = true;
            continue;
        }

        if(passedZero) {
            if(array[i] % 2 == 0) {
                result(FAIL, "First evens then odds");
                return;
            } 
        } else {
            if(array[i] % 2) {
                result(FAIL, "First evens then odds");
                return;
            } 
        }

    }
    if(passedZero) result(PASS, "First evens then odds");
    else result(FAIL, "First evens then odds");
}


void firstEvensThenOddsSequentially() {
    jsch::JobScheduler jobScheduler(10);

    pthread_mutex_t mutex;

    pthread_mutex_init(&mutex, NULL);

    int array[51];
    int index = 0;

    jsch::JobScheduler::JobPlan* jobPlan1 = jobScheduler.makeJobPlan();

    jsch::JobScheduler::JobPlan* jobPlan2 = jobScheduler.makeJobPlan();

    jsch::JobScheduler::JobSequence* jobSequence1 = jobScheduler.makeJobSequence();

    jsch::JobScheduler::JobSequence* jobSequence2 = jobScheduler.makeJobSequence();

    for (int i = 1 ; i <= 50; i++){
        jobSequence1->then(jsch::make_job(
        [&, i]()mutable {
            if (i % 2 == 0){
                pthread_mutex_lock(&mutex);
                array[index++] = i;
               // printf("%d\n", i);
                pthread_mutex_unlock(&mutex);
            }
        }));
    }
    jobPlan1->addRequiredJob(jobSequence1);

    jobPlan2->addRequiredJob(
        jsch::make_job(
            [&] {
                array[index++] = 0;
            }
        )
    );

    for (int i = 1 ; i <= 50; i++){
        jobSequence2->then(jsch::make_job(
        [&, i]()mutable {
            if (i % 2){
                pthread_mutex_lock(&mutex);
                array[index++] = i;
               // printf("%d\n", i);
                pthread_mutex_unlock(&mutex);
            }
        }));
    }
    jobPlan2->addDependentJob(jobSequence2);

    jobPlan1->addDependentJob(jobPlan2);

    jsch::Future* future = jobScheduler.submitJobWithFuture(jobPlan1);

    future->wait();

    bool passedZero = false;
    int max = 0;
    for(int i = 0 ; i < 50 ; i++) {
        if(array[i] == 0) {
            passedZero = true;
            max = 0;
            continue;
        }

        if(passedZero) {
            if(array[i] % 2 == 0) {
                result(FAIL, "First evens then odds sequentially");
                return;
            } 
        } else {
            if(array[i] % 2) {
                result(FAIL, "First evens then odds sequentially");
                return;
            } 
        }

        if(array[i] < max) {
            result(FAIL, "First evens then odds sequentially");
            return;
        } else max = array[i];
    }

    if(passedZero) result(PASS, "First evens then odds sequentially");
    else result(FAIL, "First evens then odds sequentially");
}


// graph testssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssssss

// Graph node
class Vertex {
public:
    string value;
    Vertex(string value) {
        this->value = value;
    }
    ~Vertex() {}
};

class Edge {
public:
    Vertex *A;
    Vertex *B;
    Edge(Vertex *A, Vertex *B) {
        this->A = A;
        this->B = B;
    }
    ~Edge() {}
};

// Graph
class Graph {
public:
    Vertex **vertexes;
    Edge **edges;
    int size_vertexes;
    int size_edges;
    int capacity_vertexes;
    int capacity_edges;

    Graph(int nodes, int relations) {
        vertexes = new Vertex*[nodes];
        edges = new Edge*[relations];
        size_vertexes = 0;
        size_edges = 0;
        capacity_vertexes = nodes;
        capacity_edges = relations;
    }

    ~Graph() {
        for(int i = 0 ; i < size_vertexes ; i++) {
            delete vertexes[i];
        }
        for(int i = 0 ; i < size_edges ; i++) {
            delete edges[i];
        }
    }

    Vertex *insertVertex(string value) {
        if (size_vertexes >= capacity_vertexes) return NULL;

        Vertex *vertex = new Vertex(value);
        vertexes[size_vertexes++] = vertex;
        return vertex;
    }

    Edge *insertEdge(Vertex *A, Vertex *B) {
        if (size_edges >= capacity_edges) return NULL;

        Edge *edge = new Edge(A, B);
        edges[size_edges++] = edge;
        return edge;
    }

    Vertex *getVertex(string value) {
        for(int i = 0 ; i < size_vertexes ; i++) {
            if(vertexes[i]->value == value) 
                return vertexes[i];
        }
        return NULL;
    }

    void printEdges() {
        for(int i = 0 ; i < size_edges ; i++) {
            Edge *edge = edges[i];
            Vertex *A = edge->A;
            Vertex *B = edge->B;
            cout << GRAY << A->value << " - " << B->value << RESET << endl;
        }
    }
};

/*
         -- D
   -- B  -- E
A 
   -- C  -- F
         -- G
*/
void graph1() {
    jsch::JobScheduler jobScheduler(10);

    pthread_mutex_t mutex;

    pthread_mutex_init(&mutex, NULL);

    Graph graph(7, 6);

    string wantedEdges[6] = {"AB", "AC", "BD", "BE", "CF", "CG"};

    jsch::JobScheduler::JobPlan* jobPlan1 = jobScheduler.makeJobPlan();

    jsch::JobScheduler::JobPlan* jobPlan2 = jobScheduler.makeJobPlan();

    jobPlan1->addRequiredJob(jsch::make_job(
    [&] {
        pthread_mutex_lock(&mutex);
        Vertex *vertex = graph.insertVertex("A");
        if(!vertex) result(PROBLEM_WITH_TEST, "graph1");
        pthread_mutex_unlock(&mutex);
    }));
    

    jobPlan2->addRequiredJob(jsch::make_job(
    [&] {
        pthread_mutex_lock(&mutex);
        Vertex *vertex1 = graph.insertVertex("B");
        if(!vertex1) result(PROBLEM_WITH_TEST, "graph1");
        Edge *edge1 = graph.insertEdge(graph.vertexes[0], vertex1);
        if(!edge1) result(PROBLEM_WITH_TEST, "graph1");

        Vertex *vertex2 = graph.insertVertex("C");
        if(!vertex2) result(PROBLEM_WITH_TEST, "graph1");
        Edge *edge2 = graph.insertEdge(graph.vertexes[0], vertex2);
        if(!edge2) result(PROBLEM_WITH_TEST, "graph1");
        pthread_mutex_unlock(&mutex);
    }));

    jobPlan2->addDependentJob(jsch::make_job(
    [&] {
        pthread_mutex_lock(&mutex);
        Vertex *vertex1 = graph.insertVertex("D");
        if(!vertex1) result(PROBLEM_WITH_TEST, "graph1");
        Edge *edge1 = graph.insertEdge(graph.vertexes[1], vertex1);
        if(!edge1) result(PROBLEM_WITH_TEST, "graph1");

        Vertex *vertex2 = graph.insertVertex("E");
        if(!vertex2) result(PROBLEM_WITH_TEST, "graph1");
        Edge *edge2 = graph.insertEdge(graph.vertexes[1], vertex2);
        if(!edge2) result(PROBLEM_WITH_TEST, "graph1");

        Vertex *vertex3 = graph.insertVertex("F");
        if(!vertex3) result(PROBLEM_WITH_TEST, "graph1");
        Edge *edge3 = graph.insertEdge(graph.vertexes[2], vertex3);
        if(!edge3) result(PROBLEM_WITH_TEST, "graph1");

        Vertex *vertex4 = graph.insertVertex("G");
        if(!vertex4) result(PROBLEM_WITH_TEST, "graph1");
        Edge *edge4 = graph.insertEdge(graph.vertexes[2], vertex4);
        if(!edge4) result(PROBLEM_WITH_TEST, "graph1");
        pthread_mutex_unlock(&mutex);
    }));

    jobPlan1->addDependentJob(jobPlan2);

    jsch::Future* future = jobScheduler.submitJobWithFuture(jobPlan1);

    future->wait();

    cout << GRAY << "         -- " << graph.vertexes[3]->value << endl;
    cout << "   -- " << graph.vertexes[1]->value << "  -- " << graph.vertexes[4]->value << endl;
    cout << graph.vertexes[0]->value << endl; 
    cout << "   -- " << graph.vertexes[2]->value << "  -- " << graph.vertexes[5]->value << endl;
    cout << "         -- " << graph.vertexes[6]->value << RESET << endl;

    int flag = 0;
    for(int i = 0 ; i < graph.size_edges ; i ++) {
        for(int j = 0 ; j < 6 ; j ++) {
            string A = graph.edges[i]->A->value;
            string B = graph.edges[i]->B->value;
            if (wantedEdges[j].find(A) != std::string::npos && wantedEdges[j].find(B) != std::string::npos) 
                flag++;
        }
    }
    if (flag != 6) result(FAIL, "graph1");
    else result(PASS, "graph1");
    cout << endl;
}


/*
A  -- \
B  --   \
          E
C  --   /
D  -- /  
*/

void graph2() {
jsch::JobScheduler jobScheduler(10);

    pthread_mutex_t mutex;

    pthread_mutex_init(&mutex, NULL);

    Graph graph(5, 4);

    string wantedEdges[4] = {"AE", "BE", "CE", "DE"};

    jsch::JobScheduler::JobPlan* jobPlan = jobScheduler.makeJobPlan();

    jobPlan->addRequiredJob(jsch::make_job(
    [&] {
        pthread_mutex_lock(&mutex);
        Vertex *vertex1 = graph.insertVertex("A");
        if(!vertex1) result(PROBLEM_WITH_TEST, "graph2");

        Vertex *vertex2 = graph.insertVertex("B");
        if(!vertex2) result(PROBLEM_WITH_TEST, "graph2");

        Vertex *vertex3 = graph.insertVertex("C");
        if(!vertex3) result(PROBLEM_WITH_TEST, "graph2");

        Vertex *vertex4 = graph.insertVertex("D");
        if(!vertex4) result(PROBLEM_WITH_TEST, "graph2");
        pthread_mutex_unlock(&mutex);
    }));
    
    jobPlan->addDependentJob(jsch::make_job(
    [&] {
        pthread_mutex_lock(&mutex);
        Vertex *vertex = graph.insertVertex("E");
        if(!vertex) result(PROBLEM_WITH_TEST, "graph2");

        Edge *edge1 = graph.insertEdge(graph.vertexes[0], vertex);
        if(!edge1) result(PROBLEM_WITH_TEST, "graph2");

        Edge *edge2 = graph.insertEdge(graph.vertexes[1], vertex);
        if(!edge2) result(PROBLEM_WITH_TEST, "graph2");

        Edge *edge3 = graph.insertEdge(graph.vertexes[2], vertex);
        if(!edge3) result(PROBLEM_WITH_TEST, "graph2");

        Edge *edge4 = graph.insertEdge(graph.vertexes[3], vertex);
        if(!edge4) result(PROBLEM_WITH_TEST, "graph2");
        pthread_mutex_unlock(&mutex);
    }));

    jsch::Future* future = jobScheduler.submitJobWithFuture(jobPlan);

    future->wait();

    cout << GRAY << graph.vertexes[0]->value << "  -- \\ " << endl;
    cout << graph.vertexes[1]->value << "  --   \\ " << endl;
    cout << "          " << graph.vertexes[4]->value << endl;
    cout << graph.vertexes[2]->value << "  --   /" << endl;
    cout << graph.vertexes[3]->value << "  -- /" << RESET << endl;  

    int flag = 0;
    for(int i = 0 ; i < graph.size_edges ; i ++) {
        for(int j = 0 ; j < 4 ; j ++) {
            string A = graph.edges[i]->A->value;
            string B = graph.edges[i]->B->value;
            if (wantedEdges[j].find(A) != std::string::npos && wantedEdges[j].find(B) != std::string::npos) 
                flag++;
        }
    }
    if (flag != 4) result(FAIL, "graph2");
    else result(PASS, "graph2");
    cout << endl;
}

/*
    /--  E  -- F
  /
A   --\    /-- G
        D
B  --/    \-- H
*/
void graph3() {
    jsch::JobScheduler jobScheduler(10);

    pthread_mutex_t mutex;

    pthread_mutex_init(&mutex, NULL);

    Graph graph(7, 6);

    string wantedEdges[6] = {"AD", "AE", "BD", "EF", "DG", "DH"};

    jsch::JobScheduler::JobPlan* jobPlan1 = jobScheduler.makeJobPlan();

    jsch::JobScheduler::JobPlan* jobPlan2 = jobScheduler.makeJobPlan();

    jobPlan1->addRequiredJob(jsch::make_job(
    [&] {
        pthread_mutex_lock(&mutex);
        Vertex *vertex1 = graph.insertVertex("A");
        if(!vertex1) result(PROBLEM_WITH_TEST, "graph3");

        Vertex *vertex2 = graph.insertVertex("B");
        if(!vertex2) result(PROBLEM_WITH_TEST, "graph3");
        pthread_mutex_unlock(&mutex);
    }));
    

    jobPlan2->addRequiredJob(jsch::make_job(
    [&] {
        pthread_mutex_lock(&mutex);
        Vertex *vertex1 = graph.insertVertex("E");
        if(!vertex1) result(PROBLEM_WITH_TEST, "graph3");
        Edge *edge1 = graph.insertEdge(graph.vertexes[0], vertex1);
        if(!edge1) result(PROBLEM_WITH_TEST, "graph3");

        Vertex *vertex2 = graph.insertVertex("D");
        if(!vertex2) result(PROBLEM_WITH_TEST, "graph3");
        Edge *edge2 = graph.insertEdge(graph.vertexes[0], vertex2);
        if(!edge2) result(PROBLEM_WITH_TEST, "graph3");

        Edge *edge3 = graph.insertEdge(graph.vertexes[1], vertex2);
        if(!edge3) result(PROBLEM_WITH_TEST, "graph3");
        pthread_mutex_unlock(&mutex);
    }));

    jobPlan2->addDependentJob(jsch::make_job(
    [&] {
        pthread_mutex_lock(&mutex);
        Vertex *vertex1 = graph.insertVertex("F");
        if(!vertex1) result(PROBLEM_WITH_TEST, "graph3");
        Edge *edge1 = graph.insertEdge(graph.vertexes[2], vertex1);
        if(!edge1) result(PROBLEM_WITH_TEST, "graph3");

        Vertex *vertex2 = graph.insertVertex("G");
        if(!vertex2) result(PROBLEM_WITH_TEST, "graph3");
        Edge *edge2 = graph.insertEdge(graph.vertexes[3], vertex2);
        if(!edge2) result(PROBLEM_WITH_TEST, "graph3");

        Vertex *vertex3 = graph.insertVertex("H");
        if(!vertex3) result(PROBLEM_WITH_TEST, "graph3");
        Edge *edge3 = graph.insertEdge(graph.vertexes[3], vertex3);
        if(!edge3) result(PROBLEM_WITH_TEST, "graph3");
        pthread_mutex_unlock(&mutex);
    }));

    jobPlan1->addDependentJob(jobPlan2);

    jsch::Future* future = jobScheduler.submitJobWithFuture(jobPlan1);

    future->wait();

    cout << GRAY << "    /--  " << graph.vertexes[2]->value << "  -- " << graph.vertexes[4]->value << endl;
    cout << "  /" << endl;
    cout << graph.vertexes[0]->value << "  --\\    /-- " << graph.vertexes[5]->value << endl;
    cout << "        " << graph.vertexes[3]->value << endl;
    cout << graph.vertexes[1]->value << "  --/    \\-- " << graph.vertexes[6]->value << RESET << endl;

    int flag = 0;
    for(int i = 0 ; i < graph.size_edges ; i ++) {
        for(int j = 0 ; j < 6 ; j ++) {
            string A = graph.edges[i]->A->value;
            string B = graph.edges[i]->B->value;
            if (wantedEdges[j].find(A) != std::string::npos && wantedEdges[j].find(B) != std::string::npos) 
                flag++;
        }
    }
    if (flag != 6) result(FAIL, "graph1");
    else result(PASS, "graph1");
    cout << endl;
}


int main(){
    operationsDoneSequentially();

    storingDoneSequentially();

    firstEvensThenOdds();

    firstEvensThenOddsSequentially();

    graph1();

    graph2();

    graph3();
} 