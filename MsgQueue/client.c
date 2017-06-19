#include"comm.c"

int main()
{
	int msgid = getMsgQueue();//创建一个新的消息队列
	char out[2*MYSIZE];
	char buf[MYSIZE];
	while(1)
	{
		printf("please enter:");
		fflush(stdout);
		ssize_t s = read(0,buf,sizeof(buf)-1);
		if(s > 0)
		{
			buf[s] = '\0';
			sendMsg(msgid,CLIENT_TYPE,buf);
		}
		if(recvMsg(msgid,SERVER_TYPE,out) < 0)//从消息队列中读取消息
		{
			break;
		}
		printf("server: %s",out);
	}
	return 0;
}
