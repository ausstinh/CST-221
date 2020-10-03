/* Includes */
#include <unistd.h>    
#include <sys/types.h>  
#include <errno.h>      
#include <stdio.h>      
#include <stdlib.h>     
#include <pthread.h>    
#include <string.h>     
#include <semaphore.h>  /* Semaphore */

/* prototype for thread routine */
void handler ( void *ptr );

/* global variables */
sem_t mutex;
/* shared variable */
int counter; 

int main()
{
    int people[20];
    pthread_t bus_a;
    pthread_t bus_b;
    
    for(int i = 0; i < 20; i++){
        people[i] = i;
    }  
    
    sem_init(&mutex, 0, 1);      /* initialize mutex to 1 - binary semaphore */
                                 /* second param = 0 - semaphore is local */
                                 
    /* Note: you can check if thread has been successfully created by checking return value of
       pthread_create */

    //threads are buses
    pthread_create (&bus_a, NULL, (void *) &handler, (void *) &people[0]);
    pthread_create (&bus_b, NULL, (void *) &handler, (void *) &people[1]);
                             
    pthread_join(bus_a, NULL);
    pthread_join(bus_b, NULL);

    sem_destroy(&mutex); /* destroy semaphore */
                  
    /* exit */  
    exit(0);
} /* main() */

void handler ( void *ptr )
{
    int x; 
    x = *((int *) ptr);
    printf("Thread %d: Bus incoming to pick up 10 people\n", x);
    sem_wait(&mutex);       /* down semaphore */
    /* START CRITICAL REGION */
    printf("Thread %d: Bus has arrived\n", x);
    //Bus can only fit 10 people
    for(int i = 0; i < 10; i++){
        printf("Thread %d: Next Person is arriving\n", x);
        counter++;
        printf("Thread %d: Person %d has hopped on the Bus\n", x, counter);
    }
    printf("Thread %d: Bus is going to drop of people\n", x);
    /* END CRITICAL REGION */    
    sem_post(&mutex);       /* up semaphore */
    
    pthread_exit(0); /* exit thread */
}
