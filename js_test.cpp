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

enum Test {PASS, FAIL};

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
                printf("%d\n", i);
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
                printf("%d\n", i);
                pthread_mutex_unlock(&mutex);
            }
        }));
    }
    jobPlan2->addDependentJob(jobSequence2);

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
                result(FAIL, "First evens then odds sequentially");
                return;
            } 
        } else {
            if(array[i] % 2) {
                result(FAIL, "First evens then odds sequentially");
                return;
            } 
        }

    }
    if(passedZero) result(PASS, "First evens then odds sequentially");
    else result(FAIL, "First evens then odds sequentially");
}



void storingThenSorting() {
    jsch::JobScheduler jobScheduler(10);

    pthread_mutex_t mutex;

    pthread_mutex_init(&mutex, NULL);

    int array[80];
    int sortedArray[80];
    int index = 0;

    jsch::JobScheduler::JobPlan* jobPlan = jobScheduler.makeJobPlan();

    for (int i = 20 ; i <= 100; i++){
        jobPlan->addRequiredJob(jsch::make_job(
        [&, i]()mutable {
            pthread_mutex_lock(&mutex);
            array[index++] = i;
            //printf("%d\n", i);
            pthread_mutex_unlock(&mutex);
        }));
    }
    
    for (int i = 0 ; i <= 80; i++){
        jobPlan->addDependentJob(jsch::make_job(
        [&, i]()mutable {
            pthread_mutex_lock(&mutex);
            sortedArray[array[i] - 20] = array[i];
            pthread_mutex_unlock(&mutex);
        }));
    }

    jsch::Future* future = jobScheduler.submitJobWithFuture(jobPlan);

    future->wait();
}


int main(){

    operationsDoneSequentially();

    storingDoneSequentially();

    firstEvensThenOdds();

    storingThenSorting();

    firstEvensThenOddsSequentially();
}