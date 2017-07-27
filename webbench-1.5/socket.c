/* $Id: socket.c 1.1 1995/01/01 07:11:14 cthuang Exp $
 *
 * This module has been modified by Radim Kolar for OS/2 emx
 */

/***********************************************************************
  module:       socket.c
  program:      popclient
  SCCS ID:      @(#)socket.c    1.5  4/1/94
  programmer:   Virginia Tech Computing Center
  compiler:     DEC RISC C compiler (Ultrix 4.1)
  environment:  DEC Ultrix 4.3 
  description:  UNIX sockets code.
 ***********************************************************************/
 
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

//该函数主要以host和clientPort来组成一个TCP套接字并进行与对应主机的连接，创建失败返回-1，否则返回这个socket。

int Socket(const char *host, int clientPort)
{
    int sock;
    unsigned long inaddr;
    struct sockaddr_in ad;//创建一个结构体存放IPv4地址
    struct hostent *hp;
    
    memset(&ad, 0, sizeof(ad));
    ad.sin_family = AF_INET;

	//将char*类型的host转换为网络字节序unsigned long
    inaddr = inet_addr(host);//inet_addr函数是将点分十进制的IP地址转换成网络中的二进制字节序。失败返回INADDR_NONE
    if (inaddr != INADDR_NONE)	//#define INADDR_NONE 0xffffffff
        memcpy(&ad.sin_addr, &inaddr, sizeof(inaddr));//如果转换的inaddr有效，就用inaddr初始化ad结构体中的ad.sin_addr
    else
    {
		//gethostbyname返回一个给定对应主机名字和信息的hostent结构体(里边包含主机名，主机别名，ip及其类型和长度)
        hp = gethostbyname(host);
        if (hp == NULL)
            return -1;
        memcpy(&ad.sin_addr, hp->h_addr, hp->h_length);
    }
    ad.sin_port = htons(clientPort);//将端口号转换为网络字节序
    
	//创建一个socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        return sock;
	//连接到相应的主机
    if (connect(sock, (struct sockaddr *)&ad, sizeof(ad)) < 0)
        return -1;
    return sock;
}
