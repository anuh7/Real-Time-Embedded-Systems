
/******************************************************************************
* @file: rmscheduler.c
* @author: Anuhya
* @date:  14-June-2023
* @references: scheduling_analysis.c, lab1.c provided by Professor Sam siewart
*******************************************************************************/

/*******************************************************************************
Header files
*******************************************************************************/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>   /* It consists of the details and functions associated with threads */
#include <sched.h>     /* scheduling parameters required for implementation of each supported scheduling policy.*/
#include <time.h>
#include <semaphore.h> /* performing semaphore operations*/
#include <syslog.h>    /* to use syslog functionality for logging */
#include <sys/time.h>
#include <sys/sysinfo.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>



/*******************************************************************************
Macros
*******************************************************************************/
#define FIB_TEST_CYCLES (500)
#define FIB_LIMIT (10)
#define MY_SCHED_POLICY             (SCHED_FIFO)
#define F10_RUN_TIME (10.0)
#define F20_RUN_TIME (20.0)
#define CPU_CORE (3)


/*******************************************************************************
Global Variables
*******************************************************************************/
int abortTest = 0;
double current_realtime;

unsigned int idx = 0, jdx = 1;
unsigned int fib = 0, fib0 = 0, fib1 = 1;

/*******************************************************************************
Function Prototypes
*******************************************************************************/
void *Sequencer(void *threadp);
void *fib10(void *threadp);
void *fib20(void *threadp);
double getTimeMsec(void);
double realtime(struct timespec *tsptr);
void print_scheduler(void);

/* sem_t type, used in performing semaphore operations*/
sem_t semF10, semF20;

/* Synthetic workload: function MACRO used to generate a fibonacci sequence. */
#define FIB_TEST(seqCnt, iterCnt)      \
   for(idx=0; idx < iterCnt; idx++)    \
   {                                   \
      fib0=0; fib1=1; jdx=1;           \
      fib = fib0 + fib1;               \
      while(jdx < seqCnt)              \
      {                                \
         fib0 = fib1;                  \
         fib1 = fib;                   \
         fib = fib0 + fib1;            \
         jdx++;                        \
      }                                \
   }          

//structure to hold thread index and time period
typedef struct
{
    int threadIdx;
    int MajorPeriods;
} threadParams_t;


/*******************************************************************************
* @Function Name: fib10
* @Description: The function burns the CPU cycles
* @input param: Thread attributes
* @credit : lab1.c provided in class
* @return : None
*******************************************************************************/
void *fib10(void *threadp)
{
  double event_time, run_time=0.0;
  int limit=0, release=0, cpucore;
  unsigned int required_test_cycles;

/* Run Fibonacci sequence for 10 numbers for 500 iterations */
  FIB_TEST(FIB_LIMIT, FIB_TEST_CYCLES); 
  event_time = getTimeMsec();
  FIB_TEST(FIB_LIMIT, FIB_TEST_CYCLES);
  run_time = getTimeMsec() - event_time;    //Run time of the function taken by the CPU

/* dynamic calculation to get the number of cycles to run for 10ms task*/
  required_test_cycles = (int)(F10_RUN_TIME/run_time);
  syslog(LOG_INFO,"F10 runtime calibration %lf msec per %d test cycles, so %u required\n", run_time, FIB_TEST_CYCLES,required_test_cycles);
  printf("F10 runtime calibration %lf msec per %d test cycles, so %u required\n", run_time, FIB_TEST_CYCLES,required_test_cycles);

  while(!abortTest)
  {
    sem_wait(&semF10); 

    if(abortTest)
      break; 
    else 
      release++;      //task iteration number

    cpucore=sched_getcpu();
    syslog(LOG_INFO,"***F10 start %d @ %lf on core %d\n", release, (event_time=getTimeMsec() - current_realtime), cpucore);
    printf("***F10 start %d @ %lf on core %d\n", release, (event_time=getTimeMsec() - current_realtime), cpucore);

    do
    {
      FIB_TEST(FIB_LIMIT, FIB_TEST_CYCLES);
      limit++;
    }
    while(limit < required_test_cycles);

    syslog(LOG_INFO,"***F10 complete %d @ %lf, %d loops\n", release, (event_time=getTimeMsec() - current_realtime), limit);  
  printf("***F10 complete %d @ %lf, %d loops\n", release, (event_time=getTimeMsec() - current_realtime), limit);
    limit=0;
  }

  pthread_exit((void *)0);
}

/*******************************************************************************
* @Function Name: fib20
* @Description: The function burns the CPU cycles
* @credit : Lab1.c provided in class
* @input param: Thread attributes
* @return: None
*******************************************************************************/
void *fib20(void *threadp)
{
  double event_time, run_time=0.0;
  int limit=0, release=0, cpucore;
  unsigned int required_test_cycles;

  FIB_TEST(FIB_LIMIT, FIB_TEST_CYCLES); 
  event_time = getTimeMsec();
  FIB_TEST(FIB_LIMIT, FIB_TEST_CYCLES);
  run_time = getTimeMsec() - event_time;

  required_test_cycles = (int)(F20_RUN_TIME/run_time);
  syslog(LOG_INFO,"F20 runtime calibration %lf msec per %d test cycles, so %u required\n", run_time, FIB_TEST_CYCLES,required_test_cycles);
  printf("F20 runtime calibration %lf msec per %d test cycles, so %u required\n", run_time, FIB_TEST_CYCLES,required_test_cycles);

  while(!abortTest)
  {
    sem_wait(&semF20); 

    if(abortTest)
      break; 
    else 
      release++;

    cpucore=sched_getcpu();
    syslog(LOG_INFO,"***F20 start %d @ %lf on core %d\n", release, (event_time=getTimeMsec() - current_realtime), cpucore);
    printf("***F20 start %d @ %lf on core %d\n", release, (event_time=getTimeMsec() - current_realtime), cpucore);
    do
    {
      FIB_TEST(FIB_LIMIT, FIB_TEST_CYCLES);
      limit++;
    }
    while(limit < required_test_cycles);

    syslog(LOG_INFO,"***F20 complete %d @ %lf, %d loops\n", release, (event_time=getTimeMsec() - current_realtime), limit);  
   printf("***F20 complete %d @ %lf, %d loops\n", release, (event_time=getTimeMsec() - current_realtime), limit); 
    limit=0;
  }

  pthread_exit((void *)0);
}

/*******************************************************************************
* @Function Name: getTimeMsec
* @Description: The function gets the current time in a double format
* @return: None
*******************************************************************************/
double getTimeMsec(void)
{
  struct timespec event_ts = {0, 0};

  clock_gettime(CLOCK_MONOTONIC, &event_ts);
  return ((event_ts.tv_sec)*1000.0) + ((event_ts.tv_nsec)/1000000.0);
}


double realtime(struct timespec *tsptr)
{
    return ((double)(tsptr->tv_sec) + (((double)tsptr->tv_nsec)/1000000000.0));
}

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

   schedType = sched_getscheduler(getpid());

   switch(schedType)
   {
     case SCHED_FIFO:
           printf("Pthread Policy is SCHED_FIFO\n");
           break;
     case SCHED_OTHER:
           printf("Pthread Policy is SCHED_OTHER\n"); exit(-1);
       break;
     case SCHED_RR:
           printf("Pthread Policy is SCHED_RR\n"); exit(-1);
           break;
     default:
       printf("Pthread Policy is UNKNOWN\n"); exit(-1);
   }

}

/*******************************************************************************
* @Function Name: Sequencer
* @Description: Sequencer thread entry function triggered every 10ms;
* Generates tasks F10, F20 for their period with use of semaphores
* @input param: Thread parameters 
* @return: None
*******************************************************************************/
void *Sequencer(void *threadp)
{
	// struct timespec current_time_val;
    struct timespec delay_time = {0,10000000}; // delay for 10.0 msec, 100 Hz
    struct timespec remaining_time;

    int rc;
    unsigned long long seqCnt=0;
    threadParams_t *threadParams = (threadParams_t *)threadp;

    current_realtime=getTimeMsec();
    syslog(LOG_INFO, "Sequencer thread @ msec=%6.9lf\n", current_realtime);
    printf("Sequencer thread @ msec=%6.9lf\n", current_realtime);

      do{
        if((rc=clock_nanosleep(CLOCK_MONOTONIC, 0, &delay_time, &remaining_time)) != 0)
        {
            perror("Sequencer nanosleep");
            exit(-1);
        }

        printf("sequencer at msec=%6.9lf\n", getTimeMsec()); 
        seqCnt++;
        
        // f10 @ RT_MAX-1	@ 50 Hz ie. every 20ms
        if((seqCnt % 2) == 0) sem_post(&semF10);

        // f20 @ RT_MAX-2	@ 20 Hz ie. every 50ms
        if((seqCnt % 5) == 0) sem_post(&semF20);

      } while(!abortTest && (seqCnt < 30));     //sequence is run for 300ms

      sem_post(&semF10); sem_post(&semF20);
      abortTest=1;

    pthread_exit((void *)0);
}

/*******************************************************************************
* @Function Name: main
* @Description: The function is the main application starting point which sets
* the scheduling policy as well as priority of the threads as per requirement
* @input param: None
* @return: success/failure
*******************************************************************************/
int main()
{
  int i, rc, scope;
 
  cpu_set_t threadcpu;
  cpu_set_t allcpuset;

  pthread_t threads[3];
  threadParams_t threadParams[3];
  pthread_attr_t rt_sched_attr[3];

  int rt_max_prio, rt_min_prio;

  struct sched_param rt_param[3];
  struct sched_param main_param;
 
  pthread_attr_t main_attr;
  pid_t mainpid;


  abortTest=0;

  syslog(LOG_INFO,"System has %d processors configured and %d available.\n", get_nprocs_conf(), get_nprocs());
  printf("System has %d processors configured and %d available.\n", get_nprocs_conf(), get_nprocs());

  CPU_ZERO(&allcpuset);
  for(i=0; i < 1; i++)
    CPU_SET(i, &allcpuset);

  syslog(LOG_INFO,"Using CPUS=%d from total available.\n", CPU_COUNT(&allcpuset));
   printf("Using CPUS=%d from total available.\n", CPU_COUNT(&allcpuset));

  /* initialize the sequencer semaphores */
  if (sem_init (&semF10, 0, 0)) { syslog(LOG_INFO,"Failed to initialize semF10 semaphore\n"); exit (-1); }
  if (sem_init (&semF20, 0, 0)) { syslog(LOG_INFO,"Failed to initialize semF20 semaphore\n"); exit (-1); }

  mainpid=getpid();

  rt_max_prio = sched_get_priority_max(MY_SCHED_POLICY);
  rt_min_prio = sched_get_priority_min(MY_SCHED_POLICY);
  
  syslog(LOG_INFO,"rt_max_prio=%d\n", rt_max_prio);
  printf("rt_max_prio=%d\n", rt_max_prio);
  syslog(LOG_INFO,"rt_min_prio=%d\n", rt_min_prio);
  printf("rt_min_prio=%d\n", rt_min_prio);

  rc=sched_getparam(mainpid, &main_param);
  main_param.sched_priority=rt_max_prio;
  rc=sched_setscheduler(getpid(), MY_SCHED_POLICY, &main_param);
  if(rc < 0) perror("main_param");
  print_scheduler();

  /* Get the current scope settings */
  pthread_attr_getscope(&main_attr, &scope);

  if(scope == PTHREAD_SCOPE_SYSTEM)
    syslog(LOG_INFO,"PTHREAD SCOPE SYSTEM\n");
   // printf("PTHREAD SCOPE SYSTEM\n");
  else if (scope == PTHREAD_SCOPE_PROCESS)
    syslog(LOG_INFO,"PTHREAD SCOPE PROCESS\n");
   // printf("PTHREAD SCOPE PROCESS\n");
  else
    syslog(LOG_INFO,"PTHREAD SCOPE UNKNOWN\n");
  //  printf("PTHREAD SCOPE UNKNOWN\n");



  for(i=0; i < 3; i++)
  {
    CPU_ZERO(&threadcpu);
    CPU_SET(CPU_CORE, &threadcpu);

     /* Set policy for FIFO operation and CPU set of 3 for all the threads*/

    rc=pthread_attr_init(&rt_sched_attr[i]);
    rc=pthread_attr_setinheritsched(&rt_sched_attr[i], PTHREAD_EXPLICIT_SCHED);
    rc=pthread_attr_setschedpolicy(&rt_sched_attr[i], MY_SCHED_POLICY);
    rc=pthread_attr_setaffinity_np(&rt_sched_attr[i], sizeof(cpu_set_t), &threadcpu);

    threadParams[i].threadIdx=i;
  }
   
  syslog(LOG_INFO,"Service threads will run on %d CPU cores\n", CPU_COUNT(&threadcpu));
  printf("Service threads will run on %d CPU cores\n", CPU_COUNT(&threadcpu));
  printf("Start sequencer\n");
  //threadParams[0].MajorPeriods=10;

  //sequencer thread
  rt_param[0].sched_priority=rt_max_prio;
  pthread_attr_setschedparam(&rt_sched_attr[0], &rt_param[0]);
  rc=pthread_create(&threads[0], &rt_sched_attr[0], Sequencer, (void *)&(threadParams[0]));

  // serviceF10
  rt_param[1].sched_priority=rt_max_prio-1;
  pthread_attr_setschedparam(&rt_sched_attr[1], &rt_param[1]);
  rc=pthread_create(&threads[1],               // pointer to thread descriptor
                    &rt_sched_attr[1],         // use specific attributes
                    //(void *)0,               // default attributes
                    fib10,                     // thread function entry point
                    (void *)&(threadParams[1]) // parameters to pass in
                    );

  // serviceF20
  rt_param[2].sched_priority=rt_min_prio;
  pthread_attr_setschedparam(&rt_sched_attr[2], &rt_param[2]);
  rc=pthread_create(&threads[2],               // pointer to thread descriptor
                    &rt_sched_attr[2],         // use specific attributes
                    //(void *)0,               // default attributes
                    fib20,                     // thread function entry point
                    (void *)&(threadParams[2]) // parameters to pass in
                    );

   for(i=0;i<3;i++)
       pthread_join(threads[i], NULL);
  
   printf("\nTEST COMPLETE\n");
  
  return 0;
}