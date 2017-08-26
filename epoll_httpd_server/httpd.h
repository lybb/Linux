#ifndef _HTTPD_
#define _HTTPD_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>

#include <sys/epoll.h>

#define NORMAL 0
#define WARNING 1
#define FATAL 2

#define SIZE 4096
#define MAX_EVNT_NUMBER 1024

typedef enum{
    false=0,
    true=!false
}bool;

typedef struct fds{
    int epollfd;
    int sockfd;
}fds;

void Usage(const char* proc);
void print_log(const char* log_msg, int level);  //打印日志消息
//int startup(const char* ip, int port);
//static int get_line(int sock, char* buf, int sz);
//void* accept_request(void* arg);
int handle(int fd);  //请求处理函数
void echo_errno();   //回显错误的请求

#endif
