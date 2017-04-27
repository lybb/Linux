#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdlib.h>

int main(int argc,char* argv[])
{
	if(argc != 3)
	{
		printf("Usage:%s [ip] [port]",argv[0]);
		return 1;
	}

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

	char buf[1024];
	while(1)
	{
		printf("Please Enter:");
		fflush(stdout);
		ssize_t r = read(0, buf, sizeof(buf)-1);
		if(r < 0)
		{
			perror("read");
			continue;
		}
		buf[r-1] = 0;

		sendto(sock,buf,sizeof(buf)-1,0,(struct sockaddr*)&peer,len);
	}
	close(sock);

	return 0;
}
