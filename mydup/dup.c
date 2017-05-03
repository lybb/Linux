#include<stdio.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>
#include<unistd.h>

int main()
{
	int fd = open("./log", O_CREAT | O_RDWR);
	if(fd < 0)
	{
		perror("open");
		exit(1);
	}
	close(1);

	int new_fd = dup(fd);
//	if(new_fd < 0)
//	{
//		perror("dup");
//		exit(3);
//	}
//	close(fd);

	char* msg = "hello world!\n";
	char buf[1024];
	int i=10;
	while(i--)
	{
		ssize_t r = write(fd, msg, strlen(msg));
		if(r < 0)
		{
			perror("write");
			exit(2);
		}
		fflush(stdout);
	}

	return 0;
}
