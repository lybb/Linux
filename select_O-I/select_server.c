#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


static Usage(char* proc)
{
	printf("usage:%s [local_ip][local_port]\n",proc);
}

int startup(const char* ip,int port)
{
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock < 0)
	{
		perror("socket!");
		exit(2);
	}

	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	local.sin_addr.s_addr = inet_addr(ip);

	if(bind(sock,(struct sockaddr*)&local,sizeof(local)) < 0)
	{
		perror("bind!");
		exit(3);
	}

	if(listen(sock,10) < 0)
	{
		perror("listen!");
		exit(4);
	}
	return sock;//返回一个监听套接字
}

int main(int argc, char* argv[])
{
	if(argc != 3)
	{
		Usage(argv[0]);
		return 1;
	}
	int listen_sock = startup(argv[1],atoi(argv[2]));
	printf("get a listen socket:%d\n",listen_sock);

	int fds[sizeof(fd_set)*8];
	int nums = sizeof(fds)/sizeof(fds[0]);
	int i=0;
	for(; i<nums; i++)	//先全部将第三方数组fds的文件描述符设置为无效
	{
		fds[i] = -1;
	}
	fds[0] = listen_sock;	//0号位置表示的是监听套接字
	fd_set rfds;
	while(1)
	{
		int maxfd = -1;//表示最大的文件描述符
		struct timeval timeout = {5,0};
		FD_ZERO(&rfds);	//由于select的后几个参数都是输入输出型参数，因此每次select之前都应该将rfds集清空
		int i=0;
		for(; i<nums; i++)
		{
			if(fds[i] == -1)
				continue ;
			FD_SET(fds[i],&rfds);		//将有效的文件描述符设置进rfds中
			if(fds[i] > maxfd)
				maxfd = fds[i];	//每次都需要更新maxfd
		}
		switch(select(maxfd+1,&rfds,NULL,NULL,&timeout))
		{
			case -1://error!!
				perror("select");
				exit(5);
				break;
			case 0://timeout
				printf("timeout...\n");
				break;
			default ://至少有一个文件描述符就绪
				for(i=0; i<nums; i++)	//循环检测fds数组，查看是否有文件描述符上的事件已经就绪
				{
					if(i==0  &&  FD_ISSET(fds[i],&rfds))//如果是0号即listen_sock就绪，表明有新的连接请求，则accept新连接
					{
						struct sockaddr_in client;
						socklen_t len = sizeof(client);
						int new_sock = accept(fds[0],(struct sockaddr*)&client,&len);
						if(new_sock < 0)	//accept失败后，重新继续监听
						{
							perror("accept");
							continue ;
						}

						//accept成功后不能立即读取数据，除非这个文件描述符上的事件已经发生了才表示有数据可以读取
						//将新连接放入rfds中继续由select托管 
						int j=1;
						for(; j<nums; j++)
						{
							if(fds[j] == -1)	//在fds中找到一个没有用的位置放入j号文件描述符
								break ;
						}
						if(j == nums)	//已达上限
						{
							close(new_sock);
						//	exit(6);
						}
						else	//找到位置
						{
							fds[j] = new_sock;
						}
					}//fi
					else if(i != 0  &&  FD_ISSET(fds[i],&rfds))	//普通文件描述符就绪,可以读取数据
					{
						char buf[1024];
						ssize_t s = read(fds[i],buf,sizeof(buf)-1);
						if(s < 0)
						{
							perror("read!");
							//exit(7);
							break;
						}
						else if(s == 0)
						{
							printf("client is quit!\n");
							close(fds[i]);
							fds[i] = -1;	//关闭该文件描述符并设置为无效
							break ;
						}
						else
						{
							buf[s] = 0;
							printf("client send::%s\n",buf);
						}
					}//else
					else	//没有文件描述符就绪---无事件发生
					{
						continue ;
					}
				}
				break;
		}
	}
	close(listen_sock);
	return 0;
}
