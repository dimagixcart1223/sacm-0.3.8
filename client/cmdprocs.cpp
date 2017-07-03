//----------------------------------------------------------
//
//   SA:CM Multiplayer Modification For GTA:SA
//   Copyright 2004-2006 SA:CM team
//
//----------------------------------------------------------

#include "main.h"
#include "game/game.h"
#include "game/util.h"
#include "game/task.h"

extern CGame		 *pGame;
extern CChatWindow   *pChatWindow;
extern CCmdWindow	 *pCmdWindow;
extern CDeathWindow	 *pDeathWindow;
extern CNetGame		 *pNetGame;
extern IDirect3DDevice9 *pD3DDevice;
extern GAME_SETTINGS tSettings;

extern BYTE	*pbyteCameraMode;
extern bool bShowDebugLabels;

CRemotePlayer *pTestPlayer;

VEHICLE_TYPE *pTrain;

int iCurrentPlayerTest=1;

extern float fFarClip;

//////////////////////////////////////////////////////
//
// -------R E L E A S E   C O M M A N D S--------
//
// (INCLUDES SCRIPTING UTILS)
//
//////////////////////////////////////////////////////

void cmdDefaultCmdProc(PCHAR szCmd)
{
	if(pNetGame) {
		CLocalPlayer *pLocalPlayer;
		pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
		pLocalPlayer->Say(szCmd);
	}
}

//----------------------------------------------------

extern BOOL gDisableAllFog;

BOOL bDontProcessVehiclePool=FALSE;

//----------------------------------------------------

void cmdFpsLimit(PCHAR szCmd)
{
	if (!strlen(szCmd)) return;

	int iLimit = atoi(szCmd);
	if (iLimit < 20 || iLimit > 100)
	{
		pChatWindow->AddDebugMessage("FrameLimiter: valid amounts are 20-100");
		return;
	}

	UnFuck(0xC1704C,1);
	*(PDWORD)0xC1704C = iLimit;

	pChatWindow->AddDebugMessage("FrameLimiter: %u", iLimit);
}

//----------------------------------------------------

void cmdQuit(PCHAR szCmd)
{
	QuitGame();
}

//----------------------------------------------------

void cmdRcon(PCHAR szCmd)
{
	if (!szCmd) return;

	BYTE bytePacketId = ID_RCON_COMMAND;
	RakNet::BitStream bsCommand;
	bsCommand.Write(bytePacketId);
	DWORD dwCmdLen = (DWORD)strlen(szCmd);
	bsCommand.Write(dwCmdLen);
	bsCommand.Write(szCmd, dwCmdLen);
	pRakClient->Send(&bsCommand, HIGH_PRIORITY, RELIABLE, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
}

//----------------------------------------------------

void cmdSavePos(PCHAR szCmd)
{
	CPlayerPed *pPlayer = pGame->FindPlayerPed();
	FILE *fileOut;
	DWORD dwVehicleID;
	float fZAngle;

	//if(!tSettings.bDebug) return;

	fileOut = fopen("savedpositions.txt","a");
	if(!fileOut) {
		pChatWindow->AddDebugMessage("I can't open the savepositions.txt file for append.");
		return;
	}

	// incar savepos

	if(pPlayer->IsInVehicle()) {

		VEHICLE_TYPE *pVehicle = pPlayer->GetGtaVehicle();
	
		dwVehicleID = GamePool_Vehicle_GetIndex(pVehicle);
		ScriptCommand(&get_car_z_angle,dwVehicleID,&fZAngle);

		fprintf(fileOut,"AddStaticVehicle(%u,%.4f,%.4f,%.4f,%.4f,%u,%u); // %s\n",
			pVehicle->entity.nModelIndex,pVehicle->entity.mat->pos.X,pVehicle->entity.mat->pos.Y,pVehicle->entity.mat->pos.Z,
			fZAngle,pVehicle->byteColor1,pVehicle->byteColor2,szCmd);

		fclose(fileOut);

		pChatWindow->AddDebugMessage("-> InCar pos saved");

		return;
	}

	// onfoot savepos

	PED_TYPE *pActor = pPlayer->GetGtaActor();
	ScriptCommand(&get_actor_z_angle,pPlayer->m_dwGTAId,&fZAngle);

	fprintf(fileOut,"AddPlayerClass(%u,%.4f,%.4f,%.4f,%.4f,0,0,0,0,0,0); // %s\n",pPlayer->GetModelIndex(),
		pActor->entity.mat->pos.X,pActor->entity.mat->pos.Y,pActor->entity.mat->pos.Z,fZAngle,szCmd);

	fclose(fileOut);

	pChatWindow->AddDebugMessage("-> OnFoot pos saved");
}

//----------------------------------------------------

void cmdRawSavePos(PCHAR szCmd)
{
	CPlayerPed *pPlayer = pGame->FindPlayerPed();
	FILE *fileOut;
	DWORD dwVehicleID;
	float fZAngle;

	if(pPlayer->IsInVehicle()) {

		fileOut = fopen("rawvehicles.txt","a");
		if(!fileOut) {
			pChatWindow->AddDebugMessage("I can't open the rawvehicles.txt file for append.");
			return;
		}

		VEHICLE_TYPE *pVehicle = pPlayer->GetGtaVehicle();
	
		dwVehicleID = GamePool_Vehicle_GetIndex(pVehicle);
		ScriptCommand(&get_car_z_angle,dwVehicleID,&fZAngle);

		fprintf(fileOut,"%u,%.4f,%.4f,%.4f,%.4f,%u,%u ; %s\n",
			pVehicle->entity.nModelIndex,pVehicle->entity.mat->pos.X,pVehicle->entity.mat->pos.Y,pVehicle->entity.mat->pos.Z,
			fZAngle,pVehicle->byteColor1,pVehicle->byteColor2,szCmd);

		fclose(fileOut);

        pChatWindow->AddDebugMessage("-> InCar pos saved");
		return;
	}

	// onfoot savepos

	PED_TYPE *pActor = pPlayer->GetGtaActor();
	ScriptCommand(&get_actor_z_angle,pPlayer->m_dwGTAId,&fZAngle);
	
	fileOut = fopen("rawpositions.txt","a");

	if(!fileOut) {
		pChatWindow->AddDebugMessage("I can't open the rawvehicles.txt file for append.");
		return;
	}

	fprintf(fileOut,"%.4f,%.4f,%.4f,%.4f ; %s\n",pActor->entity.mat->pos.X,pActor->entity.mat->pos.Y,pActor->entity.mat->pos.Z,fZAngle,szCmd);
	fclose(fileOut);

	pChatWindow->AddDebugMessage("-> OnFoot pos saved");
}

//----------------------------------------------------

void cmdPlayerSkin(PCHAR szCmd)
{
#ifndef _DEBUG
	if(!tSettings.bDebug) return;
#endif

	if(!strlen(szCmd)){	
		pChatWindow->AddDebugMessage("Usage: player_skin (skin number).");
		return;
	}
	int iPlayerSkin = atoi(szCmd);

	if(pGame->IsGameLoaded())
	{
		CPlayerPed *pPlayer = pGame->FindPlayerPed();

		if(pPlayer)
		{
			pPlayer->SetModelIndex(iPlayerSkin);
		}		
	}
	return;
}

//----------------------------------------------------

void cmdCreateVehicle(PCHAR szCmd)
{
	if(!tSettings.bDebug) return;

	if(!strlen(szCmd)){
		pChatWindow->AddDebugMessage("Usage: /v (vehicle id).");
		return;
	}	
	int iVehicleType = atoi(szCmd);

	if(pGame->IsGameLoaded())
	{
		pGame->RequestModel(iVehicleType);
		pGame->LoadRequestedModels();

		// place this actor near the player.
		CPlayerPed *pPlayer = pGame->FindPlayerPed();

		if(pPlayer) 
		{
			MATRIX4X4 matPlayer;
			pPlayer->GetMatrix(&matPlayer);
			CHAR blank[9] = "";
			sprintf(blank, "TYPE_%d", iVehicleType);
			CVehicle *pTestVehicle = pGame->NewVehicle(iVehicleType,
				(matPlayer.pos.X - 5.0f), (matPlayer.pos.Y - 5.0f),
				matPlayer.pos.Z+1.0f, 0.0f, (PCHAR)blank);

			pTestVehicle->Add();

			return;
		}
		else {
			pChatWindow->AddDebugMessage("I couldn't find the player actor.");
			return;
		}
	}
	else {
		pChatWindow->AddDebugMessage("Game is not loaded.");
	}
}

//----------------------------------------------------

void cmdDebugLabels(PCHAR szCmd)
{
	bShowDebugLabels = !bShowDebugLabels;
}

//-----------------------------------------------------

void SetupCommands()
{
	// RELEASE COMMANDS
	pCmdWindow->AddDefaultCmdProc(cmdDefaultCmdProc);
	pCmdWindow->AddCmdProc("quit",cmdQuit);
	pCmdWindow->AddCmdProc("q",cmdQuit);
	pCmdWindow->AddCmdProc("save",cmdSavePos);
	pCmdWindow->AddCmdProc("rs",cmdRawSavePos);
	pCmdWindow->AddCmdProc("rcon",cmdRcon);
	pCmdWindow->AddCmdProc("fpslimit",cmdFpsLimit);
	pCmdWindow->AddCmdProc("dl", cmdDebugLabels);

#ifndef _DEBUG
	if (tSettings.bDebug)
	{
#endif
		pCmdWindow->AddCmdProc("vehicle",cmdCreateVehicle);
		pCmdWindow->AddCmdProc("skin",cmdPlayerSkin);
#ifndef _DEBUG
	}
#endif
}

//----------------------------------------------------
