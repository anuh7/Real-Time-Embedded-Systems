// For the heap message queue, take this message copy version and create a global heap and queue and dequeue pointers to that
// heap instead of the full data in the messages.
//
// Otherwise, the code should not be substantially different.
//
// You can set up an array in the data segement as a global or use malloc and free for a true heap message queue.
//
// Either way, the queue and dequeue should be ZERO copy.
//
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

// On Linux the file systems slash is needed
#define SNDRCV_MQ "/send_receive_mq"

#define MAX_MSG_SIZE 128
#define ERROR (-1)

struct mq_attr mq_attr;
static mqd_t mymq;

pthread_t th_receive, th_send; // create threads
pthread_attr_t attr_receive, attr_send;
struct sched_param param_receive, param_send;

static char canned_msg[] = "This is a test, and only a test, in the event of real emergency, you would be instructed...."; // Message to be sent

/* receives pointer to heap, reads it, and deallocate heap memory */

void *receiver(void *arg)
{
  char buffer[sizeof(void *)];
  void *ptr;
  int prio;
  int rc;
 
   printf("Receiver - thread entry \n");

    while (1)
    {
        if((rc = mq_receive(mymq, buffer, (size_t)(sizeof(void *)),&prio)) == ERROR)
        {
          perror("mq_receive");      
          printf("receive: no success");
        } 
        else
        {
          memcpy(&ptr, buffer, sizeof(void *));
          printf("receive: msg %s received with priority = %d, rc = %d\n", (char *)ptr, prio, rc);
          free(ptr);
        }

    }  
  
}


void *sender(void *arg)
{
  char buffer[sizeof(void *)];
   int prio;
   int rc;
   void *ptr;

  printf("sender - thread entry\n");

  while (1)
  {
    ptr = (void *)malloc(sizeof(canned_msg));
    strcpy(ptr, canned_msg);
    printf("sender - sending message of size=%d\n", sizeof(canned_msg));
    printf("message sent: %s \n", (char *)ptr);

    memcpy(buffer, &ptr, sizeof(void *));

    if((rc = mq_send(mymq, buffer, (size_t)(sizeof(void *)),30)) == ERROR)
    {
      perror("mq_send");
      printf("send: no success");
    }
    else
    {
       printf("send: message successfully sent, 0x%p\n", ptr);
    }
  sleep(3);
  }  
}


void main(void)
{
  int i=0, rc=0;
  /* setup common message q attributes */
  mq_attr.mq_maxmsg = 10;
  mq_attr.mq_msgsize = sizeof(void *);
  mq_attr.mq_flags = 0;

  int rt_max_prio, rt_min_prio;

  rt_max_prio = sched_get_priority_max(SCHED_FIFO);
  rt_min_prio = sched_get_priority_min(SCHED_FIFO);


  rc = pthread_attr_init(&attr_receive);
  rc = pthread_attr_setinheritsched(&attr_receive, PTHREAD_EXPLICIT_SCHED);
  rc = pthread_attr_setschedpolicy(&attr_receive, SCHED_FIFO); 
  param_receive.sched_priority = rt_min_prio;
  pthread_attr_setschedparam(&attr_receive, &param_receive);
  

  rc = pthread_attr_init(&attr_send);
  rc = pthread_attr_setinheritsched(&attr_send, PTHREAD_EXPLICIT_SCHED);
  rc = pthread_attr_setschedpolicy(&attr_send, SCHED_FIFO); 
  param_send.sched_priority = rt_max_prio;
  pthread_attr_setschedparam(&attr_send, &param_send);

   mymq = mq_open(SNDRCV_MQ, (O_CREAT|O_RDWR), 0664, &mq_attr); 

	if(mymq == (mqd_t)ERROR)
	{
		perror("mq_open");
	}
  
  if((rc=pthread_create(&th_send, &attr_send, sender, NULL)) == 0)
  {
    printf("\n\rSender Thread Created with rc=%d\n\r", rc);
  }
  else 
  {
    perror("\n\rFailed to Make Sender Thread\n\r");
    printf("rc=%d\n", rc);
  }

  if((rc=pthread_create(&th_receive, &attr_receive, receiver, NULL)) == 0)
  {
    printf("\n\r Receiver Thread Created with rc=%d\n\r", rc);
  }
  else
  {
    perror("\n\r Failed Making Reciever Thread\n\r"); 
    printf("rc=%d\n", rc);
  }

  printf("pthread join send\n");  
  pthread_join(th_send, NULL);

  printf("pthread join receive\n");  
  pthread_join(th_receive, NULL);

  	mq_close(mymq);

}
