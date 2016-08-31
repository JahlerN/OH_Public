#pragma once

#include <map>
#include "SocketMgr.h"
#include "OHSocket.h"

typedef std::map<uint16, OHSocket*> sessionMap;

template <class T>
class ClientSocketMgr : public OHSocketMgr<T>
{
	ClientSocketMgr<T>() {}

	virtual Socket* AssingSocket(SOCKET socket) { return NULL; }

	virtual ~ClientSocketMgr() {}
};