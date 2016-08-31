#ifndef MYSQLCONNECTION_H
#define MYSQLCONNECTION_H

struct ConnectionInfo
{
	std::string m_dbName;
	std::string m_username;
	std::string m_password;
};

class MySQLConnection
{
public:
	MySQLConnection();
	~MySQLConnection();

	bool Open(ConnectionInfo conInfo);
	void Close();

	bool ExecuteNonQuery(const char* sql);
	QueryResult* ExecuteQuery(const char* sql);

private:
	ConnectionInfo m_conInfo;
	MYSQL* m_mySqlHandle;
	
};

#endif