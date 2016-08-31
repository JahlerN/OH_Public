#include "stdafx.h"
#include "OdbcConnection.h"

OdbcConnection::OdbcConnection()
	: m_connHandle(NULL), m_envHandle(NULL), m_bMarsEnabled(false)
{
}

bool OdbcConnection::Connect(tstring szDSN, tstring szUser, tstring szPass, bool bMarsEnabled /*= true*/)
{
	m_szDSN = szDSN;
	m_szUser = szUser;
	m_szPass = szPass;
	m_bMarsEnabled = bMarsEnabled;

	return Connect();
}

bool OdbcConnection::Connect()
{
	if (m_szDSN.length() == 0)
		return false;

	m_lock.Acquire();

	tstring szConn = _T("DSN=") + m_szDSN + _T(";");
	// Reconnect if we need to.
	if (isConnected())
		Disconnect();

	// Allocate enviroment handle
	if (!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_envHandle)))
	{
		ReportSQLError(SQL_HANDLE_ENV, m_envHandle, _T("SQLAllocHandle"), _T("Unable to allocate environment handle."));
		goto error_handler;
	}

	// Request ODBC3 support
	if (!SQL_SUCCEEDED(SQLSetEnvAttr(m_envHandle, SQL_ATTR_ODBC_VERSION, (void *)SQL_OV_ODBC3, 0)))
	{
		ReportSQLError(SQL_HANDLE_ENV, m_envHandle, _T("SQLSetEnvAttr"), _T("Unable to set environment attribute (SQL_ATTR_ODBC_VERSION)."));
		goto error_handler;
	}

	// Allocate the connection handle
	if (!SQL_SUCCEEDED(SQLAllocHandle(SQL_HANDLE_DBC, m_envHandle, &m_connHandle)))
	{
		ReportSQLError(SQL_HANDLE_ENV, m_envHandle, _T("SQLAllocHandle"), _T("Unable to allocate connection handle."));
		goto error_handler;
	}

	if (m_szUser.length())
	{
		szConn += _T("UID=") + m_szUser + _T(";");
		if (m_szPass.length())
			szConn += _T("PWD=") + m_szPass + _T(";");
	}

	// Enable multiple active result sets
	//if (m_bMarsEnabled)
		//szConn += _T("MARS_Connection=yes;"); //TODO: Using mysql so there's no MARS feature.

	if (!SQL_SUCCEEDED(SQLDriverConnect(m_connHandle, NULL, (SQLTCHAR *)szConn.c_str(), SQL_NTS, NULL, NULL, NULL, NULL)))
	{
		ReportSQLError(SQL_HANDLE_DBC, m_connHandle, _T("SQLDriverConnect"), _T("Unable to establish connection."));
		goto error_handler;
	}

	for (auto itr = m_commandSet.begin(); itr != m_commandSet.end(); itr++)
		(*itr)->SetConnectionHandle(m_connHandle);

	m_lock.Release();
	return true;

error_handler:
	ResetHandles();
	m_lock.Release();
	return false;
}

OdbcCommand *OdbcConnection::CreateCommand()
{
	if (!isConnected()
		&& !Connect())
		return NULL;

	return new OdbcCommand(this);
}

void OdbcConnection::AddCommand(OdbcCommand *dbCommand)
{
	m_lock.Acquire();
	m_commandSet.insert(dbCommand);
	m_lock.Release();
}

void OdbcConnection::RemoveCommand(OdbcCommand *dbCommand)
{
	m_lock.Acquire();
	m_commandSet.erase(dbCommand);
	m_lock.Release();
}

// Used to internally reset handles. Should ONLY be used in special cases, otherwise we'll break the state of the connection.
void OdbcConnection::ResetHandles()
{
	// Free the connection handle if it's allocated
	if (m_connHandle != NULL)
	{
		SQLFreeHandle(SQL_HANDLE_DBC, m_connHandle);
		m_connHandle = NULL;
	}

	// Free the environment handle if it's allocated
	if (m_envHandle != NULL)
	{
		SQLFreeHandle(SQL_HANDLE_ENV, m_envHandle);
		m_envHandle = NULL;
	}
}

tstring OdbcConnection::ReportSQLError(SQLSMALLINT handleType, SQLHANDLE handle,
	TCHAR *szSource, TCHAR *szError, ...)
{
	tstring szErrorMessage;
	TCHAR szErrorBuffer[256];
	OdbcError *error = new OdbcError();

	va_list args;
	va_start(args, szError);
	_vsntprintf_s(szErrorBuffer, sizeof(szErrorBuffer), sizeof(szErrorBuffer), szError, args);
	va_end(args);

	error->Source = szSource;
	error->ErrorMessage = szErrorBuffer;

	m_odbcErrors.push_back(error);

	if (handle != NULL)
	{
		error->ExtendedErrorMessage = GetSQLError(handleType, handle);
		return error->ExtendedErrorMessage;
	}

	return szErrorMessage;
}

tstring OdbcConnection::GetSQLError(SQLSMALLINT handleType, SQLHANDLE handle)
{
	tstring result;
	SQLTCHAR SqlState[256], SqlMessage[256];
	SQLINTEGER NativeError;
	SQLSMALLINT TextLength;

	if (!SQL_SUCCEEDED(SQLGetDiagRec(handleType, handle, 1, (SQLTCHAR *)&SqlState, &NativeError, (SQLTCHAR *)&SqlMessage, sizeof(SqlMessage), &TextLength)))
		return result;

	result = (TCHAR *)SqlMessage;
	return result;
}

OdbcError *OdbcConnection::GetError()
{
	if (!isError())
		return NULL;

	m_lock.Acquire();
	OdbcError *pError = m_odbcErrors.back();
	m_odbcErrors.pop_back();
	m_lock.Release();

	return pError;
}

void OdbcConnection::ResetErrors()
{
	if (!isError())
		return;

	m_lock.Acquire();
	OdbcError *pError;
	while ((pError = GetError()) != NULL)
		delete pError;
	m_lock.Release();
}

void OdbcConnection::Disconnect()
{
	// Make sure our handles are open. If not, there's nothing to do.
	if (!isConnected())
		return;

	m_lock.Acquire();
	// Kill off open statements
	if (m_commandSet.size())
	{
		for (auto itr = m_commandSet.begin(); itr != m_commandSet.end(); itr++)
		{
			// Detach from the connection first so we don't try to remove it from the set (while we're using it!)
			(*itr)->Detach();

			// Now free it.
			delete (*itr);
		}

		m_commandSet.clear();
	}

	// Close the connection to the server & reset our handles
	Close();
	m_lock.Release();
}

void OdbcConnection::Close()
{
	// Disconnect from server.
	SQLDisconnect(m_connHandle);

	// Reset handles
	ResetHandles();
}

OdbcConnection::~OdbcConnection()
{
	Disconnect();
	ResetErrors();
}