#include"comm.c"
#include<unistd.h>

int main()
{
	int shmid = creatShm();
	char* mem = (char*)shmat(shmid, NULL, 0);

	int i = 0;
	while(1)
	{
		sleep(1);
		mem[i++] = 'A';
		i %= (SIZE-1);
		mem[i] = '\0';
	}
	shmdt(mem);
	destoryShm(shmid);
	return 0;
}
