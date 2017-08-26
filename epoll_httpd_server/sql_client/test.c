#include <stdio.h>
#include <mysql.h>
#include <pthread.h>
//#include <my_global.h>

int main(int argc,char* argv[])
{
	printf("mysql_client:%s\n",mysql_get_client_info());
	return 0;
}
