#include"comm.h"

static int commShm(int flgs)
{
	key_t k = ftok(PATHNAME,PROJID);
	if(k < 0)
	{
		perror("ftok");
		return -1;
	}

	int shmid = shmget(k, SIZE, flgs);
	if(shmid < 0)
	{
		perror("shmget");
		return -1;
	}
	return shmid;
}

int creatShm()
{
	return commShm(IPC_CREAT | IPC_EXCL | 0666);
}
int getShm()
{
	return commShm(IPC_CREAT);
}
int Shmat();
int Shmdt();
int destoryShm(int shmid)
{
	int ret = shmctl(shmid, IPC_RMID, NULL);
	if(ret < 0)
	{
		perror("shmctl");
		return -1;
	}
	return 0;
}
