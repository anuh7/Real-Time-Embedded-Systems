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

#define UPDATE_RATE_HZ 1
#define READ_RATE_HZ 0.1
#define NUM_SECONDS 180
#define ITERATION_LIMIT             (180)
#define MY_SCHED_POLICY             (SCHED_FIFO)
#define NUM_THREADS                 (2) 
#define COUNT_TO_LOAD               (100000000)     //100ms, 10Hz
#define HZ_1                       (10)             //1s, 1Hz
#define HZ_10                       (100)           //10s, 0.1Hz
#define NUM_CPU_CORES               (1)  


typedef struct
{
    int threadIdx;
} threadParams_t;

// Structure to hold the state data
typedef struct {
    double lat;
    double lon;
    double alt;
    double roll;
    double pitch;
    double yaw;
    struct timespec timestamp;
}state_t;

// Global state structure
state_t state;

// Semaphore declarations

sem_t update_sem;
sem_t read_sem;
bool abortTest = false;
timer_t timer_1;
struct itimerspec last_itime;
struct itimerspec itime = {{1,0}, {1,0}};
unsigned long long seqCnt=0;
unsigned long long int LimitIterationCount = 0;
double time_now;
struct timespec read_timestamp;
double read_time;
pthread_mutex_t lock;

int interation_count;


void Sequencer();
void print_scheduler(void);
void set_sequencer_timer_interval();

void print_scheduler()
{
  int schedType;
  schedType = sched_getscheduler(getpid());
  switch(schedType)
  {
    case SCHED_FIFO:
      syslog(LOG_INFO,"Pthread Policy is SCHED_FIFO\n");
      break;
    case SCHED_OTHER:
      syslog(LOG_INFO,"Pthread Policy is SCHED_OTHER\n"); //exit(-1);
      break;
    case SCHED_RR:
      syslog(LOG_INFO,"Pthread Policy is SCHED_RR\n");// exit(-1);
      break;
    default:
      syslog(LOG_INFO,"Pthread Policy is UNKNOWN\n"); exit(-1);
  }
}



void Sequencer()
{
  int flags=0;
  // received interval timer signal

  if(abortTest)
  {
    // disable interval timer
    itime.it_interval.tv_sec = 0;
    itime.it_interval.tv_nsec = 0;
    itime.it_value.tv_sec = 0;
    itime.it_value.tv_nsec = 0;
    timer_settime(timer_1, flags, &itime, &last_itime);
    printf("Disabling sequencer interval timer with abort=%d and %llu\n", abortTest, seqCnt);
    sem_post(&update_sem);
    sem_post(&read_sem);
  }
  seqCnt++;

  if((seqCnt % HZ_1) == 0) 
  {
    sem_post(&update_sem);
    LimitIterationCount++;
  }

  if((seqCnt % HZ_10) == 0)
  {
   sem_post(&read_sem);
  }

  if(LimitIterationCount == ITERATION_LIMIT)
  {
    abortTest = true;
  }
}


void set_sequencer_timer_interval()
{

  int flags=0;
  /* set up to signal SIGALRM if timer expires */
  timer_create(CLOCK_MONOTONIC, NULL, &timer_1);
  signal(SIGALRM, (void(*)()) Sequencer);

  /* arm the interval timer */
  itime.it_interval.tv_sec = 0;
  itime.it_interval.tv_nsec = COUNT_TO_LOAD;
  itime.it_value.tv_sec = 0;
  itime.it_value.tv_nsec = COUNT_TO_LOAD;
  timer_settime(timer_1, flags, &itime, &last_itime);
}



void *update_state(void *threadp)

{
   while(!abortTest)
  {
    sem_wait(&update_sem); 
    if(abortTest)
      break; 
    pthread_mutex_lock(&lock);
    state.lat = ((double)rand());
    state.lon = ((double)rand());
    state.alt = ((double)rand());
    state.roll = ((double)rand());
    state.pitch = ((double)rand());
    state.yaw = ((double)rand());
    interation_count++;

    if (interation_count%10==0)
    {
    printf("*********************Update thread*************************");
    clock_gettime(CLOCK_MONOTONIC_RAW, &(state.timestamp));
    time_now = (double)(((state.timestamp.tv_sec)*1000.0) + ((state.timestamp.tv_nsec)/1000000.0));
    printf("Time now: %lf\n", time_now);
    printf("Latitude: %lf, Longitude: %lf, Altitude: %lf\n", state.lat, state.lon, state.alt);
    printf("Roll: %lf, Pitch: %lf, Yaw: %lf\n", state.roll, state.pitch, state.yaw);
    }
    pthread_mutex_unlock(&lock);
  }
      pthread_exit((void *)0);

}

void *read_state(void *threadp)
{
   while(!abortTest)
  {
    sem_wait(&read_sem); 

    if(abortTest)
      break; 
    pthread_mutex_lock(&lock);
    printf("*********************Read thread*************************");
    clock_gettime(CLOCK_MONOTONIC_RAW, &(read_timestamp));
    read_time = (double)(((read_timestamp.tv_sec)*1000.0) + ((read_timestamp.tv_nsec)/1000000.0));
    printf("Time now: %lf\n", read_time);
    printf("Latitude: %lf, Longitude: %lf, Altitude: %lf\n", state.lat, state.lon, state.alt);
    printf("Roll: %lf, Pitch: %lf, Yaw: %lf\n", state.roll, state.pitch, state.yaw);
    pthread_mutex_unlock(&lock);
  }
      pthread_exit((void *)0);
}


int main() 
{
  int i, rc;
  cpu_set_t threadcpu;
  pthread_t threads[NUM_THREADS];
  threadParams_t threadParams[NUM_THREADS];
  pthread_attr_t rt_sched_attr[NUM_THREADS];
  int rt_max_prio, rt_min_prio;
  struct sched_param rt_param[NUM_THREADS];
  struct sched_param main_param;
  //pthread_attr_t main_attr;
  pid_t mainpid;
  cpu_set_t allcpuset;
  abortTest=false;
  syslog(LOG_INFO,"System has %d processors configured and %d available.\n", get_nprocs_conf(), get_nprocs());
  CPU_ZERO(&allcpuset);

  pthread_mutex_init(&lock,NULL);

  for(i=0; i < NUM_CPU_CORES; i++)
    CPU_SET(i, &allcpuset);
  syslog(LOG_INFO,"Using CPUS=%d from total available.\n", CPU_COUNT(&allcpuset));
  if (sem_init (&update_sem, 0, 0)) { syslog(LOG_INFO,"Failed to initialize update_sem semaphore\n"); exit (-1); }
  if (sem_init (&read_sem, 0, 0)) { syslog(LOG_INFO,"Failed to initialize read_sem semaphore\n"); exit (-1); }
  mainpid=getpid();
  rc=sched_getparam(mainpid, &main_param);

  rt_max_prio = sched_get_priority_max(MY_SCHED_POLICY);
  rt_min_prio = sched_get_priority_min(MY_SCHED_POLICY);
  main_param.sched_priority=rt_max_prio;
  rc=sched_setscheduler(getpid(), MY_SCHED_POLICY, &main_param);
  if(rc < 0) perror("main_param");
  print_scheduler();
  
  for(i=0; i < NUM_THREADS; i++)
  {
    CPU_ZERO(&threadcpu);
    CPU_SET(3, &threadcpu);
    rc=pthread_attr_init(&rt_sched_attr[i]);
    rc=pthread_attr_setinheritsched(&rt_sched_attr[i], PTHREAD_EXPLICIT_SCHED);
    rc=pthread_attr_setschedpolicy(&rt_sched_attr[i], MY_SCHED_POLICY);
    rc=pthread_attr_setaffinity_np(&rt_sched_attr[i], sizeof(cpu_set_t), &threadcpu);
  }
    print_scheduler();
    sem_init(&update_sem, 0, 0);
    sem_init(&read_sem, 0, 0);
   syslog(LOG_INFO,"Service threads will run on %d CPU cores\n", CPU_COUNT(&threadcpu));

  rt_param[0].sched_priority=rt_max_prio-1;
  pthread_attr_setschedparam(&rt_sched_attr[0], &rt_param[0]);
  rc=pthread_create(&threads[0],               // pointer to thread descriptor
                    &rt_sched_attr[0],         // use specific attributes
                    //(void *)0,               // default attributes
                    update_state,                     // thread function entry point
                    (void *)&(threadParams[0]) // parameters to pass in
                    );


  rt_param[1].sched_priority=rt_max_prio-2;
  pthread_attr_setschedparam(&rt_sched_attr[1], &rt_param[1]);
  rc=pthread_create(&threads[1],               // pointer to thread descriptor
                  &rt_sched_attr[1],         // use specific attributes
                    //(void *)0,               // default attributes
                    read_state,                     // thread function entry point
                    (void *)&(threadParams[1]) // parameters to pass in
                    );

  
  set_sequencer_timer_interval();
 for(i=0;i<NUM_THREADS;i++)
       pthread_join(threads[i], NULL);
     printf("\nTEST COMPLETE\n");


    // Destroy semaphores
   sem_destroy(&update_sem);
   sem_destroy(&read_sem);

  pthread_mutex_destroy(&lock);
    return 0;
}



