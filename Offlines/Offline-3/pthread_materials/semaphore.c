#include<stdio.h>
#include<pthread.h>
#include<semaphore.h>
#include<unistd.h>

// Difference between sem_t (Semaphore) and pthread_mutex_t (Mutex):
/*
------------------------------------------------------------------------
| Feature         | sem_t (Semaphore)         | pthread_mutex_t (Mutex) |
---------------------------------------------------------------
| Purpose         | Synchronization, can      | Mutual exclusion only    |
|                 | allow multiple threads    | (only 1 thread at a time)|
|                 | (counting or binary)      |                         |
-------------------------------------------------------------------------
| Value           | Integer (>=0), can be     | Binary (locked/unlocked) |
|                 | >1 (counting) or 0/1      |                         |
-------------------------------------------------------------------------
| Ownership       | No ownership; any thread  | Must be unlocked by      |
|                 | can wait/post             | thread that locked it    |
-------------------------------------------------------------------------
| Use Case        | Resource counting,        | Protect critical section |
|                 | signaling, producer-      | (mutex lock/unlock)      |
|                 | consumer, etc.            |                         |
-------------------------------------------------------------------------
| Blocking        | Blocks if value is 0      | Blocks if already locked |
| Types           | Binary or counting        | Only binary              |
---------------------------------------------------------------
*/



sem_t bin_sem;						
pthread_mutex_t mtx;
char message[100];



void * thread_function(void * arg)
{	
	int x;
	char message2[10];
	while(1)
	{	
		printf("thread2:waiting..\n");
		//pthread_mutex_lock(&mtx);
		sem_wait(&bin_sem);		
		printf("hi i am the new thread waiting inside critical..\n");
		scanf("%s",message);
		printf("You entered:%s\n",message);
		sem_post(&bin_sem);
		//pthread_mutex_unlock(&mtx);
	
	}
	
}

int main(void)
{
	pthread_t athread;
	pthread_attr_t ta;
	char message2[10];
	int x;
	sem_init(&bin_sem,0,1);
	pthread_mutex_init(&mtx,NULL);
	
	pthread_attr_init(&ta);
	pthread_attr_setschedpolicy(&ta,SCHED_RR);	                                                                                                                                                                                                     

	pthread_create(&athread,&ta,thread_function,NULL);
	while(1)
	{	
		//pthread_mutex_lock(&mtx);
		printf("main waiting..\n");
		sem_wait(&bin_sem);	
		printf("hi i am the main thread waiting inside critical..\n");
		scanf("%s",message);
		printf("You entered:%s\n",message);
		sem_post(&bin_sem);
		//pthread_mutex_unlock(&mtx);
	}
	sleep(5);		
}
