#include "httpd.h"

int main(int argc, char *argv[])
{
	if(argc != 3)
	{
		Usage(argv[0]);
	}
	int listen_sock = startup(argv[1],atoi(argv[2]));
	
	struct sockaddr_in client;
	socklen_t len = sizeof(client);
	while(1)
	{
		int sock = accept(listen_sock,(struct sockaddr*)&client,&len);
		if(sock < 0)
		{
			print_log("accept failed!",FATAL);
			continue ;
		}

//		printf("get a client,ip:%s,port:%d\n",inet_ntoa(client.sin_addr),ntohs(client.sin_port));
		pthread_t tid;
		pthread_create(&tid, NULL, accept_request, (void*)sock);
//		if(pthread_create(&tid, NULL, accept_request, (void*)sock) > 0)
//		{
//			print_log("create thread failed!",FATAL);
//			close(sock);
//		}
	}
	close(listen_sock);
	return 0;
}
