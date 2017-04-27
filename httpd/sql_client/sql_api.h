//#ifndef _SQL_API_H_
//#define _SQL_API_H_

#include <iostream>
#include <string>
#include <mysql.h>
using namespace std;

class mysqlapi
{
	public:
		mysqlapi();
		~mysqlapi();

		bool sql_connect();
		int sql_insert(string& data);
		int sql_delete(const string& data);
		int sql_select(const string& data);
		int sql_update(const string& data);
	private:
		MYSQL conn;
};

//#endif
