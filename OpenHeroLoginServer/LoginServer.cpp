#include "stdafx.h"

OHSocketMgr<LoginSession> LoginServer::s_socketMgr;

LoginServer::LoginServer() : m_ini("LoginServerConf.ini"), m_version(1)
{
	memset(m_ODBCName, 0, sizeof(m_ODBCName));
	memset(m_ODBCLogin, 0, sizeof(m_ODBCLogin));
	memset(m_ODBCPw, 0, sizeof(m_ODBCPw));
}

bool LoginServer::Startup()
{
	LoadIniSettings();

	if (!m_DBProcess.Connect(m_ODBCName, m_ODBCLogin, m_ODBCPw))
	{
		printf("Unable to connect to DB with the details located in LoginServerConf.ini.\n");
		return false;
	}

	printf("Successfully connected to database.\n");

	printf("Loading server information.\n");

	if (!m_DBProcess.LoadServerInformation(m_serverList))
		return false;

	InitPacketHandlers();

	if (!s_socketMgr.Listen(_LISTEN_PORT, MAX_USERS))
	{
		printf("Unable to start listening on server port %u.\n", _LISTEN_PORT);
		return false;
	}

	s_socketMgr.RunServer();

	return true;
}

void LoginServer::MainLoop()
{
	m_running = true;

	while (m_running)
	{
		Sleep(5000);//TODO: This is temp for now, the only thing we actually do here is update the server info.
		//m_DBProcess.UpdateServerInfo(m_serverList);//TODO: For some fucking reason, using this function crashes everything basicly, it start dropping the connection to mysql etc..
	}
}

void LoginServer::LoadIniSettings()
{
	m_ini.GetString("ODBC", "DBN", "OHDB,", m_ODBCName, sizeof(m_ODBCName), false);
	m_ini.GetString("ODBC", "UID", "OpenHeroLogin,", m_ODBCLogin, sizeof(m_ODBCLogin), false);
	m_ini.GetString("ODBC", "UPW", "root,", m_ODBCPw, sizeof(m_ODBCPw), false);
}

LoginServer::~LoginServer()
{

}