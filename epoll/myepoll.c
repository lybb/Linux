#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/epoll.h>
#include<string.h>

#define _SIZE_ 10240
typedef struct epbuf
{
	int fd;
	char buf[_SIZE_];
}epbuf_t, *epbuf_p, **epbuf_pp;

//申请一块buf缓冲区
static epbuf_p alloc_epbuf(int fd)
{
	epbuf_p ptr = (epbuf_p)malloc(sizeof(epbuf_t));
	if(ptr == NULL)
	{
		perror("malloc");
		return NULL;
	}

	ptr->fd = fd;
	return ptr;
}

//释放buf
static void del_epbuf(epbuf_p ptr)
{
	if(ptr != NULL)
		free(ptr);
}

//初始化
int startup(const char* ip, int port)
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
	{
		perror("socket");
		exit(2);
	}
	int opt = 1;
	setsockopt(0, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	local.sin_addr.s_addr = inet_addr(ip);
	socklen_t len = sizeof(local);
	//bind
	if(bind(sock, (struct sockaddr*)&local, len) < 0)
	{
		perror("bind");
		exit(3);
	}
	//listen
	if(listen(sock, 5) < 0)
	{
		perror("listen");
		exit(4);
	}
	return sock;
}

int main(int argc, const char* argv[])
{
	if(argc != 3)
	{
		printf("Usage:%s, [ip], [port]\n", argv[0]);
		return 1;
	}

	int listen_sock = startup(argv[1], atoi(argv[2]));

	//创建epoll模型
	int epfd = epoll_create(256); //创建成功后空结点的RB Tree
	if(epfd < 0)
	{
		perror("epoll_create");
		close(epfd);
		exit(5);
	}
	printf("epfd = %d\n",epfd);

	//添加listen_sock
	struct epoll_event _ev;
	_ev.events = EPOLLIN | EPOLLET;
	_ev.data.ptr = alloc_epbuf(listen_sock);
	epoll_ctl(epfd, EPOLL_CTL_ADD, listen_sock, &_ev);

	int timeout = 5000;
	while(1)
	{
		int num = 0;
		struct epoll_event _ev[32];
		int maxev = 32;
		switch((num = epoll_wait(epfd, _ev, maxev, timeout)))
		{
			case -1:	//error
				perror("epoll_wait");
				exit(6);
				break;
			case 0:		//timeout
				printf("timeout...\n");
				break;
			default:	//有事件变化
				{
					int i=0;
					for(; i< num; i++)
					{
						int fd = ((epbuf_p)(_ev[i].data.ptr))->fd;

						if((fd == listen_sock) && (_ev[i].events & EPOLLIN)) //fd为监听套接字且此时有读入事件发生
						{
							//accept
							struct sockaddr_in peer;
							socklen_t len = sizeof(peer);
							int new_sock = accept(listen_sock, (struct sockaddr*)&peer, &len);
							if(new_sock < 0)
							{
								perror("accept");
								continue;	//失败可以重新accept
							}

							//accept成功后，接收数据需要有自己的缓冲区buf------alloc_ptr
							_ev[i].events = EPOLLIN;	//更改事件为EPOLLIN
							_ev[i].data.ptr = alloc_epbuf(new_sock);	//给每个fd开辟自己的一块缓冲区
							epoll_ctl(epfd, EPOLL_CTL_ADD, new_sock, _ev);	//将就绪的fd的事件添加到epoll模型里边
						}//if
						else if((fd != listen_sock) && (_ev[i].events & EPOLLIN))	//fd不是监听套接字 & 此时有读事件就绪
						{
							//接收数据并存入自己的buf
							char* buf = ((epbuf_p)(_ev[i].data.ptr))->buf;
							ssize_t _r = read(fd, buf, _SIZE_-1);
							if(_r < 0)	//error
							{
								perror("read");
							//	continue ;	//这次read失败可以重新读取
							}
							else if(_r == 0)	//client quit
							{
								//对方退出，则server需要释放fd对应的缓冲区，删除fd结点，并且关闭链接
								del_epbuf(_ev[i].data.ptr);
								_ev[i].data.ptr = NULL;
								epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
								close(fd);
								printf("client is quit!\n");
							}
							else	//_s>0 ,读成功
							{
								buf[_r] = 0;
								printf("%s\n", buf);

								//将事件修改为写事件
								_ev[i].events = EPOLLOUT;
								epoll_ctl(epfd, EPOLL_CTL_MOD, fd, _ev);
							}
						}//else if
						else if((fd != listen_sock) && (_ev[i].events & EPOLLOUT)) //fd不是监听套接字 & 此时有写事件就绪
						{
							//char* buf = ((epbuf_p)(_ev[i].data.ptr))->buf;
							const char* msg = "HTTP/1.0 200 OK\r\n\r\n<html><h1>HELLO WORLD!</h1></html>\n";
							write(fd, msg, strlen(msg));

							//写完之后删除缓冲区内存，并删除fd关心的事件
							del_epbuf(_ev[i].data.ptr);
							_ev[i].data.ptr = NULL;
							epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
							close(fd);
						}//else if
						else	//other thing
						{}//else
					}//for
				}
				break;
		}
	}

	close(epfd);
	close(listen_sock);
	return 0;
}
