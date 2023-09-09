#define _GNU_SOURCE
#include <pthread.h>    /* It consists of the details and functions associated with threads */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sched.h>      /* scheduling parameters required for implementation of each supported scheduling policy.*/

#define NUM_THREADS 64
#define NUM_CPUS 8


/* A structure to store the thread index parameter */
typedef struct
{
    int threadIdx;
} threadParams_t;


/* POSIX thread declarations and scheduling attributes */ 
pthread_t threads[NUM_THREADS];
pthread_t mainthread;
pthread_t startthread;
threadParams_t threadParams[NUM_THREADS];

//thread attributed object used to define the attributes for thread creation 
pthread_attr_t fifo_sched_attr;
pthread_attr_t orig_sched_attr;

//used to specify the scheduling parameters for a thread
struct sched_param fifo_param;

#define SCHED_POLICY SCHED_FIFO
#define MAX_ITERATIONS (1000000)


/*******************************************************************************
* @Function Name: print_scheduler
* @Description: The function returns the scheduling policy set currently 
* to the current thread running the task
* @input param: None
* @return: None
*******************************************************************************/
void print_scheduler(void)
{
   int schedType;

/* sched_setscheduler() sets both the scheduling policy and the associated parameters
 for the process whose ID is specified in pid.*/

   schedType = sched_getscheduler(getpid());

   switch(schedType)
   {

/* Process will run until they voluntarily yield the CPU or are preempted by a higher-priority process. */
     case SCHED_FIFO:
           printf("Pthread Policy is SCHED_FIFO\n");
           break;

/* The default scheduling policy for non-real-time processes. It uses a time-sharing algorithm 
and allows the system to dynamically allocate CPU time among eligible processes. */
     case SCHED_OTHER:
           printf("Pthread Policy is SCHED_OTHER\n");
       break;

/* Processes are given a time slice (quantum) to execute before being preempted and placed
 at the end of the scheduling queue */
     case SCHED_RR:
           printf("Pthread Policy is SCHED_RR\n");
           break;
     default:
       printf("Pthread Policy is UNKNOWN\n");
   }
}


/*******************************************************************************
* @Function Name: set_scheduler
* @Description: CPU affinity is set and the running process is set to SCHED_FIFO
* @input param: None
* @return: None
*******************************************************************************/
void set_scheduler(void)
{
    int max_prio, scope, rc, cpuidx;
    cpu_set_t cpuset;

    printf("INITIAL "); print_scheduler();

/* Initialize thread attributes object */
    pthread_attr_init(&fifo_sched_attr);

/* Threads take their scheduling attributes from the values specified by the attributes object. */
    pthread_attr_setinheritsched(&fifo_sched_attr, PTHREAD_EXPLICIT_SCHED);

/* Sets the scheduling policy of the thread to SCHED_FIFO */
    pthread_attr_setschedpolicy(&fifo_sched_attr, SCHED_POLICY);
    CPU_ZERO(&cpuset);
    cpuidx=(3);
    CPU_SET(cpuidx, &cpuset);

/* Sets the CPU affinity to Core 4 mentioned in cpuset */
    pthread_attr_setaffinity_np(&fifo_sched_attr, sizeof(cpu_set_t), &cpuset);

/* Threads are assigned the maximum priority that the policy can offer */
    max_prio=sched_get_priority_max(SCHED_POLICY);
    fifo_param.sched_priority=max_prio;    

/* Sets the scheduling policy of the current process */
    if((rc=sched_setscheduler(getpid(), SCHED_POLICY, &fifo_param)) < 0)
        perror("sched_setscheduler");

/* scheduling parameters are set in the thread attributes object */
    pthread_attr_setschedparam(&fifo_sched_attr, &fifo_param);

    printf("ADJUSTED "); print_scheduler();
}



/*******************************************************************************
* @Function Name: counterThread
* @Description: Threads entry point 
* @input param: Thread attributes
* @return: None
*******************************************************************************/
void *counterThread(void *threadp)
{
    int sum=0, i, rc, iterations;
    threadParams_t *threadParams = (threadParams_t *)threadp;
    pthread_t mythread;
    double start=0.0, stop=0.0;
    struct timeval startTime, stopTime;

/* function gives the number of seconds and microseconds since the Epoch */
/* start time stamp */
    gettimeofday(&startTime, 0);
    start = ((startTime.tv_sec * 1000000.0) + startTime.tv_usec)/1000000.0;

//do work
    for(iterations=0; iterations < MAX_ITERATIONS; iterations++)
    {
        sum=0;
        for(i=1; i < (threadParams->threadIdx)+1; i++)
            sum=sum+i;
    }

/* stop time stamp */
    gettimeofday(&stopTime, 0);
    stop = ((stopTime.tv_sec * 1000000.0) + stopTime.tv_usec)/1000000.0;

    printf("\nThread idx=%d, sum[0...%d]=%d, running on CPU=%d, start=%lf, stop=%lf", 
           threadParams->threadIdx,
           threadParams->threadIdx, sum, sched_getcpu(),
           start, stop);
}

/*******************************************************************************
* @Function Name: starterThread
* @Description: Starter thread entry point which creates the worker threads
* @input param: Thread attributes
* @return: None
*******************************************************************************/
void *starterThread(void *threadp)
{
   int i, rc;

   printf("starter thread running on CPU=%d\n", sched_getcpu());

   for(i=0; i < NUM_THREADS; i++)
   {
       threadParams[i].threadIdx=i;

       pthread_create(&threads[i],   // pointer to thread descriptor
                      &fifo_sched_attr,     // use FIFO RT max priority attributes
                      counterThread, // thread function entry point
                      (void *)&(threadParams[i]) // parameters to pass in
                     );

   }

/* Waits for all the worker threads to complete */
   for(i=0;i<NUM_THREADS;i++)
       pthread_join(threads[i], NULL);

}

/*******************************************************************************
* @Function Name: main
* @Description: The function is teh main application starting point which sets
* the scheduling policy as well as priority of the threads as per requirement
* @input param: None
* @author: nodified by rishab shah
* @return: success/failure
*******************************************************************************/
int main (int argc, char *argv[])
{
   int rc;
   int i, j;
   cpu_set_t cpuset;

   //set up the fifo policy, priority and amp emulation affinity here
   set_scheduler();

  /* Clears set, so that it contains no CPUs*/
   CPU_ZERO(&cpuset);

   /*  get affinity set for main thread */
   mainthread = pthread_self();

   // Check the affinity mask assigned to the thread 
   rc = pthread_getaffinity_np(mainthread, sizeof(cpu_set_t), &cpuset);
   if (rc != 0)
       perror("pthread_getaffinity_np");
   else
   {
       printf("main thread running on CPU=%d, CPUs =", sched_getcpu());

       for (j = 0; j < CPU_SETSIZE; j++)
           if (CPU_ISSET(j, &cpuset))
               printf(" %d", j);

       printf("\n");
   }

/* create the starter thread */
   pthread_create(&startthread,   // pointer to thread descriptor
                  &fifo_sched_attr,     // use FIFO RT max priority attributes
                  starterThread, // thread function entry point
                  (void *)0 // parameters to pass in
                 );

/* wait for starter thread to terminate */
   pthread_join(startthread, NULL);

   printf("\nTEST COMPLETE\n");
}
