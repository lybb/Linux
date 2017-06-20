#ifndef _COMM_
#define _COMM_

#include<unistd.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/sem.h>

#define PATHNAME "."
#define PROJID 0x6666

union semun {
	int val;    /* Value for SETVAL */
	struct semid_ds *buf;/* Buffer for IPC_STAT,IPC_SET */     unsigned short  *array;/* Array for GETALL, SETALL */
	struct seminfo  *__buf;/* Buffer for IPC_INFO
							  (Linux-specific) */
};


int creatSemSet(int num);
int initSem(int semid,int which);
int getSemSet();
int P(int semid,int which);
int V(int semid,int which);
int destorySemSet(int semid);

#endif

