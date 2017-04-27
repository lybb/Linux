#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdlib.h>


int main(int argc,char* argv[])
{
	if(argc != 3)
	{
		printf("Usage:%s [local_ip] [local_port]",argv[0]);
		return 1;
	}

	//将type通信类型改为SOCK_DGRAM
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if(sock < 0)
	{
		perror("socket");
		return 2;
	}

	struct sockaddr_in peer;
	peer.sin_family = AF_INET;
	peer.sin_port = htons(atoi(argv[2]));
	peer.sin_addr.s_addr = inet_addr(argv[1]);
	socklen_t len = sizeof(peer);

	if(bind(sock, (struct sockaddr*)&peer, sizeof(peer)) < 0)
	{
		perror("bind");
		return 3;
	}

	char buf[1024];
	while(1)
	{
		ssize_t r = recvfrom(sock, buf,sizeof(buf),0,(struct sockaddr*)&peer,&len);
		if(r < 0)
		{
			perror("recvfrom");
			continue;
		}
		else if(r == 0)
		{
			printf("client is quit!\n");
		}
		else
		{
			printf("recv a msg,is:%s\n",buf);
		}
	}
	close(sock);

	return 0;
}
