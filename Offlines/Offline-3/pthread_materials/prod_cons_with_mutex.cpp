#include<stdio.h>
#include<pthread.h>
#include<semaphore.h>
#include<queue>
#include<unistd.h>
#include<cstdlib>
using namespace std;


// This code simulates the famous producer-consumer problem with mutual exclusion and busy wait solve

//semaphore to control sleep and wake up
sem_t sem_empty;
sem_t sem_full;
queue<int> q;
pthread_mutex_t lock;


void init_semaphore()
{
	sem_init(&sem_empty,0,5);		// this is similar like semaphore empty = 5
	sem_init(&sem_full,0,0);		// this is semaphore full = 0
	pthread_mutex_init(&lock,0);	// this is semaphore mutex = 0 
}

void * ProducerFunc(void * arg)
{	
	printf("%s\n",(char*)arg);
	int i;
	for(i=1;i<=10;i++)
	{
		sem_wait(&sem_empty);			// down(&empty)

		pthread_mutex_lock(&lock);		// down(&mutex)

		// adding random sleep to simulate critical region work
		int random_sleep = rand() % 3 + 1; // sleep between 1 to 3 seconds
		sleep(random_sleep);
		q.push(i);
		printf("producer produced item %d\n",i);		// simulate critical region work
		
		pthread_mutex_unlock(&lock);	// up(&mutex)
	
		sem_post(&sem_full);			// up(&full)
	}
}

void * ConsumerFunc(void * arg)
{
	printf("%s\n",(char*)arg);
	int i;
	for(i=1;i<=10;i++)
	{	
		sem_wait(&sem_full);			// up(&full)
 		
		pthread_mutex_lock(&lock);		// down(&mutex)
			
		sleep(1);
		int item = q.front();
		q.pop();
		printf("consumer consumed item %d\n",item);		// simulate critical region work

		pthread_mutex_unlock(&lock);	// up(&mutex)
		
		sem_post(&sem_empty);			// up(&empty)
	}
}





int main(void)
{	
	pthread_t thread1;
	pthread_t thread2;
	
	init_semaphore();
	
	char * message1 = "i am producer";
	char * message2 = "i am consumer";	
	
	pthread_create(&thread1,NULL,ProducerFunc,(void*)message1 );
	pthread_create(&thread2,NULL,ConsumerFunc,(void*)message2 );

	
	while(1);
	return 0;
}
