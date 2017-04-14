#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#define SIZE 1024

static void math_data(char* content_data)
{
	assert(content_data);

	char* arr[3];
	int i=0;
	char* start = content_data;
	//METHOD=...&...&...&...
	while(*start != '\0')
	{
		if(*start == '=')
		{
			arr[i] = start+1;
			i++;
		}
		else if(*start == '&')
		{
			*start = '\0';
		}
		else{}
		start++;
	}

	printf("<html><h1>data1 + data2 = %d\n</h1></html>",atoi(arr[0]) + atoi(arr[1]));
}

int main()
{

	printf("get in cgi_math.c!\n");
	//获取环境变量
	//GET->query_string    POST->CONTENT-LENGTH
	char method[SIZE];
	char query_string[SIZE];
	char content_len[SIZE];
	char content_data[SIZE];

	if( getenv("METHOD") )
	{
		strcpy(method, getenv("METHOD"));
	}

	if(strcasecmp(method,"GET") == 0)
	{
		if( getenv("QUERY_STRING"))
		{
			strcpy(query_string, getenv("QUERY_STRING"));
		}
		else
		{
			printf("QUERY-STRING is not exist!\n");
		}
	}
	else
	{
		//是POST方法就从浏览器读取content_length个字符------参数
		int i=0;
		strcpy(content_len, getenv("CONTENT-LENGTH"));
		int len=atoi(content_len);
		char ch='\0';
		for(; i<len; i++)
		{
			read(0, &ch, 1);
			content_data[i] = ch;
		}
		content_data[i] = '\0';
	}

	math_data(content_data);

	return 0;
}
