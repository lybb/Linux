#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>

int main()
{
	umask(0);
	int fd = open("./myfifo",O_RDONLY | 0666);
	if(fd < 0)
	{
		printf("open error!\n");
		return 1;
	}

	char buf[128];
	int count = 10;
	while(count--)
	{
		sleep(1);
		ssize_t r = read(fd,buf,sizeof(buf)-1);
		if(r > 0)
		{
			buf[r] = '\0';
			printf("buf is :%S\n",buf);
		}
		else
		{
			perror("read");
			return 3;
		}
	}
	close(fd);
	return 0;

}

