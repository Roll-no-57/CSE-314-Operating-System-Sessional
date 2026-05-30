#include<stdio.h>
#include<pthread.h>
#include<semaphore.h>
#include<queue>
#include<unistd.h>

using namespace std;

// This code simulates the famous producer-consumer problem without mutual exclusion 

//semaphore to control sleep and wake up
sem_t sem_empty;
sem_t sem_full;
queue<int> q;


void init_semaphore()
{
	sem_init(&sem_empty,0,5);		// semaphore empty = 5

	sem_init(&sem_full,0,0); 		// semaphore full = 0
}

void * ProducerFunc(void * arg)
{	
	printf("%s\n",(char*)arg);
	int i;
	for(i=1;i<=10;i++)
	{
		sem_wait(&sem_empty);		 // down(&empty)

			
		sleep(1);
		
		q.push(i);
		printf("producer produced item %d\n",i);		// simulate critical region work
		
		
	
		sem_post(&sem_full);		// up(&full)
	}
}

void * ConsumerFunc(void * arg)
{
	printf("%s\n",(char*)arg);
	int i;
	for(i=1;i<=10;i++)
	{	
		sem_wait(&sem_full);		// down(&full)
 		
		sleep(1);
		

		int item = q.front();
		q.pop();
		printf("consumer consumed item %d\n",item);	

			
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
