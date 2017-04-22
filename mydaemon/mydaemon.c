#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/stat.h>

void mydaemon()
{
	umask(0);

	if(fork() > 0)
	{
		//father
		exit(0);
	}

	//child
	setsid();

	chdir("/");

	close(0);
	close(1);
	close(2);

	signal(SIGCHLD,SIG_IGN);
}

int main()
{
//	mydaemon();
	while(1)
	{
		sleep(1);
	}
	return 0;
}
