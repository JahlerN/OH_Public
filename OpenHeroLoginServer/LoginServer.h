#pragma once
#include "define.h"
#include "LoginSession.h"
#include "..\SharedFiles\OHSocketMgr.h"

#define MAX_SERVER_PER_TAB = 10

class LoginSession;
class LoginServer
{
	friend class CDBProcess;
public:
	LoginServer();
	~LoginServer();

	inline short GetVersion() { return m_version; }
	__forceinline ServerInfoList* GetServerList() { return &m_serverList; }

	bool Startup();
	void MainLoop();
	
	bool ReloadIniSettings();//TODO: Implement this, nice to be able to reload the config anytime.

	bool m_running;

	CDBProcess m_DBProcess;

	static OHSocketMgr<LoginSession> s_socketMgr;

private:
	void LoadIniSettings();
	ServerInfoList m_serverList;
	std::string m_tabName1;

	CIni m_ini;

	short m_version;

	char m_ODBCName[32], m_ODBCLogin[32], m_ODBCPw[32];
};

extern LoginServer* g_main;