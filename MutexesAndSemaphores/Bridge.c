//THIS IS MUTEXE EXAMPLE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include  <signal.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <pthread.h>
#define CARS_SEEN 10000

int cars = 0;
pthread_mutex_t mutex;
void *driveAcrossBridge(void*a){
    int x, tmp;
    for(x =0; x < CARS_SEEN; ++x){

        //start of critical region
        pthread_mutex_lock(&mutex);
        //Copy the number of cars crossing the bridge to a local variable,
        //add 1 to the total, and save the total back in the global variable
        tmp = cars;
        tmp = tmp + 1;
        cars = tmp;
        //end of critical region
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void *thread1(void*a){
    int x;
    for(x=0; x < 10; ++x){
        printf("This is lane 1....%d\n",x);
        sleep(1);
    }
    return NULL;
}

int main(){

    pthread_mutex_init(&mutex, 0);

    pthread_t lane1;

   
    //Create 1 thread
    if(pthread_create(&lane1, NULL, driveAcrossBridge, NULL)){
        printf("\n lane 1 could not be created");
        exit(1);
    }

   
    if(pthread_join(lane1, NULL)){
         printf("\n cars could not get on lane 1");
        exit(1);
    }
    //if the number of vechicles on the bridge is more than 
    //the total amount that can be on the bridge
    if(cars < (CARS_SEEN)){
        printf("\n  %d number of cars crossed the bridge than the usual of %d during rush hour.\n", cars, 2*CARS_SEEN);
    }
    else{
         printf("\n The usual amount of cars of %d crossed the bridge during rush hour.\n", cars);
    }

    pthread_mutex_destroy(&mutex);
}

