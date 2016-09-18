#include "stdafx.h"
//#include "../SharedFiles/tstring.h"
#include "mysql.h"
#include "mysql_connection.h"

MySQLConnection::MySQLConnection()
{
	/*m_conInfo.m_dbName = conInfo.m_dbName;
	m_conInfo.m_username = conInfo.m_username;
	m_conInfo.m_password = conInfo.m_password;*/

	m_mySqlHandle = NULL;
}

MySQLConnection::~MySQLConnection()
{
	Close();
}

void MySQLConnection::Close()
{
	if (m_mySqlHandle != NULL)
	{
		mysql_close(m_mySqlHandle);
		m_mySqlHandle == nullptr;
	}
}

bool MySQLConnection::Open(ConnectionInfo conInfo)
{
	memcpy(&m_conInfo, &conInfo, sizeof(ConnectionInfo));
	m_mySqlHandle = mysql_init(NULL);
	mysql_real_connect(m_mySqlHandle, "localhost", m_conInfo.m_username.c_str(), m_conInfo.m_password.c_str(), m_conInfo.m_dbName.c_str(), 3306, NULL, 0);

	return m_mySqlHandle != NULL;
}

bool MySQLConnection::ExecuteNonQuery(const char* sql)
{
	if (m_mySqlHandle == NULL)
		return false;

	if (mysql_query(m_mySqlHandle, sql))
	{
		PrintErrorAndAttemptReconnect();
		return false;
	}

	return true;
}

QueryResult MySQLConnection::ExecuteQuery(const char* sql)
{
	if (!sql || m_mySqlHandle == NULL)
		return QueryResult(NULL, NULL, 0, 0);

	MYSQL_RES* result = NULL;
	MYSQL_FIELD* fields = NULL;
	uint64 rowCount = 0;
	uint32 fieldCount = 0;

	if (mysql_query(m_mySqlHandle, sql))
	{
		PrintErrorAndAttemptReconnect();
		return QueryResult(NULL, NULL, 0, 0);
	}

	result = mysql_store_result(m_mySqlHandle);
	rowCount = mysql_affected_rows(m_mySqlHandle);
	fieldCount = mysql_field_count(m_mySqlHandle);

	if (result == NULL)
		return QueryResult(NULL, NULL, 0, 0);

	if (rowCount <= 0)
	{
		mysql_free_result(result);
		return QueryResult(NULL, NULL, 0, 0);
	}
	fields = mysql_fetch_fields(result);

	return QueryResult(result, fields, rowCount, fieldCount);
}

bool MySQLConnection::PrintErrorAndAttemptReconnect()
{
	uint32 err = mysql_errno(m_mySqlHandle);
	std::string errMsg = mysql_error(m_mySqlHandle);

	printf("SQL Error %d: %s.\n\n Attempting reconnection, closing server if it fails.", err, errMsg.c_str());
	if (!Open(m_conInfo))
		std::abort();

	return true;
}