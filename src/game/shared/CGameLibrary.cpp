/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#include "cbase.h"

#include "CGameLibrary.h"

#include "config/GameConfigLoader.h"

#include "scripting/AS/CASManager.h"

#include "utils/command_utils.h"
#include "utils/GameSystem.h"
#include "utils/json_utils.h"

bool CGameLibrary::InitializeCommon()
{
	if (!FileSystem_LoadFileSystem())
	{
		Con_Printf("Could not load filesystem library\n");
		return false;
	}

	g_pDeveloper = g_ConCommands.GetCVar("developer");

	//Done separately before game systems to avoid tightly coupling logging to json.
	g_Logging.PreInitialize();

	//Logging must initialize after JSON so that it can load the config file immediately.
	//It must also initialize after concommands since it creates a few of those.
	//It is safe to use loggers after logging has shut down so having it shut down first is not an issue.
	g_GameSystems.Add(&g_ConCommands);
	g_GameSystems.Add(&g_JSON);
	g_GameSystems.Add(&g_Logging);

	if (!g_GameSystems.Initialize())
	{
		return false;
	}

	g_GameSystems.Invoke(&IGameSystem::PostInitialize);

	if (!g_ASManager.Initialize())
	{
		Con_Printf("Could not initialize Angelscript engine manager\n");
		return false;
	}

	//Depends on Angelscript
	if (!g_GameConfigLoader.Initialize())
	{
		Con_Printf("Could not initialize game configuration loader\n");
		return false;
	}

	return true;
}

void CGameLibrary::ShutdownCommon()
{
	g_GameConfigLoader.Shutdown();
	g_ASManager.Shutdown();

	g_GameSystems.InvokeReverse(&IGameSystem::Shutdown);

	g_GameSystems.Remove(&g_Logging);
	g_GameSystems.Remove(&g_ConCommands);
	g_GameSystems.Remove(&g_JSON);

	FileSystem_FreeFileSystem();
}
