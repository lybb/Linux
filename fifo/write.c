#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>

int main()
{
	umask(0);
	int ret = mkfifo("./myfifo",S_IFIFO | 0666);//创建一个命名管道且存取权限为666
	if(ret == -1)
	{
		perror("mkfifo");
		return 1;
	}

	char* msg = "hello world!\n";

	int fd = open("./myfifo",O_WRONLY);//以写的方式打开该管道
	if(fd == -1)
	{
		perror("open");
		return 2;
	}

	char buf[128];
	int count = 10;
	while(count--)
	{
		ssize_t w = write(fd,msg,strlen(msg));
		sleep(1);
		if(w < 0)
		{
			perror("write");
			return 3;
		}
	}
	close(fd);

	return 0;
}
