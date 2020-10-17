/* Includes */
#include <unistd.h>     /* Symbolic Constants */
#include <sys/types.h>  /* Primitive System Data Types */ 
#include <errno.h>      /* Errors */
#include <stdio.h>      /* Input/Output */
#include <stdlib.h>     /* General Utilities */
#include <pthread.h>    /* POSIX Threads */
#include <string.h>     /* String handling */
#include <signal.h>
#include <sys/mman.h>
#include <semaphore.h>  /* Semaphore */

/* Global Variables */
sem_t* semaphore;
pid_t otherPid;
sigset_t sigSet;
pthread_t tid1;
char file[100];
void childProcess()
{
    int value;
    //get semaphore value
    sem_getvalue(semaphore, &value);
    printf("Detailed search file count is %d.\n", value);
    printf("Detailed search is grabbing the file. \n");
    sem_wait(semaphore);
    sem_getvalue(semaphore, &value);
    printf("Detailed search file count is %d.\n", value);
    //START CRITICAL REGION
    printf("Starting to do a detailed search...\n");
    for(int i =0; i < 60; ++i){
        printf(".\n");
        sleep(1);
    }
    //END CRITICAL REGION
    sem_post(semaphore);


    //Exit child process
    printf("Exit detailed search\n");
    _exit(0);
}

void* checkHungChild(void*a)
{
    int* status = a;
    //check if child process is waiting long
    printf("Checking if detailed search is doing O.K.....\n");
    sleep(10);
    //check if there is a lock on the semaphore
    if(sem_trywait(semaphore) != 0){
        printf("detailed search appears to be taking a while...\n");
        *status = 1;
    }
    else{
        printf("detailed search appears to be running fine...\n");
        *status = 0;
    }
    return NULL;
}
void parentProcess()
{
    //Detect hung Child Process and kill it after a timeout
    sleep(2);
    //check if child process is running
    if(getpgid(otherPid) >= 0){
        printf("detailed search is running...\n");
    }
    int value;
    sem_getvalue(semaphore, &value);
    printf("The fast search has a total of %d files related to your input.\n", value);
   //check if there is a lock on the semaphore
    if(sem_trywait(semaphore) != 0){
        pthread_t tid1;
        int status = 0;
        printf("Detecting if detailed search's hung or running to long to find your file....\n");
      //check if child process is running a long time
        if(pthread_create(&tid1, NULL, checkHungChild, &status))
        {
            printf("ERROR creating timer thread\n");
            _exit(1);
        }
        if(pthread_join(tid1,NULL)){
            printf("\n ERROR joining timer thread.\n");
            exit(1);
        }
        if(status ==1){
            //Kill child process
            printf("Stopping detailed search on your computer with ID of %d.\n", value);
            //kill(otherPid, SIGUSR1);
            kill(otherPid, SIGTERM);
            printf("detailed search ended.\n");

            //Prove that the child process is killed
            printf("Checking if detailed search has been terminated\n");
            sleep(5);
            kill(otherPid,SIGUSR2);
            sleep(1);
            printf("Confirmed detailed search is done.\n");
            printf("Started a fast search.\n");
            //Try to get semaphore
            sem_getvalue(semaphore, &value);
            printf("Your fast search finds %d file(s).\n", value);
            if(sem_trywait(semaphore) != 0)
            {
                if(value == 0)
                {
                    sem_post(semaphore);
                    printf("Cleaned up and finally got the file.\n");
                    sem_getvalue(semaphore, &value);
                    printf("The fast search got %d file related to your input.\n", value);

                }
                else{
                    printf("Finally got the file.\n");
                }
                //check if the found file is corrupt
                //if so, delete the file
                printf("Your file is %s.\n", file);
                if(strcmp(file, "file2") == 0){
                    printf("file is corrupted. Deleting file\n");
                    sem_destroy(semaphore);
                }
                
            }
            printf("Exit fast search.\n");
                _exit(0);
        }
    }
}
//Utility Methods
void signalHandeler1(int signum){
    printf("Caught Signal: %d\n", signum);
    printf(" Exit detailed search Process\n");
    sem_post(semaphore);
    _exit(0);
}
void signalHandeler2(int signum){
    printf("I am alive!\n");
}

int main(int argc, char* argv[])
{
     pid_t pid;
     //ask for user input and make sure input is not empty
     //and its a real file (file1 does not exist)
  do{
    printf("Please input a real file:   ");
    scanf(" %s", file);
  }while(file==NULL || strcmp(file, "file1") == 0 );
    //create shared semaphore
    semaphore = (sem_t*)mmap(0,sizeof(sem_t), PROT_READ|
    PROT_WRITE,MAP_SHARED|
    MAP_ANONYMOUS, -1, 0);

    if(sem_init(semaphore,1,1) != 0){
        printf("failed to create semaphore.\n");
        exit(EXIT_FAILURE);
    }

    //use fork()
    setvbuf(stdout, NULL,_IOLBF,0); //so redirect works
    pid = fork();
     if (pid == -1)
    {
        // Error: If fork() returns -1 then an error happened (for example, number of processes reached the limit).
        fprintf(stderr,"Can't fork, error %d\n", errno);
        exit(EXIT_FAILURE);
    }
     printf("Searching your computer for file!\n");
    // OK: If fork() returns non zero then the parent process is running else child process is running
    if (pid == 0)
    {
        printf("fork() returned 0 to run a fast level search\n");
        // Run Producer Process logic as a Child Process
        otherPid = getppid();
        childProcess();
    }
    else
    {
        printf("fork() returned non zero number to run a detailed search\n");
        // Run Consumer Process logic as a Parent Process
        otherPid = pid;
        parentProcess();
    }
    //Cleanup
    sem_destroy(semaphore);

    // Return OK
    return 0;
} /* main() */


