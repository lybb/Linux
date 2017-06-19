#ifndef _COMM_
#define _COMM_

#include<unistd.h>
#include<stdio.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/msg.h>
#include<string.h>

#define PATHNAME "."
#define PROJID 0x6666
#define MYSIZE 128

#define SERVER_TYPE 1
#define CLIENT_TYPE 2

struct msgbuf{
	long mtype;
	char mtext[MYSIZE];
};

int creatMsgQueue();
int getMsgQueue();
int sendMsg(int msgid,long type,const char* msg);
int recvMsg(int msgid,int type,char out[]);
int destoryMsgQueue(int msgid);

#endif

