#include<stdio.h>
#include<pthread.h>
#include<string.h>
#include<unistd.h>

pthread_rwlock_t mylock;

int g_val = 0;

void* reader(void *arg)
{
	while(1)
	{
		sleep(2);
		int ret = pthread_rwlock_rdlock(&mylock);
		if(ret == 0)
		{
			printf("reader:%d, %s, g_val:%d\n",ret,strerror(ret),g_val);
			pthread_rwlock_unlock(&mylock);
		}
	}
}

void* writer(void* arg)
{
	while(1)
	{
		sleep(1);
		int ret = pthread_rwlock_wrlock(&mylock);
		if(ret == 0)
		{
			g_val++;
			printf("writer:%d, %s, g_val:%d\n",ret,strerror(ret),g_val);
		}
		pthread_rwlock_unlock(&mylock);
	}
}

int main()
{
	pthread_rwlock_init(&mylock,NULL);

	pthread_t read;
	pthread_t write;
	pthread_create(&read,NULL,reader, NULL);
	pthread_create(&write,NULL,writer, NULL);

	pthread_join(read,NULL);
	pthread_join(write,NULL);

	pthread_rwlock_destroy(&mylock);
	return 0;
}
