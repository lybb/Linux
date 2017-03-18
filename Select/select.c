#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/times.h>
#include<sys/select.h>
#include<unistd.h>

#define SIZE 1024

int gfds[SIZE];

int startup(int port, const char* ip)
{
	int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(listen_sock < 0)
	{
		perror("socket");
		exit(2);
	}

	int opt = 1;
	setsockopt(listen_sock,SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	
	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	local.sin_addr.s_addr = inet_addr(ip);
	socklen_t len = sizeof(local);

	if(bind(listen_sock, (struct sockaddr*)&local, len) < 0)
	{
		perror("bind");
		exit(3);
	}

	if(listen(listen_sock, 5) < 0)
	{
		perror("listen");
		exit(4);
	}

	return listen_sock;
}


void Usage(const char* proc)
{
	printf("%s, [ip], [port]\n",proc);
}

int main(int argc, const char* argv[])
{
	if(argc != 3)
	{
		Usage(argv[0]);
		return 1;
	}
	int listen_sock = startup(atoi(argv[2]), argv[1]);
	printf("listen_sock:%d\n",listen_sock);

	int i=0;
	for(; i< SIZE; i++)
	{
		gfds[i] = -1; //将文件描述符组设置为无效
	}
	gfds[0] = listen_sock;	//0号表示监听socket
	while(1)
	{
		int maxfd = -1;
		struct timeval timeout = {5,0};
		fd_set rfds;
		FD_ZERO(&rfds);
		for(i=0; i<SIZE; i++)
		{
			if(gfds[i] > maxfd)
			{
				maxfd = gfds[i];	//将最大文件描述符的值改变
			}
			if(gfds[i]>=0)
			{
				FD_SET(gfds[i], &rfds);	//设置文件描述副对应的文件描述符集
			}
		}
		switch(select(maxfd+1, &rfds, NULL, NULL, &timeout))
		{
			case -1: //error
				perror("select");
				break;
			case 0:	//超时
				printf("timeout....\n");
				break;
			default: //ok
				for(int j=0; j<SIZE; j++)
				{
					if(gfds[j] < 0)
						continue;	//文件描述符没有准备好
					else if(gfds[j]==listen_sock && FD_ISSET(gfds[j], &rfds))
					{
	                    struct sockaddr_in peer;
	                    socklen_t len = sizeof(peer);
						int sock = accept(listen_sock,(struct sockaddr*)&peer, &len);
						if(sock < 0)
						{
							perror("accept");
							exit(5);
						}
						int k=0;
						for(k=0; k<SIZE; k++)
						{
							if(gfds[k] == -1)
							{
								//寻找已经准备好的最小fd,并设置为accept sock
								gfds[k] = sock;
								printf("get a client,ip:%s,port:%d\n",inet_ntoa(peer.sin_addr),	ntohs(peer.sin_port));
								break;
							}
						}
						if(k == SIZE)
							close(sock);
					}
					else if(FD_ISSET(gfds[j], &rfds))
					{
						char buf[128];
						while(1)
						{
							ssize_t _r = read(gfds[j], buf, sizeof(buf));
							if(_r > 0)
							{
								buf[_r-1] = 0;
								printf("client##### %s\n",buf);
							}
							else if(_r == 0)
							{
								close(gfds[j]);
								gfds[j] = -1;
								break;
							}
							else
							{
								perror("read");
								exit(2);
							}
						}
					}
				}
		}
	}
	close(listen_sock);

	return 0;
}
