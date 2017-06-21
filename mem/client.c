#include"comm.c"
#include<unistd.h>

int main()
{
	int shmid = getShm();
	char* mem = (char*)shmat(shmid, NULL, 0);
	int i=0;
	while(1)
	{
		printf("%s\n",mem);
		fflush(stdout);
		sleep(1);
	}
	shmdt(mem);
	return 0;
}
