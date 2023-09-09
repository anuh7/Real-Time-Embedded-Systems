/******************************************************************************
* @file: pthread.c
* @brief: Assignment to make the incdecthread code deterministic such that only when increment
            thread is finished, the decrement thread will start.
* @author: Modified by Anuhya, code provided in class-incdecthread.c
* @date:  15-June-2023
* @references: http://www.csc.villanova.edu/~mdamian/threads/posixsem.html#init
                https://pages.cs.wisc.edu/~remzi/Classes/537/Fall2008/Notes/threads-semaphores.txt
                https://dextutor.com/process-synchronization-using-semaphores/
*******************************************************************************/



#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sched.h>
#include <semaphore.h>

#define COUNT  1000

typedef struct
{
    int threadIdx;
    sem_t *semaphore;       /* sem_t type, used in performing semaphore operations*/
} threadParams_t;


// POSIX thread declarations and scheduling attributes
pthread_t threads[2];
threadParams_t threadParams[2];
sem_t semaphore;


// Unsafe global
int gsum=0;

/*******************************************************************************
* @Function Name: incThread
* @Description: thread entry point for incrementing the global value
* @input param: Thread attributes
* @return : None
*******************************************************************************/
void *incThread(void *threadp)
{
    int i;
    threadParams_t *threadParams = (threadParams_t *)threadp;

    for(i=0; i<COUNT; i++)
    {
        sem_wait(threadParams->semaphore);
        gsum=gsum+i;
        printf("Increment thread idx=%d, gsum=%d\n", threadParams->threadIdx, gsum);
        sem_post(threadParams->semaphore);
    }
}

/*******************************************************************************
* @Function Name: decThread
* @Description: thread entry point for decrementing the global value
* @input param: Thread attributes
* @return : None
*******************************************************************************/
void *decThread(void *threadp)
{
    int i;
    threadParams_t *threadParams = (threadParams_t *)threadp;

    for(i=0; i<COUNT; i++)
    {
        sem_wait(threadParams->semaphore); 
        gsum=gsum-i;
        printf("Decrement thread idx=%d, gsum=%d\n", threadParams->threadIdx, gsum);
        sem_post(threadParams->semaphore);
    }
}

/*******************************************************************************
* @Function Name: main
* @Description: The function is the main application starting point which creates 
* the two threads and waits till they terminate
* @input param: None
* @return: success/failure
*******************************************************************************/
int main (int argc, char *argv[])
{
   int rc;
   int i=0;

    sem_init(&semaphore, 0, 1);

   threadParams[i].threadIdx=i;
   threadParams[i].semaphore = &semaphore;
   pthread_create(&threads[i],   // pointer to thread descriptor
                  (void *)0,     // use default attributes
                  incThread, // thread function entry point
                  (void *)&(threadParams[i]) // parameters to pass in
                 );
   
   i++;

   threadParams[i].threadIdx=i;
   threadParams[i].semaphore = &semaphore;
   pthread_create(&threads[i], (void *)0, decThread, (void *)&(threadParams[i]));

   for(i=0; i<2; i++)
     pthread_join(threads[i], NULL);

    sem_destroy(&semaphore);

   printf("TEST COMPLETE\n");
}
