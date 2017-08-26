#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#define SIZE 1024

static void math_data(char* data)
{
//	assert(data);

	char* arr[3];
	int i=0;
	char* start = data;
	//METHOD=...&...&...&...
	while(*start)
	{
	perror("math_data !");
		if(*start == '=')
		{
			arr[i] = start+1;
			i++;
		}
		else if(*start == '&')
		{
			*start = '\0';
		}
		else{
			//DO NOTHING!!
		}
		start++;
	}
	arr[i] = NULL;

	int num1 = atoi(arr[0]);
	int num2 = atoi(arr[1]);

	perror("data1 and data2!");
	printf("<html>\n");
	printf("<body>\n");
	printf("<h1>data1 + data2 = %d</h1>\n",num1 + num2);
	printf("<h1>data1 - data2 = %d</h1>\n",num1 - num2);
	printf("<h1>data1 * data2 = %d</h1>\n",num1 * num2);
	printf("<h1>data1 / data2 = %d</h1>\n",num1 / num2);
	printf("</body>\n");
	printf("</html>\n");
}

int main()
{
//	printf("hello world!\n");
	//获取环境变量
	//GET->query_string    POST->CONTENT-LENGTH
	char method[SIZE];
	char content_len[SIZE];
	char data[SIZE];

	if( getenv("METHOD") )
	{
		strcpy(method, getenv("METHOD"));
	}
	else
	{
		printf("METHOD is no exist!\n");
		return 1;
	}

	if(strcasecmp(method,"GET") == 0)
	{
		if( getenv("QUERY-STRING"))
		{
			strcpy(data, getenv("QUERY-STRING"));
		}
		else
		{
			printf("QUERY-STRING is not exist!\n");
		}
	}
	else
	{
		//是POST方法就从内存读取content_length个字符------参数
		int i=0;
		strcpy(content_len, getenv("CONTENT-LENGTH"));
		int len=atoi(content_len);
		char ch='\0';
		for(; i<len; i++)
		{
			read(0, &ch, 1);
			data[i] = ch;
		}
		data[i] = '\0';
	}

	math_data(data);

	return 0;
}
