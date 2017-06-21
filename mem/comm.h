#ifndef _COMM_
#define _COMM_

#include<stdio.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>

#define PATHNAME "."
#define PROJID 0x6666
#define SIZE 4096*1

int creatShm();
int getShm();
int Shmat();
int Shmdt();
int destoryShm(int shmid);


#endif
