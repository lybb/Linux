#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<arpa/inet.h>

int main(int argc,const char* argv[])
{
	if(argc != 3)
	{
		printf("Usage:%s [ip] [port]\n",argv[0]);
		return 0;
	}

	int sock = socket(AF_INET,SOCK_STREAM, 0);
	if(sock < 0)
	{
		perror("socket");
		return 1;
	}

	struct sockaddr_in server;
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(argv[2]));
	server.sin_addr.s_addr = inet_addr(argv[1]);

	socklen_t len = sizeof(struct sockaddr_in);

	if(connect(sock, (struct sockaddr*)&server, len) < 0 )
	{
		perror("connect");
		return 2;
	}

	char buf[1024];
	while(1)
	{
		printf("send###");
		fflush(stdout);

		ssize_t _s = read(0, buf, sizeof(buf)-1);
		buf[_s] = 0;
		write(sock, buf, _s);
	}
	close(sock);

	return 0;
}
