/****************************************************************************/
/* Function: nanosleep and POSIX 1003.1b RT clock demonstration             */
/*                                                                          */
/* Sam Siewert - 02/05/2011                                                 */
/*                                                                          */
/****************************************************************************/

#include <pthread.h> /* It consists of the details and functions associated with threads */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#define NSEC_PER_SEC (1000000000)
#define NSEC_PER_MSEC (1000000)
#define NSEC_PER_USEC (1000)
#define ERROR (-1)
#define OK (0)
#define TEST_SECONDS (0)

/* nanosleep function is tested for 10ms */
#define TEST_NANOSECONDS (NSEC_PER_MSEC * 10)     

/*******************************************************************************
Function Prototypes
*******************************************************************************/
void end_delay_test(void);

/*******************************************************************************
Global Variables
*******************************************************************************/
static struct timespec sleep_time = {0, 0};
static struct timespec sleep_requested = {0, 0};
static struct timespec remaining_time = {0, 0};

static unsigned int sleep_count = 0;

pthread_t main_thread;                //stores the identifier of the thread
pthread_attr_t main_sched_attr;       //thread attributed object used to define the attributes for thread creation 
int rt_max_prio, rt_min_prio, min;
struct sched_param main_param;        //used to specify the scheduling parameters for a thread


/*******************************************************************************
* @Function Name: print_scheduler
* @Description: The function is prints the scheduling policy of the current process running
* @input param: None 
* @return: None
*******************************************************************************/
void print_scheduler(void)
{
   int schedType;

/* getpid() returns the process ID(PID) of the calling process
  sched_getscheduler returns the scheduling policy of that process */
   schedType = sched_getscheduler(getpid());    

   switch(schedType)
   {
     case SCHED_FIFO:
           printf("Pthread Policy is SCHED_FIFO\n");
           break;
     case SCHED_OTHER:
           printf("Pthread Policy is SCHED_OTHER\n");
       break;
     case SCHED_RR:
           printf("Pthread Policy is SCHED_RR\n");
           break;
     default:
       printf("Pthread Policy is UNKNOWN\n");
   }
}

/*******************************************************************************
* @Function Name: d_ftime
* @Description: The function calculates the difference between two timespec structures
* @input param: fstart   pointer to timespec structure to hold code execution start time stamp
* @input param: fstop    pointer to timespec structure to hold code execution end time stamp
* @return: double   returns the difference in start and stop time stamps 
*******************************************************************************/
double d_ftime(struct timespec *fstart, struct timespec *fstop)
{
  double dfstart = ((double)(fstart->tv_sec) + ((double)(fstart->tv_nsec) / 1000000000.0));
  double dfstop = ((double)(fstop->tv_sec) + ((double)(fstop->tv_nsec) / 1000000000.0));

  return(dfstop - dfstart); 
}


/*******************************************************************************
* @Function Name: delta_t
* @Description: The function calculates the difference between two timespec structures taking into account any potential roll-over
* @input param: start   pointer to timespec structure to hold code execution start time stamp
* @input param: stop    pointer to timespec structure to hold code execution end time stamp
* @return: struct timespec   returns the difference in start and stop time stamps 
*******************************************************************************/
int delta_t(struct timespec *stop, struct timespec *start, struct timespec *delta_t)
{

  /* calculating time difference */
  int dt_sec=stop->tv_sec - start->tv_sec;
  int dt_nsec=stop->tv_nsec - start->tv_nsec;

  /* case 1 - less than a second of change */
  if(dt_sec == 0)
  {
	  /* dt less than 1 second */

	  if(dt_nsec >= 0 && dt_nsec < NSEC_PER_SEC)
	  {
	          /* nanoseconds are greater at stop time stamp than start time stamp */
		  delta_t->tv_sec = 0;
		  delta_t->tv_nsec = dt_nsec;
	  }

	  else if(dt_nsec > NSEC_PER_SEC)
	  {
	          /* nanosecond overflow */
		  delta_t->tv_sec = 1;
		  delta_t->tv_nsec = dt_nsec-NSEC_PER_SEC;
	  }

    /* error condition: stop timestamp is earlier than start time stamp */
	  else 
	  {
	         printf("stop is earlier than start\n");
		 return(ERROR);  
	  }
  }

  /* case 2 - more than a second of change, checks for roll-over */
  else if(dt_sec > 0)
  {
    /* difference in seconds is more than 1 second */

	  if(dt_nsec >= 0 && dt_nsec < NSEC_PER_SEC)
	  {
	          /* nanoseconds are greater at stop time stamp than start time stamp */
		  delta_t->tv_sec = dt_sec;
		  delta_t->tv_nsec = dt_nsec;
	  }

	  else if(dt_nsec > NSEC_PER_SEC)
	  {
	          /* nanosecond overflow */
		  delta_t->tv_sec = delta_t->tv_sec + 1;
		  delta_t->tv_nsec = dt_nsec-NSEC_PER_SEC;
	  }

    // roll over condition: dt_sec<0
	  else 
	  {
	          /* nanosecond rollover */
		  delta_t->tv_sec = dt_sec-1;
		  delta_t->tv_nsec = NSEC_PER_SEC + dt_nsec;
	  }
  }

  return(OK);
}

static struct timespec rtclk_dt = {0, 0};
static struct timespec rtclk_start_time = {0, 0};
static struct timespec rtclk_stop_time = {0, 0};
static struct timespec delay_error = {0, 0};

//#define MY_CLOCK CLOCK_REALTIME
//#define MY_CLOCK CLOCK_MONOTONIC
#define MY_CLOCK CLOCK_MONOTONIC_RAW
//#define MY_CLOCK CLOCK_REALTIME_COARSE
//#define MY_CLOCK CLOCK_MONOTONIC_COARSE

#define TEST_ITERATIONS (100)       //nanosleep function ie. delay is test for 100 times

/*******************************************************************************
* @Function Name: delay_test
* @Description: The function performs the requested sleep for given number of test iterations
* @input param: threadID  pointer to the thread ID
* @return: none
*******************************************************************************/
void *delay_test(void *threadID)
{
  int idx, rc;
  unsigned int max_sleep_calls=3;
  int flags = 0;
  struct timespec rtclk_resolution;

  sleep_count = 0;

/* clock_getres() retrieves the resolution of the specified clock
  returns (-1) on error */
  if(clock_getres(MY_CLOCK, &rtclk_resolution) == ERROR)
  {
      perror("clock_getres");
      exit(-1);
  }
  else
  {
      printf("\n\nPOSIX Clock demo using system RT clock with resolution:\n\t%ld secs, %ld microsecs, %ld nanosecs\n", rtclk_resolution.tv_sec, (rtclk_resolution.tv_nsec/1000), rtclk_resolution.tv_nsec);
  }

  for(idx=0; idx < TEST_ITERATIONS; idx++)
  {
      printf("test %d\n", idx);

      /* run test for defined seconds */
      sleep_time.tv_sec=TEST_SECONDS;
      sleep_time.tv_nsec=TEST_NANOSECONDS;

      /*sleep_requested keeps track of how much delay has been completed*/
      sleep_requested.tv_sec=sleep_time.tv_sec;
      sleep_requested.tv_nsec=sleep_time.tv_nsec;

      /* start time stamp */ 
      clock_gettime(MY_CLOCK, &rtclk_start_time);

      /* request sleep time and repeat if time remains */
      do 
      {
          /* nanosleep suspends the execution of thread for given duration and
            stores the remaining delay time if sleep is interrupted */
          if(rc=nanosleep(&sleep_time, &remaining_time) == 0) break;
         
          sleep_time.tv_sec = remaining_time.tv_sec;
          sleep_time.tv_nsec = remaining_time.tv_nsec;

          /*nanosleep can only be interrupted for 3 times*/
          sleep_count++;
      } 
      while (((remaining_time.tv_sec > 0) || (remaining_time.tv_nsec > 0))
		      && (sleep_count < max_sleep_calls));

      /* stop time stamp*/
      clock_gettime(MY_CLOCK, &rtclk_stop_time);

      /*difference of start and stop timestamps = rtclk_dt*/
      delta_t(&rtclk_stop_time, &rtclk_start_time, &rtclk_dt);

      /*difference in sleep requested and real time clock difference = delay error*/
      delta_t(&rtclk_dt, &sleep_requested, &delay_error);

      /*prints the results of the sleep operation */
      end_delay_test();
  }

}

/*******************************************************************************
* @Function Name: end_delay_test
* @Description: prints the results of the sleep operation: the actual sleep time and delay error.
* @input param: none
* @return: none
*******************************************************************************/
void end_delay_test(void)
{
    double real_dt;
#if 0
  printf("MY_CLOCK start seconds = %ld, nanoseconds = %ld\n", 
         rtclk_start_time.tv_sec, rtclk_start_time.tv_nsec);
  
  printf("MY_CLOCK clock stop seconds = %ld, nanoseconds = %ld\n", 
         rtclk_stop_time.tv_sec, rtclk_stop_time.tv_nsec);
#endif

  real_dt=d_ftime(&rtclk_start_time, &rtclk_stop_time);
  printf("MY_CLOCK clock DT seconds = %ld, msec=%ld, usec=%ld, nsec=%ld, sec=%6.9lf\n", 
         rtclk_dt.tv_sec, rtclk_dt.tv_nsec/1000000, rtclk_dt.tv_nsec/1000, rtclk_dt.tv_nsec, real_dt);

#if 0
  printf("Requested sleep seconds = %ld, nanoseconds = %ld\n", 
         sleep_requested.tv_sec, sleep_requested.tv_nsec);

  printf("\n");
  printf("Sleep loop count = %ld\n", sleep_count);
#endif
  printf("MY_CLOCK delay error = %ld, nanoseconds = %ld\n", 
         delay_error.tv_sec, delay_error.tv_nsec);
}

#define RUN_RT_THREAD

/*******************************************************************************
* @Function Name: main
* @Description: The function is the main application starting point which sets
* the scheduling policy and priority of the threads and creates the thread
* @input param: None
* @return: success/failure
*******************************************************************************/
void main(void)
{
   int rc, scope;

   printf("Before adjustments to scheduling policy:\n");
   print_scheduler();

#ifdef RUN_RT_THREAD

  /* initializing thread scheduling attributed */
   pthread_attr_init(&main_sched_attr);

  /* indicates the created thread to use scheduling attributes set in 
  the attribute object - main_sched_attr */
   pthread_attr_setinheritsched(&main_sched_attr, PTHREAD_EXPLICIT_SCHED);

   /* SCHED_FIFO is policy for real-time processes */
   pthread_attr_setschedpolicy(&main_sched_attr, SCHED_FIFO);

   /* maximum and minimum priorities of the policy, SCHED_FIFO are received */
   rt_max_prio = sched_get_priority_max(SCHED_FIFO);
   rt_min_prio = sched_get_priority_min(SCHED_FIFO);

  /* process priority is assigned as the highest priority */
   main_param.sched_priority = rt_max_prio;

  /*the current process PID is assigned the scheduling policy and priority */
   rc=sched_setscheduler(getpid(), SCHED_FIFO, &main_param);

  /*error condition*/
   if (rc)
   {
       printf("ERROR; sched_setscheduler rc is %d\n", rc);
       perror("sched_setschduler"); exit(-1);
   }

   printf("After adjustments to scheduling policy:\n");
   
   /*Checks the scheduling policy of the current process*/
   print_scheduler();

  // main_param.sched_priority = rt_max_prio;

  /* The thread attributes are passed from the process so a newly created thread
  with the main_sched_attr inherits the scheduling parameters*/
   pthread_attr_setschedparam(&main_sched_attr, &main_param);

   rc = pthread_create(&main_thread,    // pointer to thread descriptor
                       &main_sched_attr,  // use specific attributes
                       delay_test,        // thread function entry point
                       (void *)0);        // parameters to pass in

   if (rc)
   {
       printf("ERROR; pthread_create() rc is %d\n", rc);
       perror("pthread_create");
       exit(-1);
   }

  /* wait for main thread to complete*/
   pthread_join(main_thread, NULL);

  /* destroy thr scheduling attributes */
   if(pthread_attr_destroy(&main_sched_attr) != 0)
     perror("attr destroy");
#else

  /*execute delay_test directly if RUN_RT_THREAD is not defined*/
   delay_test((void *)0);
#endif

   printf("TEST COMPLETE\n");
}

