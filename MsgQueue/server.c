#include"comm.c"

int main()
{
	int msgid = creatMsgQueue();
	char buf[2*MYSIZE];
	while(1)
	{
		if(recvMsg(msgid,CLIENT_TYPE,buf) < 0)
		{
			break;
		}
		else
		{
			printf("client: %s",buf);
		}
		if(sendMsg(msgid,SERVER_TYPE,buf) < 0)
		{
			break;
		}
	}
	destoryMsgQueue(msgid);
	return 0;
}
