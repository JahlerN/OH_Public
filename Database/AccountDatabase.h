#ifndef ACCOUNTDATABASE_H
#define ACCOUNTDATABASE_H

#include "MySQLConnection.h"

class AccountDatabaseConnection : public MySQLConnection
{
public:
	AccountDatabaseConnection() : MySQLConnection() { }
};

extern AccountDatabaseConnection AccountDatabase;

#endif