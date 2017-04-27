#include "httpd.h"

//打印错误信息
void print_log(const char* log_msg, int level)
{
	char* err_list[5]={
		"NORMAL",
		"WARNING",
		"FATAL",
	};
//	printf("[%s]...[%s]",log_msg,err_list[level]);

//#ifdef DEBUG
//#define DEBUG
	char* log_path = "log/filelog";
	int fd = open(log_path, O_WRONLY|O_APPEND|O_CREAT, 0644);
	
	char buf[SIZE];
	memset(buf, 0, sizeof(buf));
	strncpy(buf, err_list[level], strlen(err_list[level]));//错误等级和提示
	strncat(buf,": ",strlen(": "));
	strncat(buf,log_msg,strlen(log_msg));//错误信息
	time_t t;
	char ti[30];
	ti[0] = ' ';
	time(&t);
	strcpy(ti+1,ctime(&t));
	strcat(buf,ti);
	write(fd, buf, strlen(buf));
	close(fd);
//#endif
}

//按行读取内容
static int get_line(int sock, char * buf, int len)
{
	assert(buf);
	char ch='\0';
    int i = 0;
	while(i < len -1 && ch != '\n' )
	{
		if(recv(sock, &ch, 1, 0) > 0)
		{
			if(ch == '\r')
			{
				//考虑下一个字符是否为\n
				if( recv(sock, &ch, 1, MSG_PEEK) > 0 && ch == '\n' )
					recv(sock, &ch, 1, 0);
				else
					ch = '\n';
			}
			buf[i++] = ch;
		}
	}
	buf[i] = '\0';
	return i;
}

static int clear_header(int sock)
{
	char line[SIZE];
	int ret = -1;
	do{
		ret = get_line(sock, line, SIZE);
	}while(ret != 1  &&  strcmp(line,"\n")!=0);

	return ret;
}

//返回正确页面 200 OK
static int echo_www(int sock, const char* path, ssize_t size)
{
	int ret = -1;
	int fd = open(path, O_RDONLY);
	if(fd < 0)
	{
		print_log("open failed!  ",FATAL);
		echo_errno(sock,503);
		ret = 8;
	}

	char line[SIZE*10];
	//响应报头
	sprintf(line, "HTTP/1.0 200 OK\r\n");
	send(sock, line, strlen(line), 0);
//	send(sock, "\r\n", strlen(line), 0);
	send(sock, "\r\n", 2, 0);
	if(sendfile(sock, fd, NULL, size) < 0)
	{
		print_log("sendfile is failed!  ", FATAL);
		echo_errno(sock,503);
		ret = 9;
	}
	close(fd);
	return ret;
}

//server应答错误页面
void bad_request(const char* path, const char* head, int sock)
{
	struct stat st;			   
	if(stat(path, &st) < 0)
	{
		return;
	}			    
	int fd = open(path, O_RDONLY);					    

	//响应报头+空行
	const char* status_line = head;
	send(sock,status_line,strlen(status_line),0);
	const char* content_type = "Content-Type:text/html;charset=ISO-8859-1\r\n";
	send(sock,content_type,strlen(content_type),0);
	send(sock, "\r\n", 2, 0);

	sendfile(sock, fd, NULL,st.st_size);
	close(fd);
}

void echo_errno(int sock, int err_code)
{
	switch(err_code)
	{
		case 404:
			bad_request("wwwroot/404.html", "HTTP/1.0 404 Not Found\r\n", sock);
			break;
		case 503:
			bad_request("wwwroot/503.html","HTTP/1.0 503 Server Unavailable\r\n",sock);
			break;
		default:
			break;
  }
}

static int excu_cgi(int sock,const char* method, char* path,const char* query_string)
{
	int ret = 0;
	int content_len = -1;
	//GET方法
	if(strcasecmp(method, "GET") == 0)
	{
		clear_header(sock);
	}
	else
	{
		//是POST方法需要读取参数，而读多少字节能读完是依靠Content-Length所提示的大小知道的
		char line[SIZE];
		do
		{
			ret = get_line(sock, line, sizeof(line));
			//printf("DEBUG:ret=%d\n",ret);

			//获得请求消息的大小
			if(ret>0  &&  strncasecmp(line,"Content-Length: " ,16) == 0)
			{
				content_len = atoi(line+16);//取得content-length的大小
			}
		}while(ret != 1  &&  strcmp(line,"\n") != 0);

		if(content_len < 0)
		{
			print_log("have no arguments!  ",FATAL);
			echo_errno(sock,404);
			return 10;
		}
	}

	//响应报头+空行
	const char* status_line = "HTTP/1.0 200 OK\r\n";
	send(sock, status_line, strlen(status_line), 0);
	const char* content_type = "Content-Type:text/html;charset=ISO-8859-1\r\n";
	send(sock, content_type, strlen(content_type), 0);
	send(sock, "\r\n", 2, 0);

	//创建管道
	int input[2];
	int output[2];
	pipe(input);
	pipe(output);

	//fork子进程
	pid_t id;
	id = fork();
	if(id == 0)
	{//child
		//1.关闭相应文件描述符
		//2.对文件描述符进行重定向
		//3.通过环境变量传递参数(method,path,query_string,content_len)
		//4.进行exec程序替换
		close(input[1]);
		close(output[0]);
		//close(sock);
		dup2(input[0],0);//将标准输入重定向到子进程
		dup2(output[1],1);//将子进程的输出重定向到标准输出---->浏览器

		char method_env[SIZE/8];
		char query_string_env[SIZE/4];
		char content_len_env[SIZE/8];
		sprintf(method_env, "METHOD=%s", method);
		putenv(method_env);
		//GET:传递query_string     POST：传递content_len
		if(strcasecmp(method, "GET") == 0)
		{
			sprintf(query_string_env, "QUERY-STRING=%s", query_string);
			putenv(query_string_env);
		}
		else
		{
			sprintf(content_len_env, "CONTENT-LENGTH=%d", content_len);
			putenv(content_len_env);
		}

		execl(path, path, NULL);
		exit(1);
	}
	else
	{//father
		//1.关闭适当的文件描述符
		//2.方法决定读写顺序(POST：需要继续读取数据，直到读完POST方法的参数;GET：可以直接从子进程读取结果)
		//3.将数据和方法全部交给子进程后等待子进程的结果

		close(input[0]);
		close(output[1]);

		//POST
		char c = '\0';
		if(strcasecmp(method, "POST") == 0)
		{
			int i =0;
			//一个一个字符读取并通过管道传递给子进程
			for(; i<content_len; i++)
			{
				recv(sock, &c, 1, 0);//先sock获取数据
				write(input[1],&c, 1);//通过管道传给子进程
			}
		}

		//GET
		while(read(output[0], &c, 1) > 0)
		{
			send(sock, &c, 1, 0);
		}

		close(input[1]);
		close(output[0]);
		waitpid(id,NULL,0);
	}
}

//处理函数
int handle(int sock)
{
	int ret=0;
	int i=0;
	int j=0;
	int cgi=0;
	char buf[SIZE];
	char method[64];	//方法
	char url[SIZE];
	char path[SIZE];
	char* query_string = NULL;	//参数（GET方法）
	if(get_line(sock, buf, sizeof(buf)) <= 0)//如果为空行，至少会有一个'\n',因此应为<=0时读取失败
	{
		print_log("get_line error!  ",FATAL);
		echo_errno(sock,404);
		ret = 5;
		goto end;
	}

	// GET /XX/YY/ZZ HTTP/1.0
	// 提取方法：遇到空格，则之前已经读到的字符就是方法 
	while(i<sizeof(buf)-1  &&  j<sizeof(method)-1  &&  !isspace(buf[j]))
	{
		method[j++] = buf[i++];
	}
	method[j] = '\0';

	//只处理 GET 和 POST 方法
	if(strcasecmp(method,"GET") && strcasecmp(method,"POST"))
	{
		echo_errno(sock,404);
		ret = 6;
		goto end;
	}
	if(strcasecmp(method,"POST") == 0)	//POST要支持CGI模式
	{
		cgi = 1;
	}

	//过滤掉空格，使j指向资源路径的有效字符处
	while(i<sizeof(buf)  &&  isspace(buf[i]))
		i++;
	//此时：i-> /XX/YY/ZZ?a=a&b=b&c=c HTTP/1.0
	
	//提取url，分离参数
	j=0;
	while(i<sizeof(buf) && j<sizeof(url)-1 && !isspace(buf[i]))
	{
		url[j++] = buf[i++];
	}
	url[j] = '\0';

	//GET方法---提取参数给query_string
	if(strcasecmp("GET",method) == 0)
	{
		query_string = url;
		while(*query_string != '\0' && *query_string != '?')
			query_string++;

		//如果有'?'表明有参数，使query_string指向参数处
		if(*query_string == '?')
		{
			*query_string = '\0';
			query_string++;
			cgi = 1;
		}
	}

	//转换路径：/XX/YY/ZZ   -->   wwwroot/XX/YY/ZZ
	sprintf(path,"wwwroot%s",url);
	if(path[strlen(path)-1] == '/')		//如果是一个目录就拼上默认主页
	{
		strcat(path,"index.html");
	}

//	printf("path:%s\n",path);
	//检查资源是否存在
	struct stat st;
	if(stat(path, &st) < 0)
	{
		print_log("file is not exist!  ",WARNING);
		clear_header(sock);
		echo_errno(sock,404);
		ret = 7;
		goto end;
	}
	else
	{
		//先判断是不是一个普通文件
		if(S_ISREG(st.st_mode))
		{
		    if((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH))	//是一个可执行程序
				cgi = 1;
		}
		else if(S_ISDIR(st.st_mode))//是一个文件目录
		{
			strcat(path, "/index.html");
		}
		else
		{
			//DO NOTHING!!!
		}
	}

	//处理 cgi模式或 非cgi模式
	if(cgi == 1)
	{
		ret = excu_cgi(sock, method, path, query_string);
	}
	else
	{
		ret = clear_header(sock);
		ret = echo_www(sock, path, st.st_size);
	}
end:
	close(sock);
	return ret;
}
