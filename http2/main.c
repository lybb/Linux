#include "httpd.h"

void Usage(const char* proc)
{
	printf("%s [local_ip] [local_port]",proc);
}

void* accept_request(void* arg)
{
	int sock = (int)arg;
	pthread_detach(pthread_self());
	return (void*)handle(sock);
}

int main(int argc, char *argv[])
{
	if(argc != 3)
	{
		Usage(argv[0]);
	}
	int listen_sock = startup(argv[1],atoi(argv[2]));
	

//	daemon(1,0);

	while(1)
	{
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
	return 0;
}
