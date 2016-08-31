#pragma once
#include "Winsock2.h"
#include "windows.h"
#include <cassert>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>

#ifdef _DEBUG
#define ASSERT assert
#define TRACE printf
#else
#define ASSERT
#define TRACE
#endif

#include "types.h"

#include "OHSocket.h"
#include "OHSocketMgr.h"
#include "STLMap.h"
#include "Ini.h"