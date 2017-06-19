#include"comm.h"

static int commMsgQueue(int flags)
{
	key_t k = ftok(PATHNAME,PROJID);
	if(k < 0)
	{
		perror("ftok");
		return -1;
	}

	int msgid = msgget(k,flags);
	if(msgid < 0)
	{
		perror("msgget");
		return -2;
	}
	return msgid;
}

int creatMsgQueue()
{
	return commMsgQueue(IPC_CREAT | IPC_EXCL | 0666);
}

int getMsgQueue()
{
	return commMsgQueue(IPC_CREAT);
}

int sendMsg(int msgid,long type,const char* msg)
{
	struct msgbuf buf;
	buf.mtype = type;
	strcpy(buf.mtext,msg);
	int ret = msgsnd(msgid,&buf,sizeof(buf.mtext),0);
	if(ret < 0)
	{
		perror("msgsnd");
		return -1;
	}
	return 0;
}

int recvMsg(int msgid,int type,char out[])
{
	struct msgbuf buf;
	int ret = msgrcv(msgid,&buf,sizeof(buf.mtext),type,0);
	if(ret > 0)
	{
	//	buf.mtext[ret] = '\0';
		strncpy(out, buf.mtext, ret);
		return 0;
	}
	perror("msgrcv");
	return -1;
}

int destoryMsgQueue(int msg_id)
{
	if(msgctl(msg_id,IPC_RMID,NULL)<0)
	{
		perror("mshctl");
		return -1;
	}
	return 0;
}
