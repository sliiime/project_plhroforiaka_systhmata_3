#include <iostream>
#include "jsch.hpp"

int main(){

    jsch::JobScheduler jobScheduler(10);

    int i = 10;

    jobScheduler.submitJob(jsch::make_job(
            [&]{
                for (int j = 0; j < 10; j++) std::cout << i++ << std::endl;      
            }
        )
    );

    std::cout << i << std::endl;

}