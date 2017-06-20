#include<unistd.h>
#include<sys/wait.h>
#include"comm.h"

int main()
{
	int semid = creatSemSet(1);
	initSem(semid,0);
	pid_t id = fork();
	if(id == 0)//child
	{
		int semid = getSemSet();
		printf("child is running.  pid:%d, ppid%d\n",getpid(),getppid());
		while(1)
		{
		    P(semid,0);
			printf("A");
			usleep(10040);
			fflush(stdout);
			printf("A");
			usleep(1000);
			fflush(stdout);
			V(semid,0);
		}
	}
	else
	{
		printf("father is running.  pid:%d, ppid%d\n",getpid(),getppid());
		while(1)
		{
		    P(semid,0);
			printf("B");
			usleep(10003);
			fflush(stdout);
			printf("B");
			usleep(1000);
			fflush(stdout);
			V(semid,0);
		}
		wait(NULL);
	}
	destorySemSet(semid);
	return 0;
}
