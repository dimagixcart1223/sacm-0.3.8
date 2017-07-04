#include "../main.h"

extern CGame		 *pGame;
extern CChatWindow   *pChatWindow;
extern CCmdWindow	 *pCmdWindow;

extern CNetGame* pNetGame;

#define REJECT_REASON_BAD_VERSION   1
#define REJECT_REASON_BAD_NICKNAME  2
#define REJECT_REASON_BAD_MOD		3
#define REJECT_REASON_BAD_PLAYERID	4

void ProcessIncommingEvent(SACMPLAYER playerId, int iEventType, DWORD dwParam1, DWORD dwParam2, DWORD dwParam3);

//----------------------------------------------------
// Sent when a client joins the server we're
// currently connected to.

void ServerJoin(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	CHAR szPlayerName[MAX_PLAYER_NAME + 1];
	SACMPLAYER playerId;
	BYTE byteNameLen = 0;
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();

	bitStream->Read(playerId);
	bitStream->Read(byteNameLen);
	bitStream->Read(szPlayerName, byteNameLen);
	szPlayerName[byteNameLen] = '\0';

	// Add this client to the player pool.
	pPlayerPool->New(playerId, szPlayerName);

	pChatWindow->AddDebugMessage("%s joined", szPlayerName);
}

//----------------------------------------------------
// Sent when a client joins the server we're
// currently connected to.

void ServerQuit(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	SACMPLAYER playerId;
	BYTE byteReason;

	bitStream->Read(playerId);
	bitStream->Read(byteReason);

	// Delete this client from the player pool.
	pPlayerPool->Delete(playerId, byteReason);
}

//----------------------------------------------------
// Server is giving us basic init information.

extern int iNetModeIdleOnfootSendRate;
extern int iNetModeNormalOnfootSendRate;
extern int iNetModeIdleIncarSendRate;
extern int iNetModeNormalIncarSendRate;
extern int iNetModeFiringSendRate;
extern int iNetModeSendMultiplier;

void InitGame(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	CLocalPlayer *pLocalPlayer = NULL;

	if (pPlayerPool) pLocalPlayer = pPlayerPool->GetLocalPlayer();

	SACMPLAYER MyPlayerID;
	bool bLanMode, bStuntBonus;
	BYTE byteVehicleModels[212];

	bitStream->Read(pNetGame->m_iSpawnsAvailable);
	bitStream->Read(MyPlayerID);
	bitStream->Read(pNetGame->m_bShowPlayerTags);
	bitStream->Read(pNetGame->m_bShowPlayerMarkers);
	bitStream->Read(pNetGame->m_bTirePopping);
	bitStream->Read(pNetGame->m_byteWorldTime);
	bitStream->Read(pNetGame->m_byteWeather);
	bitStream->Read(pNetGame->m_fGravity);
	bitStream->Read(bLanMode);
	bitStream->Read((int)pNetGame->m_iDeathDropMoney);
	bitStream->Read(pNetGame->m_bInstagib);
	bitStream->Read(pNetGame->m_bZoneNames);
	bitStream->Read(pNetGame->m_bUseCJWalk);
	bitStream->Read(pNetGame->m_bAllowWeapons);
	bitStream->Read(pNetGame->m_bLimitGlobalChatRadius);
	bitStream->Read(pNetGame->m_fGlobalChatRadius);
	bitStream->Read(bStuntBonus);
	bitStream->Read(pNetGame->m_fNameTagDrawDistance);
	bitStream->Read(pNetGame->m_bDisableEnterExits);
	bitStream->Read(pNetGame->m_bNameTagLOS);
	pNetGame->m_bNameTagLOS = true;

	// Server's send rate restrictions
	bitStream->Read(iNetModeIdleOnfootSendRate);
	bitStream->Read(iNetModeNormalOnfootSendRate);
	bitStream->Read(iNetModeIdleIncarSendRate);
	bitStream->Read(iNetModeNormalIncarSendRate);
	bitStream->Read(iNetModeFiringSendRate);
	bitStream->Read(iNetModeSendMultiplier);

	BYTE byteStrLen;
	bitStream->Read(byteStrLen);
	if (byteStrLen) {
		memset(pNetGame->m_szHostName, 0, sizeof(pNetGame->m_szHostName));
		bitStream->Read(pNetGame->m_szHostName, byteStrLen);
	}
	pNetGame->m_szHostName[byteStrLen] = '\0';

	bitStream->Read((char *)&byteVehicleModels, 212);
	pGame->SetRequiredVehicleModels(byteVehicleModels);

	if (pPlayerPool) pPlayerPool->SetLocalPlayerID(MyPlayerID);
	pGame->EnableStuntBonus(bStuntBonus);
	if (bLanMode) pNetGame->SetLanMode(TRUE);

	pNetGame->InitGameLogic();

	// Set the gravity now
	pGame->SetGravity(pNetGame->m_fGravity);

	// Disable the enter/exits if needed.
	if (pNetGame->m_bDisableEnterExits) {
		pGame->ToggleEnterExits(true);
	}

	pNetGame->SetGameState(GAMESTATE_CONNECTED);
	if (pLocalPlayer) pLocalPlayer->HandleClassSelection();

	pChatWindow->AddDebugMessage("Connected to {FFFFFF}%.64s", pNetGame->m_szHostName);

}

//----------------------------------------------------
// Remote player has sent a chat message.

void Chat(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	SACMPLAYER playerId;
	BYTE byteTextLen;

	if (pNetGame->GetGameState() != GAMESTATE_CONNECTED)	return;

	char szText[256];
	memset(szText, 0, 256);

	bitStream->Read(playerId);
	bitStream->Read(byteTextLen);
	bitStream->Read(szText, byteTextLen);

#ifdef _DEBUG
	pChatWindow->AddDebugMessage("Chat(%u, %s)", playerId, szText);
#endif

	szText[byteTextLen] = '\0';

	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
	if (playerId == pPlayerPool->GetLocalPlayerID())
	{
		CLocalPlayer *pLocalPlayer = pPlayerPool->GetLocalPlayer();
		if (pLocalPlayer) {
			pChatWindow->AddChatMessage(pPlayerPool->GetLocalPlayerName(),
				pLocalPlayer->GetPlayerColorAsARGB(), (char*)szText);
		}
	}
	else {
		CRemotePlayer *pRemotePlayer = pPlayerPool->GetAt(playerId);
		if (pRemotePlayer) {
			pRemotePlayer->Say(szText);
		}
	}
}

//----------------------------------------------------
// Reply to our class request from the server.

void RequestClass(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	BYTE byteRequestOutcome = 0;
	PLAYER_SPAWN_INFO SpawnInfo;

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	CLocalPlayer *pPlayer = NULL;

	if (pPlayerPool) pPlayer = pPlayerPool->GetLocalPlayer();

	bitStream->Read(byteRequestOutcome);
	bitStream->Read((PCHAR)&SpawnInfo, sizeof(PLAYER_SPAWN_INFO));

	if (pPlayer) {
		if (byteRequestOutcome) {
			pPlayer->SetSpawnInfo(&SpawnInfo);
			pPlayer->HandleClassSelectionOutcome(TRUE);
		}
		else {
			pPlayer->HandleClassSelectionOutcome(FALSE);
		}
	}
}

//----------------------------------------------------
// The server has allowed us to spawn!

void RequestSpawn(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	BYTE byteRequestOutcome = 0;
	bitStream->Read(byteRequestOutcome);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	CLocalPlayer *pPlayer = NULL;
	if (pPlayerPool) pPlayer = pPlayerPool->GetLocalPlayer();

	if (pPlayer) {
		if (byteRequestOutcome == 2 || (byteRequestOutcome && pPlayer->m_bWaitingForSpawnRequestReply)) {
			pPlayer->Spawn();
		}
		else {
			pPlayer->m_bWaitingForSpawnRequestReply = false;
		}
	}
}

//----------------------------------------------------
// Add a physical ingame player for this remote player.

void WorldPlayerAdd(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	CRemotePlayer *pRemotePlayer;
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();

	SACMPLAYER playerId;
	PLAYER_SPAWN_INFO spawnInfo;
	DWORD dwColor;
	BYTE byteFightingStyle;
	BOOL bVisible;

	bitStream->Read(playerId);
	bitStream->Read((char *)&spawnInfo, sizeof(PLAYER_SPAWN_INFO));
	bitStream->Read(dwColor);
	bitStream->Read(byteFightingStyle);
	bitStream->Read(bVisible);

#ifdef _DEBUG
	pChatWindow->AddDebugMessage("WorldPlayerAdd(%u)", playerId);
#endif

	if (pPlayerPool) {
		pRemotePlayer = pPlayerPool->GetAt(playerId);
		if (pRemotePlayer) pRemotePlayer->Spawn(spawnInfo.byteTeam, spawnInfo.iSkin, &spawnInfo.vecPos, spawnInfo.fRotation, dwColor, byteFightingStyle, bVisible);
	}
}

//----------------------------------------------------
// Physical player is dead

void WorldPlayerDeath(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	SACMPLAYER playerId;
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	bitStream->Read(playerId);

#ifdef _DEBUG
	pChatWindow->AddDebugMessage("WorldPlayerDeath(%u)", playerId);
#endif

	if (pPlayerPool) {
		CRemotePlayer *pRemotePlayer = pPlayerPool->GetAt(playerId);
		if (pRemotePlayer) pRemotePlayer->HandleDeath();
	}
}

//----------------------------------------------------
// Physical player should be removed

void WorldPlayerRemove(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	SACMPLAYER playerId = 0;
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	bitStream->Read(playerId);

#ifdef _DEBUG
	pChatWindow->AddDebugMessage("WorldPlayerRemove(%u)", playerId);
#endif

	if (pPlayerPool) {
		CRemotePlayer *pRemotePlayer = pPlayerPool->GetAt(playerId);
		if (pRemotePlayer) pRemotePlayer->Remove();
	}
}

//----------------------------------------------------
// crashors. but much smoother model loading.

void LoadRequestedModelsThread(PVOID n)
{
	pGame->LoadRequestedModels();
	_endthread();
}

//----------------------------------------------------

void WorldVehicleAdd(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	if (!pVehiclePool) return;

	VEHICLE_SPAWN_INFO NewVehicle;

	bitStream->Read((char *)&NewVehicle, sizeof(VEHICLE_SPAWN_INFO));

#ifdef _DEBUG
	pChatWindow->AddDebugMessage("WorldVehicleAdd(%u)", NewVehicle.VehicleId);
#endif

	if (NewVehicle.iVehicleType < 400 || NewVehicle.iVehicleType > 611) return;

	//pGame->RequestModel(NewVehicle.iVehicleType);
	//_beginthread(LoadRequestedModelsThread,0,NULL); // <- leet crash CRenderer:ConstructRenderList
	//pGame->LoadRequestedModels();

	pVehiclePool->New(&NewVehicle);
}

//----------------------------------------------------

void WorldVehicleRemove(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	CPlayerPed *pPlayerPed = pGame->FindPlayerPed();
	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();

	if (!pVehiclePool) return;

	SACMVEHICLE VehicleID;
	SACMVEHICLE MyVehicleID;

	bitStream->Read(VehicleID);
	if (pPlayerPed) {

		MyVehicleID = pVehiclePool->FindIDFromGtaPtr(pPlayerPed->GetGtaVehicle());

#ifdef _DEBUG
		pChatWindow->AddDebugMessage("WorldVehicleRemove(%u)", VehicleID);
#endif

		if (MyVehicleID == VehicleID) {
			MATRIX4X4 mat;
			pPlayerPed->GetMatrix(&mat);
			pPlayerPed->RemoveFromVehicleAndPutAt(mat.pos.X, mat.pos.Y, mat.pos.Z);
		}
		pVehiclePool->Delete(VehicleID);
	}
}

//----------------------------------------------------

void DamageVehicle(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	SACMVEHICLE VehicleID;
	DWORD	dwPanels;
	DWORD	dwDoors;
	BYTE	byteLights;

	bitStream->Read(VehicleID);
	bitStream->Read(dwPanels);
	bitStream->Read(dwDoors);
	bitStream->Read(byteLights);

	CVehicle *pVehicle = pNetGame->GetVehiclePool()->GetAt(VehicleID);
	if (pVehicle) {
#ifdef _DEBUG
		pChatWindow->AddDebugMessage("UpdateDamageStatus(%u)", VehicleID);
#endif
		pVehicle->UpdateDamageStatus(dwPanels, dwDoors, byteLights);
	}
}

//----------------------------------------------------
// Remote client is trying to enter vehicle gracefully.

void EnterVehicle(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	SACMPLAYER playerId;
	SACMVEHICLE VehicleID = 0;
	BYTE bytePassenger = 0;
	BOOL bPassenger = FALSE;

	bitStream->Read(playerId);
	bitStream->Read(VehicleID);
	bitStream->Read(bytePassenger);

	if (bytePassenger) bPassenger = TRUE;

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();

	if (pPlayerPool) {
		CRemotePlayer *pRemotePlayer = pPlayerPool->GetAt(playerId);
		if (pRemotePlayer) {
			pRemotePlayer->EnterVehicle(VehicleID, bPassenger);
		}
	}

#ifdef _DEBUG
	pChatWindow->AddDebugMessage("Player(%u)::EnterVehicle(%u)", playerId, VehicleID);
#endif
}

//----------------------------------------------------
// Remote client is trying to exit vehicle gracefully.

void ExitVehicle(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	SACMPLAYER playerId;
	SACMVEHICLE VehicleID = 0;

	bitStream->Read(playerId);
	bitStream->Read(VehicleID);

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();

	if (pPlayerPool) {
		CRemotePlayer *pRemotePlayer = pPlayerPool->GetAt(playerId);
		if (pRemotePlayer) {
			pRemotePlayer->ExitVehicle();
		}
	}
#ifdef _DEBUG
	pChatWindow->AddDebugMessage("Player(%u)::ExitVehicle(%u)", playerId, VehicleID);
#endif
}

//----------------------------------------------------

void SetCheckpoint(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	float fX, fY, fZ, fSize;

	bitStream->Read(fX);
	bitStream->Read(fY);
	bitStream->Read(fZ);
	bitStream->Read(fSize);

	VECTOR Pos, Extent;

	Pos.X = fX;
	Pos.Y = fY;
	Pos.Z = fZ;
	Extent.X = fSize;
	Extent.Y = fSize;
	Extent.Z = fSize;

	pGame->SetCheckpointInformation(&Pos, &Extent);
	pGame->ToggleCheckpoints(TRUE);
}

//----------------------------------------------------

void DisableCheckpoint(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	pGame->ToggleCheckpoints(FALSE);
}

//----------------------------------------------------

void SetRaceCheckpoint(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	float fX, fY, fZ;
	BYTE byteType; //, byteSize;
	VECTOR Pos, Next;

	bitStream->Read(byteType);
	bitStream->Read(fX);
	bitStream->Read(fY);
	bitStream->Read(fZ);
	Pos.X = fX;
	Pos.Y = fY;
	Pos.Z = fZ;

	bitStream->Read(fX);
	bitStream->Read(fY);
	bitStream->Read(fZ);
	Next.X = fX;
	Next.Y = fY;
	Next.Z = fZ;

	bitStream->Read(fX);

	pGame->SetRaceCheckpointInformation(byteType, &Pos, &Next, fX);
	pGame->ToggleRaceCheckpoints(TRUE);
}

//----------------------------------------------------

void DisableRaceCheckpoint(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	pGame->ToggleRaceCheckpoints(FALSE);
}

//----------------------------------------------------

void UpdateScoresPingsIPs(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	SACMPLAYER playerId;
	int  iPlayerScore;
	int iPlayerPing;

	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();

	for (SACMPLAYER i = 0; i < MAX_PLAYERS; i++)
	{
		bitStream->Read(playerId);
		bitStream->Read(iPlayerScore);
		bitStream->Read(iPlayerPing);

		pPlayerPool->UpdateScore(playerId, iPlayerScore);
		pPlayerPool->UpdatePing(playerId, iPlayerPing);
	}
}

//----------------------------------------------------

void GameModeRestart(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	pNetGame->ShutdownForGameModeRestart();
}

//----------------------------------------------------

void ConnectionRejected(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	BYTE byteRejectReason;

	bitStream->Read(byteRejectReason);

	if (byteRejectReason == REJECT_REASON_BAD_VERSION) {
		pChatWindow->AddInfoMessage("CONNECTION REJECTED. INCORRECT SA:CM VERSION!");
	} else if (byteRejectReason == REJECT_REASON_BAD_NICKNAME) {
		pChatWindow->AddInfoMessage("CONNECTION REJECTED. BAD NICKNAME!");
		pChatWindow->AddInfoMessage("Please choose another nick between 3-16 characters");
		pChatWindow->AddInfoMessage("containing only A-Z a-z 0-9 [ ] or _");
		pChatWindow->AddInfoMessage("Use /quit to exit or press ESC and select Quit Game");
	} else if (byteRejectReason == REJECT_REASON_BAD_MOD) {
		pChatWindow->AddInfoMessage("CONNECTION REJECTED");
		pChatWindow->AddInfoMessage("YOUR'RE USING AN INCORRECT MOD!");
	} else if (byteRejectReason == REJECT_REASON_BAD_PLAYERID) {
		pChatWindow->AddInfoMessage("Connection was closed by the server.");
		pChatWindow->AddInfoMessage("Unable to allocate a player slot. Try again.");
	}

	pNetGame->GetRakClient()->Shutdown(500);
}

//----------------------------------------------------

void ClientMessage(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	DWORD dwStrLen;
	DWORD dwColor;

	bitStream->Read(dwColor);
	bitStream->Read(dwStrLen);
	char* szMsg = (char*)malloc(dwStrLen + 1);
	bitStream->Read(szMsg, dwStrLen);
	szMsg[dwStrLen] = 0;

	pChatWindow->AddClientMessage(dwColor, szMsg);

	free(szMsg);
}

//----------------------------------------------------

void WorldTime(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	BYTE byteWorldTime;
	bitStream->Read(byteWorldTime);
	pNetGame->m_byteWorldTime = byteWorldTime;
}

//----------------------------------------------------

void Pickup(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	PICKUP Pickup;
	int iIndex;

	bitStream->Read(iIndex);
	bitStream->Read((PCHAR)&Pickup, sizeof(PICKUP));

	CPickupPool *pPickupPool = pNetGame->GetPickupPool();
	if (pPickupPool) pPickupPool->New(&Pickup, iIndex);
}

//----------------------------------------------------

void DestroyPickup(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	int iIndex;

	bitStream->Read(iIndex);

	CPickupPool *pPickupPool = pNetGame->GetPickupPool();
	if (pPickupPool) pPickupPool->Destroy(iIndex);
}

//----------------------------------------------------

void DestroyWeaponPickup(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	BYTE byteIndex;

	bitStream->Read(byteIndex);

	CPickupPool *pPickupPool = pNetGame->GetPickupPool();
	if (pPickupPool) pPickupPool->DestroyDropped(byteIndex);
}

//----------------------------------------------------

void ScmEvent(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	SACMPLAYER playerId;
	int iEvent;
	DWORD dwParam1, dwParam2, dwParam3;

	bitStream->Read(playerId);
	bitStream->Read(iEvent);
	bitStream->Read(dwParam1);
	bitStream->Read(dwParam2);
	bitStream->Read(dwParam3);

	ProcessIncommingEvent(playerId, iEvent, dwParam1, dwParam2, dwParam3);
}

//----------------------------------------------------

void Weather(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	BYTE byteWeather;
	bitStream->Read(byteWeather);
	pNetGame->m_byteWeather = byteWeather;
}

//----------------------------------------------------

void SetTimeEx(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	BYTE byteHour;
	BYTE byteMinute;
	bitStream->Read(byteHour);
	bitStream->Read(byteMinute);
	//pNetGame->m_byteHoldTime = 0;
	pGame->SetWorldTime(byteHour, byteMinute);
	pNetGame->m_byteWorldTime = byteHour;
	pNetGame->m_byteWorldMinute = byteMinute;
}

//----------------------------------------------------

void ToggleClock(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	BYTE byteClock;
	bitStream->Read(byteClock);
	pGame->EnableClock(byteClock);
	if (byteClock)
	{
		pNetGame->m_byteHoldTime = 0;
	}
	else
	{
		pNetGame->m_byteHoldTime = 1;
		pGame->GetWorldTime((int*)&pNetGame->m_byteWorldTime, (int*)&pNetGame->m_byteWorldMinute);
	}
}

//----------------------------------------------------

void Instagib(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	bitStream->Read(pNetGame->m_bInstagib);
}

//----------------------------------------------------

void RegisterRPCs()
{
	pNetGame->GetRPC()->RegisterFunction(RPC_ServerJoin, ServerJoin);
	pNetGame->GetRPC()->RegisterFunction(RPC_ServerQuit, ServerQuit);
	pNetGame->GetRPC()->RegisterFunction(RPC_InitGame, InitGame);
	pNetGame->GetRPC()->RegisterFunction(RPC_Chat, Chat);
	pNetGame->GetRPC()->RegisterFunction(RPC_RequestClass, RequestClass);
	pNetGame->GetRPC()->RegisterFunction(RPC_RequestSpawn, RequestSpawn);
	pNetGame->GetRPC()->RegisterFunction(RPC_WorldPlayerAdd, WorldPlayerAdd);
	pNetGame->GetRPC()->RegisterFunction(RPC_WorldPlayerDeath, WorldPlayerDeath);
	pNetGame->GetRPC()->RegisterFunction(RPC_WorldPlayerRemove, WorldPlayerRemove);
	pNetGame->GetRPC()->RegisterFunction(RPC_WorldVehicleAdd, WorldVehicleAdd);
	pNetGame->GetRPC()->RegisterFunction(RPC_WorldVehicleRemove, WorldVehicleRemove);
	pNetGame->GetRPC()->RegisterFunction(RPC_DamageVehicle, DamageVehicle);
	pNetGame->GetRPC()->RegisterFunction(RPC_EnterVehicle, EnterVehicle);
	pNetGame->GetRPC()->RegisterFunction(RPC_ExitVehicle, ExitVehicle);
	pNetGame->GetRPC()->RegisterFunction(RPC_SetCheckpoint, SetCheckpoint);
	pNetGame->GetRPC()->RegisterFunction(RPC_DisableCheckpoint, DisableCheckpoint);
	pNetGame->GetRPC()->RegisterFunction(RPC_SetRaceCheckpoint, SetRaceCheckpoint);
	pNetGame->GetRPC()->RegisterFunction(RPC_DisableRaceCheckpoint, DisableRaceCheckpoint);
	pNetGame->GetRPC()->RegisterFunction(RPC_UpdateScoresPingsIPs, UpdateScoresPingsIPs);
	pNetGame->GetRPC()->RegisterFunction(RPC_GameModeRestart, GameModeRestart);
	pNetGame->GetRPC()->RegisterFunction(RPC_ConnectionRejected, ConnectionRejected);
	pNetGame->GetRPC()->RegisterFunction(RPC_ClientMessage, ClientMessage);
	pNetGame->GetRPC()->RegisterFunction(RPC_WorldTime, WorldTime);
	pNetGame->GetRPC()->RegisterFunction(RPC_Pickup, Pickup);
	pNetGame->GetRPC()->RegisterFunction(RPC_DestroyPickup, DestroyPickup);
	pNetGame->GetRPC()->RegisterFunction(RPC_DestroyWeaponPickup, DestroyWeaponPickup);
	pNetGame->GetRPC()->RegisterFunction(RPC_ScmEvent, ScmEvent);
	pNetGame->GetRPC()->RegisterFunction(RPC_Weather, Weather);
	pNetGame->GetRPC()->RegisterFunction(RPC_Instagib, Instagib);
	pNetGame->GetRPC()->RegisterFunction(RPC_SetTimeEx, SetTimeEx);
	pNetGame->GetRPC()->RegisterFunction(RPC_ToggleClock, ToggleClock);
}

//----------------------------------------------------

void UnRegisterRPCs()
{
	pNetGame->GetRPC()->UnregisterFunction(RPC_ServerJoin);
	pNetGame->GetRPC()->UnregisterFunction(RPC_ServerQuit);
	pNetGame->GetRPC()->UnregisterFunction(RPC_InitGame);
	pNetGame->GetRPC()->UnregisterFunction(RPC_Chat);
	pNetGame->GetRPC()->UnregisterFunction(RPC_RequestClass);
	pNetGame->GetRPC()->UnregisterFunction(RPC_RequestSpawn);
	pNetGame->GetRPC()->UnregisterFunction(RPC_WorldPlayerAdd);
	pNetGame->GetRPC()->UnregisterFunction(RPC_WorldPlayerDeath);
	pNetGame->GetRPC()->UnregisterFunction(RPC_WorldPlayerRemove);
	pNetGame->GetRPC()->UnregisterFunction(RPC_WorldVehicleAdd);
	pNetGame->GetRPC()->UnregisterFunction(RPC_WorldVehicleRemove);
	pNetGame->GetRPC()->UnregisterFunction(RPC_DamageVehicle);
	pNetGame->GetRPC()->UnregisterFunction(RPC_EnterVehicle);
	pNetGame->GetRPC()->UnregisterFunction(RPC_ExitVehicle);
	pNetGame->GetRPC()->UnregisterFunction(RPC_SetCheckpoint);
	pNetGame->GetRPC()->UnregisterFunction(RPC_DisableCheckpoint);
	pNetGame->GetRPC()->UnregisterFunction(RPC_SetRaceCheckpoint);
	pNetGame->GetRPC()->UnregisterFunction(RPC_DisableRaceCheckpoint);
	pNetGame->GetRPC()->UnregisterFunction(RPC_UpdateScoresPingsIPs);
	pNetGame->GetRPC()->UnregisterFunction(RPC_SvrStats);
	pNetGame->GetRPC()->UnregisterFunction(RPC_GameModeRestart);
	pNetGame->GetRPC()->UnregisterFunction(RPC_ConnectionRejected);
	pNetGame->GetRPC()->UnregisterFunction(RPC_ClientMessage);
	pNetGame->GetRPC()->UnregisterFunction(RPC_WorldTime);
	pNetGame->GetRPC()->UnregisterFunction(RPC_Pickup);
	pNetGame->GetRPC()->UnregisterFunction(RPC_DestroyPickup);
	pNetGame->GetRPC()->UnregisterFunction(RPC_DestroyWeaponPickup);
	pNetGame->GetRPC()->UnregisterFunction(RPC_ScmEvent);
	pNetGame->GetRPC()->UnregisterFunction(RPC_Weather);
	pNetGame->GetRPC()->UnregisterFunction(RPC_Instagib);
	pNetGame->GetRPC()->UnregisterFunction(RPC_SetTimeEx);
	pNetGame->GetRPC()->UnregisterFunction(RPC_ToggleClock);
}

//----------------------------------------------------
