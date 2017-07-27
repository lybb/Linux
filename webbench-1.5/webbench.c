/*
 * (C) Radim Kolar 1997-2004
 * This is free software, see GNU Public License version 2 for
 * details.
 *
 * Simple forking WWW Server benchmark:
 *
 * Usage:
 *   webbench --help
 *
 * Return codes:
 *    0 - sucess
 *    1 - benchmark failed (server is not on-line)
 *    2 - bad param
 *    3 - internal error, fork failed
 * 
 */ 
#include "socket.c"
#include <unistd.h>
#include <sys/param.h>
#include <rpc/types.h>
#include <getopt.h>
#include <strings.h>
#include <time.h>
#include <signal.h>

/* values */
volatile int timerexpired=0;//用来设置alarm函数的超时值
int speed=0;//记录进程成功得到服务器响应的数量
int failed=0;//记录失败的数量
int bytes=0;//记录读取成功的字节数
/* globals */
int http10=1; /* 0 - http/0.9, 1 - http/1.0, 2 - http/1.1 */
/* Allow: GET, HEAD, OPTIONS, TRACE */
#define METHOD_GET 0
#define METHOD_HEAD 1
#define METHOD_OPTIONS 2
#define METHOD_TRACE 3
#define PROGRAM_VERSION "1.5"
int method=METHOD_GET;//请求方法是GET方法
int clients=1;	//默认只有一个客户端进行请求，可通过-c选项来更改
int force=0;	//是否需要服务器返回数据，设置为0表示需要等待
int force_reload=0;//是否需要缓存页面，0表示需要缓存，1表示不缓存
int proxyport=80;	//代理服务器的端口号是80
char *proxyhost=NULL;	//代理服务器的IP
int benchtime=30;		//默认的测压时间是30s,可通过-t选项修改
/* internal */
int mypipe[2];
char host[MAXHOSTNAMELEN];//服务器的IP
#define REQUEST_SIZE 2048
char request[REQUEST_SIZE];	//请求报文。最大长度是2048

//长选项，getopt_long的参数
static const struct option long_options[]=
{
	{"force",no_argument,&force,1},
	{"reload",no_argument,&force_reload,1},
	{"time",required_argument,NULL,'t'},
	{"help",no_argument,NULL,'?'},
	{"http09",no_argument,NULL,'9'},
	{"http10",no_argument,NULL,'1'},
	{"http11",no_argument,NULL,'2'},
	{"get",no_argument,&method,METHOD_GET},
	{"head",no_argument,&method,METHOD_HEAD},
	{"options",no_argument,&method,METHOD_OPTIONS},
	{"trace",no_argument,&method,METHOD_TRACE},
	{"version",no_argument,NULL,'V'},
	{"proxy",required_argument,NULL,'p'},
	{"clients",required_argument,NULL,'c'},
	{NULL,0,NULL,0}
};

/* prototypes */
static void benchcore(const char* host,const int port, const char *request);//对每个子进程实际进行测压的函数
static int bench(void);//fork子进程，父进程获取每个子进程的结果
static void build_request(const char *url);//构造HTTP请求

//信号处理函数，当时钟结束时进行调用
//当测试完成时，产生SIGALARM信号，调用该函数，将timerexpired设置为1，在后边的while循环中不断检测该值，当其为1时子进程退出
static void alarm_handler(int signal)
{
   timerexpired=1;
}	

//使用方法
static void usage(void)
{
   fprintf(stderr,
		   "webbench [option]... URL\n"
		   "  -f|--force               Don't wait for reply from server.\n"	//不需要等待服务器响应
		   "  -r|--reload              Send reload request - Pragma: no-cache.\n"	//重新发送请求
		   "  -t|--time <sec>          Run benchmark for <sec> seconds. Default 30.\n"//设置运行时间
		   "  -p|--proxy <server:port> Use proxy server for request.\n"			//使用代理服务器发送请求
		   "  -c|--clients <n>         Run <n> HTTP clients at once. Default one.\n"	//创建客户端的个数，默认是1
		   "  -9|--http09              Use HTTP/0.9 style requests.\n"	//http版本
		   "  -1|--http10              Use HTTP/1.0 protocol.\n"
		   "  -2|--http11              Use HTTP/1.1 protocol.\n"
		   "  --get                    Use GET request method.\n"	//使用GET方法请求
		   "  --head                   Use HEAD request method.\n"
		   "  --options                Use OPTIONS request method.\n"
		   "  --trace                  Use TRACE request method.\n"
		   "  -?|-h|--help             This information.\n"		//打印帮助消息
		   "  -V|--version             Display program version.\n"	//查看版本号
		   );
};

int main(int argc, char *argv[])
{
	int opt=0;//底下作为getopt_long的返回值
	int options_index=0;
	char *tmp=NULL;
	
	if(argc==1)//不带参数时直接输出help信息，并结束进程
	{
		usage();
		return 2;
	} 
	
	//使用getopt_long函数获取命令行参数，来设置全局变量的值
	while((opt=getopt_long(argc,argv,"912Vfrt:p:c:?h",long_options,&options_index)) != EOF)//该函数在调用时应多次调用，直到返回-1为止
	{//getopt_long解析命令行参数，但支持长选项
		switch(opt)
		{
			case  0 : break;
			case 'f': force=1;break;//force为1表明不需要等待服务器返回数据
			case 'r': force_reload=1;break; 
			case '9': http10=0;break;
			case '1': http10=1;break;
			case '2': http10=2;break;
			case 'V': printf(PROGRAM_VERSION"\n");exit(0);
			case 't': benchtime=atoi(optarg);break;	     
			case 'p': 
					  /* proxy server parsing server:port */
					  tmp=strrchr(optarg,':');	//tmp指向最后一个':'
					  proxyhost=optarg;
					  if(tmp==NULL)
					  {
						  break;
					  }
					  if(tmp==optarg)
					  {
						  fprintf(stderr,"Error in option --proxy %s: Missing hostname.\n",optarg);
						  return 2;
					  }
					  if(tmp==optarg+strlen(optarg)-1)
					  {
						  fprintf(stderr,"Error in option --proxy %s Port number is missing.\n",optarg);
						  return 2;
					  }
					  *tmp='\0';
					  proxyport=atoi(tmp+1);break;
			case ':':
			case 'h':
			case '?': usage();return 2;break;
			case 'c': clients=atoi(optarg);break;
		}
	}

	if(optind==argc){
		fprintf(stderr,"webbench: Missing URL!\n");
		usage();
		return 2;
	}
	
	//如果没有指定-c 和 -t选项，则将其默认设置为1和60秒
	if(clients==0) clients=1;
	if(benchtime==0) benchtime=60;

	//以下打印测试信息，包括：版本，请求方法及URL，并发的进程数，进程运行时间等等...
	
	/* Copyright */
	fprintf(stderr,"Webbench - Simple Web Benchmark "PROGRAM_VERSION"\n"
	    "Copyright (c) Radim Kolar 1997-2004, GPL Open Source Software.\n");

	build_request(argv[optind]);//根据argv[optind]，即URL来包装请求报头
	/* print bench info */
	printf("\nBenchmarking: ");
	switch(method)
	{
		case METHOD_GET:
		default:
			printf("GET");break;
	    case METHOD_OPTIONS:
			printf("OPTIONS");break;
	    case METHOD_HEAD:
			printf("HEAD");break;
	    case METHOD_TRACE:
			printf("TRACE");break;
	}
	printf(" %s",argv[optind]);
	switch(http10)
	{
	    case 0: printf(" (using HTTP/0.9)");break;
	    case 2: printf(" (using HTTP/1.1)");break;
	}
	printf("\n");
	if(clients==1) printf("1 client");
	else
		printf("%d clients",clients);
	
	printf(", running %d sec", benchtime);
	if(force) printf(", early socket close");
	if(proxyhost!=NULL) printf(", via proxy server %s:%d",proxyhost,proxyport);
	if(force_reload) printf(", forcing reload");
	printf(".\n");
	return bench();//开始测压----核心部分
}
	
void build_request(const char *url)
{
	char tmp[10];
	int i;
	
	//先将host(服务器IP)和request(请求报头)清零
	bzero(host,MAXHOSTNAMELEN);
	bzero(request,REQUEST_SIZE);
	
	//判断应该使用的协议版本
	if(force_reload && proxyhost!=NULL && http10<1) http10=1; //http/1.0
	if(method==METHOD_HEAD && http10<1) http10=1; //http/1.0
	if(method==METHOD_OPTIONS && http10<2) http10=2; //http/1.1
	if(method==METHOD_TRACE && http10<2) http10=2;   //http/1.1
	
	//确定方法
	switch(method)
	{
	    default:
	    case METHOD_GET: strcpy(request,"GET");break;
	    case METHOD_HEAD: strcpy(request,"HEAD");break;
	    case METHOD_OPTIONS: strcpy(request,"OPTIONS");break;
	    case METHOD_TRACE: strcpy(request,"TRACE");break;
	}
	  	  
	strcat(request," ");

	//判断URL是否合法
	if(NULL==strstr(url,"://"))
	{
	    fprintf(stderr, "\n%s: is not a valid URL.\n",url);
	    exit(2);
	}
	if(strlen(url)>1500)
	{
		fprintf(stderr,"URL is too long.\n");
		exit(2);
	}
	if(proxyhost==NULL)	//未使用代理服务器的情况下只能使用HTTP协议
		if (0!=strncasecmp("http://",url,7))
		{
			fprintf(stderr,"\nOnly HTTP protocol is directly supported, set --proxy for others.\n");
			exit(2);
		}

	/* protocol/host delimiter */
	i=strstr(url,"://")-url+3;
	/* printf("%d\n",i); */			//i==7（注意指针-指针是表示两个指针之间相隔了多少个该类型的元素，不是字节）
	
	//例如url是http://www.baidu.com/，则url+i指向的是第一个w处，即跳过协议以及://
	if(strchr(url+i,'/')==NULL) {	//如果在剩下的字符串中如果找不到一个'/'，即没有指明请求的资源，则报错
		fprintf(stderr,"\nInvalid URL syntax - hostname don't ends with '/'.\n");
		exit(2);
	}
	if(proxyhost==NULL)
	{
		/* get port from hostname */
		//是ip:port的形式，则分别解析出ip与port
		if(index(url+i,':')!=NULL && index(url+i,':')<index(url+i,'/')) //如果在/之前出现了:，则提取端口号
		{
			strncpy(host,url+i,strchr(url+i,':')-url-i);//将IP地址拷贝到host数组中
			bzero(tmp,10);
			strncpy(tmp,index(url+i,':')+1,strchr(url+i,'/')-index(url+i,':')-1);//找到端口号，先拷贝到tmp数组中
			/* printf("tmp=%s\n",tmp); */
			proxyport=atoi(tmp);	//将字符串类型的端口号转换为整型
			if(proxyport==0) proxyport=80;	//默认的代理服务器的端口号是80
		} 
		else //如果没有使用上述的ip:port形式，则只将ip存储进host中，默认的端口号是80
		{
			strncpy(host,url+i,strcspn(url+i,"/"));//如果没有端口号，则将“http://” 之后以及 ‘/’之前（即URL中的host部分）拷贝到host变量中
			//strcspn返回url+i的字符串中开头连续n个不包含‘/’的字符个数
		}
		// printf("Host=%s\n",host);
		strcat(request+strlen(request),url+i+strcspn(url+i,"/"));//此时request中存储的是：method ，现在将资源的路径追加在其之后
	}
	else	//使用了代理服务器时
	{
		// printf("ProxyHost=%s\nProxyPort=%d\n",proxyhost,proxyport);
		strcat(request,url);
	}

	//给request中追加协议版本
	if(http10==1)
	    strcat(request," HTTP/1.0");
	else if (http10==2)
	    strcat(request," HTTP/1.1");
	strcat(request,"\r\n");

	if(http10>0)	//HTTP/1.0 和 HTTP/1.1
	    strcat(request,"User-Agent: WebBench "PROGRAM_VERSION"\r\n");
		//"User-Agent"是客户端的一些必要信息，如浏览器版本，操作系统，浏览器语言等等。。。
	if(proxyhost==NULL && http10>0)
	{
	    strcat(request,"Host: ");//Host是请求的主机名
	    strcat(request,host);
	    strcat(request,"\r\n");
	}
	if(force_reload && proxyhost!=NULL)	//如果需要缓存响应页面，且使用了代理服务器时，则在报头中添加该字段，表名不缓存
	{
	    strcat(request,"Pragma: no-cache\r\n");//Pragma是缓存指令
	}
	if(http10>1)	//如果是HTPP/1.1版本，则存在长连接，则应该在请求报头中主动加上close，表名处理完请求tcp连接会被关闭
	    strcat(request,"Connection: close\r\n");
	/* add empty line at end */
	if(http10>0) strcat(request,"\r\n"); //在消息包头末添加空行
	// printf("Req=%s\n",request);
}

/* vraci system rc error kod */
static int bench(void)
{
	int i,j,k;	
	pid_t pid=0;
	FILE *f;
	
	//先检查与服务器是否能连通，如果不能，则需要关闭当前的端口
	/* check avaibility of target server */
	i=Socket(proxyhost==NULL?host:proxyhost,proxyport);
	if(i<0) { 
		fprintf(stderr,"\nConnect to server failed. Aborting benchmark.\n");
		return 1;
	}
	close(i);
	/* create pipe */
	if(pipe(mypipe))
	{
	    perror("pipe failed.");
	    return 3;
	}
	
	/* not needed, since we have alarm() in childrens */
	/* wait 4 next system clock tick */
	/*
	cas=time(NULL);
	while(time(NULL)==cas)
	      sched_yield();
	*/
	
	/* fork childs */
	for(i=0;i<clients;i++)
	{
	     pid=fork();
	     if(pid <= (pid_t) 0)//?????
	     {
	  	   /* child process or error*/
			 sleep(1); /* make childs faster */
			 break;
	     }
	}
	
	if( pid< (pid_t) 0)
	{
		fprintf(stderr,"problems forking worker no. %d\n",i);
	    perror("fork failed.");
	    return 3;
	}
	
	if(pid== (pid_t) 0)
	{
		/* I am a child */
		if(proxyhost==NULL)
			benchcore(host,proxyport,request);
		else
			benchcore(proxyhost,proxyport,request);
	
		/* write results to pipe */
		f=fdopen(mypipe[1],"w");
		if(f==NULL)
		{
			perror("open pipe for writing failed.");
			return 3;
		}
		/* fprintf(stderr,"Child - %d %d\n",speed,failed); */
		fprintf(f,"%d %d %d\n",speed,failed,bytes);
		fclose(f);//子进程写完后关闭文件描述符，并返回
		return 0;
	} else
	{//父进程
		f=fdopen(mypipe[0],"r");
		if(f==NULL) 
	    {
			perror("open pipe for reading failed.");
			return 3;
	    }
	    setvbuf(f,NULL,_IONBF,0);//将打开的文件流设置为无缓冲---直接读或者直接写
	    speed=0;
		failed=0;
		bytes=0;
	
	    while(1)
	    {
			pid=fscanf(f,"%d %d %d",&i,&j,&k);
			if(pid<2)
			{
				fprintf(stderr,"Some of our childrens died.\n");
				break;
			}
			speed+=i;
			failed+=j;
			bytes+=k;
			/* fprintf(stderr,"*Knock* %d %d read=%d\n",speed,failed,pid); */
			if(--clients==0) break;//当从所有的客户端(即子进程)都读完消息后，则跳出循环
		}
		fclose(f);

	//打印测压结果
	printf("\nSpeed=%d pages/min, %d bytes/sec.\nRequests: %d susceed, %d failed.\n",
	  	  (int)((speed+failed)/(benchtime/60.0f)),
	  	  (int)(bytes/(float)benchtime),
	  	  speed,
	  	  failed);
	}
	return i;
}


//该函数是实际的测压函数，会被每个子进程调用
void benchcore(const char *host,const int port,const char *req)
{
	int rlen;
	char buf[1500];
	int s,i;
	struct sigaction sa;
	
	/* setup alarm signal handler */
	sa.sa_handler=alarm_handler; //设置alarm_handler函数为信号处理函数
	sa.sa_flags=0;
	if(sigaction(SIGALRM,&sa,NULL)) //超时时会产生SIGALARM信号，会调用sa中所指定的函数去执行
		exit(3);
	alarm(benchtime);	//注册闹钟
	
	rlen=strlen(req);
	nexttry:while(1)
	{
		//一旦超时就返回
		if(timerexpired)//???????
		{
			if(failed>0)
			{
				/* fprintf(stderr,"Correcting failed by signal\n"); */
				failed--;
			}
			return;
		}
		s=Socket(host,port);//尝试与远端的服务器进行连接            
		if(s<0) { failed++;continue;} 
		if(rlen!=write(s,req,rlen)) {failed++;close(s);continue;}  //如果发送请求报头失败，则failed++
		if(http10==0) //http/0.9版本,关闭写操作，成功返回0，失败返回-1.(shutdown中1表示参数how设置为SHUT_WR)
			if(shutdown(s,1)) { failed++;close(s);continue;}
		if(force==0) //force表示是否需要等待服务器返回数据，为0时表示需要
		{
			/* read all available data from socket */
			while(1)
			{
				if(timerexpired) break;
				i=read(s,buf,1500); //从建立连接的socket处读取数据
				/* fprintf(stderr,"%d\n",i); */
				if(i<0) //从服务器读取数据失败
				{ 
					failed++;
					close(s);
	                goto nexttry;
				}
				else if(i==0) //数据读完
					break;
				else //读取成功
					bytes+=i;
			}
		}
	   if(close(s)) {failed++;continue;}
	   speed++;	
	}
}
