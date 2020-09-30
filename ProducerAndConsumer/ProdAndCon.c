#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include  <signal.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <errno.h>

// Constants
int MAX = 100;
int WAKEUP = SIGUSR1;


// The Child PID if the Parent else the Parent PID if the Child
pid_t otherPid;

// A Signal Set
sigset_t sigSet;

// Shared Circular Buffer
struct CIRCULAR_BUFFER
{
    int count;          // Number of items in the buffer
    int lower;          // Next slot to read in the buffer
    int upper;          // Next slot to write in the buffer
    int buffer[100];
};
struct CIRCULAR_BUFFER *buffer = NULL;

/****************************************************************************************************/

// This method will put the current Process to sleep forever until it is awoken by the WAKEUP signal
void sleepAndWait()
{
    int nSig;

    printf("Sleeping...........\n");
    // TODO: Sleep until notified to wake up using the sigwait() method
    sigwait(&sigSet, &nSig);
    printf("Awoken\n");
}

// This method will signal the Other Process to WAKEUP
void wakeupOther()
{
	//Signal Other Process to wakeup using the kill() method
    kill(otherPid, WAKEUP);
}

// Gets a value from the shared buffer
int getValue()
{
    // Get a value from the Circular Buffer and adjust where to read from next   
    int value = (int) buffer->buffer[buffer->lower];
    buffer->lower = buffer->lower + 1;
    return value;
}

// Puts a value in the shared buffer
void putValue(struct CIRCULAR_BUFFER* buffer, int value)
{
   //Write to the next available position in the Circular Buffer and adjust where to write next
   buffer->buffer[buffer->upper] = value;
   buffer->upper = buffer->upper + 1;
}

/****************************************************************************************************/

/**
 * Logic to run to the Consumer Process
 **/
void consumer()
{
    // Set Signal Set to watch for WAKEUP signal
    sigemptyset(&sigSet);
    sigaddset(&sigSet, WAKEUP);
    sigprocmask(SIG_BLOCK, &sigSet, NULL);

    // Run the Consumer Logic
    printf("Running Consumer Process.....\n");
    
    //Implement Consumer Logic
    int value = 0;
      while (value <= 300) 
    {
        //if buffer is empty make consumer process sleep
        if (buffer->count == 0) 
        {
            sleepAndWait();
        }
        //print consumer value
        printf("Consumer: ");
        value = getValue();
        printf("Value: %d\n", value);

        //update buffer count -1
        buffer->count = buffer->count - 1;

        //if the buffer is full start producer process again
        if (buffer->count == 99) 
        {
            wakeupOther();
        }
    }
    // Exit cleanly from the Consumer Process
    _exit(1);
}

/**
 * Logic to run to the Producer Process
 **/
void producer()
{
    // Set Signal Set to watch for WAKEUP signal
    sigemptyset(&sigSet);
    sigaddset(&sigSet, SIGUSR1);
    sigprocmask(SIG_BLOCK, &sigSet, NULL);

    // Buffer value to write
    int value = 100;
    // Run the Producer Logic
    printf("Running Producer Process.....\n");
    
    // TODO: Implement Producer Logic (see page 129 in book)
    while (value <= 300) 
    { 
       
        //if buffer is fill make producer sleep and wait.
        if (buffer->count == MAX) 
        {
            sleepAndWait();
        }
        //print producer value
        printf("Producer: ");
        putValue(buffer, value);
        printf("Value: %d\n", value);

        //add +1 to value
        value++;

        //update buffer count +1 
        buffer->count = buffer->count + 1;

        //wake up consumer if the buffer has a value in it
        if (buffer->count == 1) 
        {
            wakeupOther();
        }
    }
}

/**
 * Main application entry point to demonstrate forking off a child process that will run concurrently with this process.
 *
 * @return 1 if error or 0 if OK returned to code the caller.
 */
int main(int argc, char* argv[])
{
    pid_t  pid;

    // Create shared memory for the Circular Buffer to be shared between the Parent and Child  Processes
    buffer = (struct CIRCULAR_BUFFER*)mmap(0,sizeof(buffer), PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    buffer->count = 0;
    buffer->lower = 0;
    buffer->upper = 0;

    // Use fork()
    pid = fork();
    if (pid == -1)
    {
        // Error: If fork() returns -1 then an error happened (for example, number of processes reached the limit).
        fprintf(stderr,"Can't fork, error %d\n", errno);
        exit(EXIT_FAILURE);
    }
    // OK: If fork() returns non zero then the parent process is running else child process is running
    if (pid == 0)
    {
        printf("fork() returned 0 to run parent process\n");
        // Run Producer Process logic as a Child Process
        otherPid = getppid();
        producer();
    }
    else
    {
        printf("fork() returned non zero number to run child process\n");
        // Run Consumer Process logic as a Parent Process
        otherPid = pid;
        consumer();
    }

    // Return OK
    return 0;
}
