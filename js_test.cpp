#include <iostream>
#include "jsch.hpp"

int main(){

    jsch::JobScheduler jobScheduler(10);

    pthread_mutex_t mutex;

    pthread_mutex_init(&mutex,NULL);

    const int TOTAL_JOBS = 1000;
    int i = 10;



    // jsch::JobScheduler::JobSequence* jobSequence = jobScheduler.makeJobSequence();
    // for (int j = 0 ; j < TOTAL_JOBS; j++) jobSequence->then(jsch::make_job(
    //     [=] () mutable{
    //         pthread_mutex_lock(&mutex);
    //         std::cout << j << std::endl;
    //         pthread_mutex_unlock(&mutex);
    //     }
    // ));
    // jsch::Future* future = jobScheduler.submitJobWithFuture(jobSequence);

    jsch::JobScheduler::JobPlan* jobPlan1 = jobScheduler.makeJobPlan();

    jsch::JobScheduler::JobPlan* jobPlan2 = jobScheduler.makeJobPlan();

    for (int i = 0 ; i < 50; i++){
        jobPlan1->addRequiredJob(jsch::make_job(
        [=] () mutable{
            if (i % 2 == 0){
                pthread_mutex_lock(&mutex);
                printf("%d\n",i);
                pthread_mutex_unlock(&mutex);
            }
        }));
    }

    jobPlan2->addRequiredJob(
        jsch::make_job(
            [] {
                printf("**************\n");
            }
        )
    );

    for (int i = 0 ; i < 50; i++){
        jobPlan2->addDependentJob(jsch::make_job(
        [=] () mutable{
            if (i % 2 ){
                pthread_mutex_lock(&mutex);
                printf("%d\n",i);
                pthread_mutex_unlock(&mutex);
            }
        }));
    }

    jobPlan1->addDependentJob(jobPlan2);

    jsch::Future* future = jobScheduler.submitJobWithFuture(jobPlan1);

    future->wait();


}