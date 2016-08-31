#include "stdafx.h"
#include "..\Database\AccountDatabase.h"
#include "..\Database\GameDatabase.h"

GameServer* g_main;
GameDatabaseConnection GameDatabase;
AccountDatabaseConnection AccountDatabase;

void MainLoop()
{
	uint32 curTime = 0;
	uint32 prevTime = GetMSTime();

	uint32 prevSleepTime = 0;

	while (g_main->IsRunning())
	{
		curTime = GetMSTime();

		uint32 diff = 0;
		if (prevTime > curTime)
			diff = (0xFFFFFFFF - prevTime) + curTime;
		else
			diff = curTime - prevTime;

		g_main->Update(diff);
		prevTime = curTime;

		if (diff <= 50 + prevSleepTime)
		{
			prevSleepTime = 50 + prevSleepTime - diff;

			std::this_thread::sleep_for(std::chrono::milliseconds(prevSleepTime));
		}
		else
			prevSleepTime = 0;
	}
}

int main()
{
	SetConsoleTitle(TEXT(string_format("OpenHero Game Server v.%.2f", GAMESERVER_VERSION).c_str()));

	printf("Setting up server..\n");

	g_main = new GameServer();

	if (g_main->Startup())
	{
		printf("Server successfully started!\n\n");

		MainLoop();

		return 0;
	}

	printf("Server setup failed.");
	system("pause");

    return 1;
}