// OpenHeroLoginServer.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"

LoginServer* g_main;

int main()
{
	SetConsoleTitle(TEXT(string_format("OpenHero Login Server v.%.2f", LOGINSERVER_VERSION).c_str()));
	
	printf("Setting up server..");

	g_main = new LoginServer();

	if (g_main->Startup())
	{
		printf("\nServer started successfully!\n");

		g_main->MainLoop();
	}
	else
	{
		
	}
    return 0;
}

