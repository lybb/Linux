#include "httpd.h"

#define _EPOLL_
//#define _PTHREAD_

//初始化
int startup(const char* ip, int port)
{
	assert(ip);

	int sock=socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
	{
		print_log("socket failed!  ",FATAL);
		return 1;
	}

	int opt=1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	local.sin_addr.s_addr = inet_addr(ip);
	socklen_t len = sizeof(local);

	if(bind(sock, (struct sockaddr*)&local, len) < 0)
	{
		print_log("bind_failed!  ",FATAL);
		return 2;
	}

	if(listen(sock,5) < 0)
	{
		print_log("listen failed!  ",FATAL);
		return 3;
	}
	return sock;
}

void Usage(const char* proc)
{
	printf("%s [local_ip] [local_port]",proc);
}

void* accept_request(void* arg)
{
#ifdef _EPOLL_
    int sock = ((fds*)arg)->sockfd;
    int epollfd = ((fds*)arg)->epollfd;
	return (void*)handle(sock);
#endif

#ifdef _PTHREAD_
	int sock = (int)arg;
	pthread_detach(pthread_self());
	return (void*)handle(sock);
#endif

}

int setnonblocking(int fd)
{
    int old_option = fcntl(fd,F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}

void Addfd(int epollfd,int fd,int oneshot)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN | EPOLLET;
    if(oneshot){
        ev.events |= EPOLLONESHOT;
    }
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&ev);
    setnonblocking(fd);
}

int main(int argc, char *argv[])
{
	if(argc != 3)  //判断启动服务器时参数是否正确
	{
		Usage(argv[0]);
	}
    int port = atoi(argv[2]);

#ifdef _EPOLL_
    printf("epoll_http_server is running on port:%d\n",port);

    int listen_sock = startup(argv[1],port);
    struct epoll_event events[MAX_EVNT_NUMBER]; //首先创建一个epoll模型
    int epfd = epoll_create(5);
    if(epfd == -1){
        perror("epoll_create");
        return -1;
    }
    Addfd(epfd,listen_sock,false); //将监听套接字加入监听事件中

    //不断从epoll模型的就绪队列中拿出就绪的文件描述符进行连接
    while(1){
        int ret = epoll_wait(epfd,events,MAX_EVNT_NUMBER,-1);
        if(ret < 0){
            perror("epoll_wait");
            break;
        }
        int i=0;
        for(; i<ret; ++i){
            int sockfd = events[i].data.fd;
            if(sockfd == listen_sock){
                struct sockaddr_in client;
                socklen_t len = sizeof(client);
        		int connfd = accept(listen_sock,(struct sockaddr*)&client,&len);
                Addfd(epfd,connfd,true);
            }else if(events[i].events & EPOLLIN){
                pthread_t new_thread;
                fds fds_for_new_worker;
                fds_for_new_worker.epollfd = epfd;
                fds_for_new_worker.sockfd = sockfd;
                pthread_create(&new_thread, NULL, accept_request, (void*)&fds_for_new_worker);
            }else{
                printf("nothing happened!\n");
            }

        }
    }
    close(epfd);
#endif

#ifdef _PTHREAD_
    printf("pthread_http_server is running on port:%d\n",port);

	int listen_sock = startup(argv[1],port);
//	daemon(1,0);   //将服务器创建成守护进程
	while(1)
	{
		//每监听到一个新链接时，创建一个新线程去执行，主线程继续监听端口
	    struct sockaddr_in client;
    	socklen_t len = sizeof(client);
		int sock = accept(listen_sock,(struct sockaddr*)&client,&len);
		if(sock < 0)
		{
			print_log("accept failed!",FATAL);
			continue ;
		}

		pthread_t tid;
		pthread_create(&tid, NULL, accept_request, (void*)sock);
	}
	close(listen_sock);
#endif
	return 0;
}
