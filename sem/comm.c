#include"comm.h"

static int commSemSet(int nums,int flags)
{
	key_t k = ftok(PATHNAME,PROJID);
	int semid = semget(k,nums,flags);
	if(semid < 0)
	{
		perror("semget");
		return -1;
	}
	return semid;
}

int creatSemSet(int nums)
{
	return commSemSet(nums,IPC_CREAT | IPC_EXCL | 0666);
}
int getSemSet()
{
	return commSemSet(0,0);
}

int initSem(int semid,int which)
{
	union semun u;
	u.val = 1;
	int ret = semctl(semid,which,SETVAL,u);
	if(ret < 0)
	{
		perror("semctl");
		return -1;
	}
	return 0;
}

static int semOP(int semid,int which,int op)
{
	struct sembuf s;
	s.sem_num = which;
	s.sem_op = op;
	s.sem_flg = 0;
	int ret = semop(semid, &s, 1); 
	if(ret < 0)
	{
		perror("semOP");
		return -1;
	}
	return 0;
}
int P(int semid,int which)
{
	return semOP(semid,which,-1);
}
int V(int semid,int which)
{
	return semOP(semid,which,1);
}
int destorySemSet(int semid)
{
	int ret = semctl(semid,0,IPC_RMID);
	if(ret < 0)
	{
		perror("semctl");
		return -1;
	}
	return 0;
}
