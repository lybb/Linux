#include <stdio.h>
#include "sql_api.h"

mysqlapi::mysqlapi()
{
	mysql_init(&conn);
}

mysqlapi::~mysqlapi()
{
	mysql_close(&conn);
}

bool mysqlapi::sql_connect()
{
	if(!mysql_real_connect(&conn,"127.0.0.1","root","root","mytable",3306,NULL,0))
	{
		cout<<"mysql_real_connect failed!"<<endl;
		perror("mysql_real_connect");
		return false;
	}
	else
		return true;
}
int mysqlapi::sql_insert(string& data)
{
	string sql = "INSERT INTO student_info(name,sex,age,school)VALUES(";
	sql += data;
	sql += ")";
	mysql_query(&conn,sql.c_str());
}
int mysqlapi::sql_delete(const string& data)
{
	string sql = "DELETE FROM student_info WHERE name=";
	sql += data;
	sql += ";";
//	cout<<sql.c_str()<<endl;
	mysql_query(&conn,sql.c_str());
}
int mysqlapi::sql_select(const string& data)
{
	string sql = "SELECT * FROM student_info";
	if(mysql_query(&conn,sql.c_str()) == 0)
	{
		MYSQL_RES* res = mysql_store_result(&conn);
		int row = mysql_num_rows(res);
		int field = mysql_num_fields(res);
//		cout<<"row="<<row<<" "<<"field="<<field<<endl;

		MYSQL_ROW line;
		for(int i=0; i<row; i++)
		{
			line = mysql_fetch_row(res);
			for(int j=0; j<field-1; j++)
			{
				cout<<line[j]<<" ";
			}
			if(i != row-1)
				cout<<" , ";

			printf("<br/>");
		}
	}
}
int mysqlapi::sql_update(const string& data)
{
	string sql = "UPDATE student_info SET school='sust' WHERE name=";
	sql += data;
	sql += ";";
	mysql_query(&conn,sql.c_str());
}

//int main()
//{
//	mysqlapi s;
//	string data="lisi";
//	s.sql_connect();
//	s.sql_select(data);
//	return 0;
//}
