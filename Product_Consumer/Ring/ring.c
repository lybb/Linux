#include<stdio.h>
#include<pthread.h>
#include<semaphore.h>
#include<unistd.h>

#define SIZE 20
int ringbuf[SIZE];

sem_t blocks;
sem_t datas;

pthread_mutex_t mylock = PTHREAD_MUTEX_INITIALIZER;

void* pro_run(void* arg)
{
	int i=0;
	while(1)
	{
		sleep(1);
		int ret = sem_wait(&blocks);
		if(ret == 0)
		{
			pthread_mutex_lock(&mylock);
			int ret = rand()%1231;
			printf("pro-tid:%lu,product is:%d \n",pthread_self(),ret);
			ringbuf[i] = ret;
			i %= SIZE;
			sem_post(&datas);
			pthread_mutex_unlock(&mylock);
		}
	}
}
void* con_run(void* arg)
{
	int i=0;
	while(1)
	{
		sleep(1);
		int ret = sem_wait(&datas);
		int sig = i;
		if(ret == 0)
		{
			i = sig;
			pthread_mutex_lock(&mylock);
			int data = ringbuf[i];
			printf("con-tid:%lu,consumer is:%d \n",pthread_self(),data);
			i %= SIZE;
			sem_post(&blocks);
			pthread_mutex_unlock(&mylock);
		}
	}
}


int main()
{
	pthread_mutex_init(&mylock,NULL);

	sem_init(&blocks,0,SIZE);
	sem_init(&datas,0,0);

	pthread_t pro1,pro2;
	pthread_t con1,con2;
	pthread_create(&pro1,NULL,pro_run,NULL);
	pthread_create(&pro2,NULL,pro_run,NULL);
	pthread_create(&con1,NULL,con_run,NULL);
	pthread_create(&con2,NULL,con_run,NULL);

	pthread_join(pro1,NULL);
	pthread_join(pro2,NULL);
	pthread_join(con1,NULL);
	pthread_join(con2,NULL);

	sem_destroy(&blocks);
	sem_destroy(&datas);

	pthread_mutex_destroy(&mylock);

	return 0;
}
