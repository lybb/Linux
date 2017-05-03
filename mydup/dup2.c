#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>

int main()
{
	int fd = open("./log", O_CREAT | O_RDWR);
	if(fd < 0)
	{
		perror("open");
		exit(1);
	}

	int new_fd = dup2(fd, 1);
	if(new_fd < 0)
	{
		perror("dup2");
		exit(2);
	}
	close(fd);

	int count = 10;
	char* msg = "HELLO WORLD!\n";
	while(count--)
	{
		ssize_t _s = write(new_fd, msg, strlen(msg));
		if(_s < 0)
		{
			perror("write");
			exit(3);
		}
		fflush(stdout);
	}

	close(new_fd);
	return 0;
}
