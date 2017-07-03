/*

	SA:CM Multiplayer Modification
	Copyright 2004-2005 SA:CM Team

	Version: $Id: netrpc.cpp,v 1.52 2006/06/02 13:24:21 mike Exp $

*/

#include "main.h"
#include "vehmods.h"

extern CNetGame *pNetGame;

// Removed for RakNet upgrade
//#define REGISTER_STATIC_RPC REGISTER_AS_REMOTE_PROCEDURE_CALL
//#define UNREGISTER_STATIC_RPC UNREGISTER_AS_REMOTE_PROCEDURE_CALL

#define REJECT_REASON_BAD_VERSION	1
#define REJECT_REASON_BAD_NICKNAME	2
#define REJECT_REASON_BAD_MOD		3
#define REJECT_REASON_BAD_PLAYERID	4

bool ContainsInvalidNickChars(char * szString);
void ReplaceBadChars(char * szString);

//----------------------------------------------------
// Sent by a client who's wishing to join us in our
// multiplayer-like activities.

void ClientJoin(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	RakNet::BitStream bsReject;
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();

	CHAR szPlayerName[256];
	BYTE bytePlayerID;
	int  iVersion;
	BYTE byteNickLen;
	BYTE byteRejectReason;
	unsigned int uiChallengeResponse = 0;

	bytePlayerID = packet->guid.systemIndex;
	SACMPLAYER MyPlayerID = bytePlayerID;

	memset(szPlayerName, 0, 256);

	bitStream->Read(iVersion);
	bitStream->Read(byteNickLen);
	bitStream->Read(szPlayerName, byteNickLen);
	szPlayerName[byteNickLen] = '\0';

	if (iVersion != NETGAME_VERSION) {
		byteRejectReason = REJECT_REASON_BAD_VERSION;
		bsReject.Write(byteRejectReason);
		pNetGame->SendRPC(RPC_ConnectionRejected, &bsReject, bytePlayerID, FALSE);
		pRakServer->CloseConnection(packet->guid, true);
		return;
	}

	if (ContainsInvalidNickChars(szPlayerName) ||
		byteNickLen < 3 || byteNickLen > 16 || pPlayerPool->IsNickInUse(szPlayerName)) {
		byteRejectReason = REJECT_REASON_BAD_NICKNAME;
		bsReject.Write(byteRejectReason);
		pNetGame->SendRPC(RPC_ConnectionRejected, &bsReject, bytePlayerID, FALSE);
		pRakServer->CloseConnection(packet->guid, true);
		return;
	}

	// Add this client to the player pool.
	if (!pPlayerPool->New(bytePlayerID, szPlayerName)) {
		pRakServer->CloseConnection(packet->guid, true);
		return;
	}

	pNetGame->ProcessClientJoin(bytePlayerID);
}

//----------------------------------------------------
// Sent by client with global chat text

void Chat(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	SACMPLAYER sender = packet->guid.systemIndex;

	unsigned char szText[256];
	memset(szText, 0, 256);

	BYTE byteTextLen;

	CPlayerPool *pPool = pNetGame->GetPlayerPool();

	bitStream->Read(byteTextLen);

	if (byteTextLen > MAX_CMD_INPUT) return;

	bitStream->Read((char *)szText, byteTextLen);
	szText[byteTextLen] = '\0';

	if (!pPool->GetSlotState(sender)) return;

	ReplaceBadChars((char *)szText);

	logprintf("[chat] [%s]: %s", pPool->GetPlayerName(sender), szText);

	BYTE bytePlayerID = sender;

	CPlayer *pPlayer = pNetGame->GetPlayerPool()->GetAt(bytePlayerID);
	CGameMode *pGameMode = pNetGame->GetGameMode();

	if (pPlayer)
	{
		// Send OnPlayerText callback to the GameMode script.
		if (pNetGame->GetFilterScripts()->OnPlayerText((cell)bytePlayerID, szText)) {
			if (pGameMode)
			{
				// Comment by spookie:
				//   The CPlayer::Say() call has moved to CGameMode::OnPlayerText(),
				//   when a gamemode is available. This is due to filter scripts.
				pGameMode->OnPlayerText((cell)bytePlayerID, szText);
			}
			else {
				// No pGameMode
				pPlayer->Say(szText, byteTextLen);
			}
		}
	}
}

//----------------------------------------------------
// Sent by client who wishes to request a class from
// the gamelogic handler.

void RequestClass(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	SACMPLAYER sender = packet->guid.systemIndex;

	if (pNetGame->GetGameState() != GAMESTATE_RUNNING) return;
	if (!pNetGame->GetPlayerPool()->GetSlotState(sender)) return;

	int iRequestedClass = 1;
	BYTE byteRequestOutcome = 0;
	bitStream->Read(iRequestedClass);

	CPlayer *pPlayer = pNetGame->GetPlayerPool()->GetAt(sender);
	if (pPlayer && (iRequestedClass <= (pNetGame->m_iSpawnsAvailable - 1)))
	{
		pPlayer->SetSpawnInfo(&pNetGame->m_AvailableSpawns[iRequestedClass]);
		//logprintf("SetSpawnInfo - iSkin = %d", pNetGame->m_AvailableSpawns[iRequestedClass].iSkin);
	}

	pNetGame->GetFilterScripts()->OnPlayerRequestClass((cell)sender, (cell)iRequestedClass);
	byteRequestOutcome = 1;
	if (pNetGame->GetGameMode()) {
		byteRequestOutcome = pNetGame->GetGameMode()->OnPlayerRequestClass((cell)sender, (cell)iRequestedClass);
	}

	RakNet::BitStream bsClassRequestReply;
	bsClassRequestReply.Write(byteRequestOutcome);

	PLAYER_SPAWN_INFO *pSpawnInfo = pPlayer->GetSpawnInfo();
	bsClassRequestReply.Write((char*)pSpawnInfo, sizeof(PLAYER_SPAWN_INFO));

	pNetGame->SendRPC(RPC_RequestClass, &bsClassRequestReply, sender, FALSE);
}

//----------------------------------------------------
// Client wants to spawn

void RequestSpawn(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	SACMPLAYER sender = packet->guid.systemIndex;

	BYTE bytePlayerID = sender;
	BYTE byteRequestOutcome = 1;

	if (pNetGame->GetGameState() != GAMESTATE_RUNNING) return;
	if (!pNetGame->GetPlayerPool()->GetSlotState(sender)) return;

	if (!pNetGame->GetFilterScripts()->OnPlayerRequestSpawn((cell)bytePlayerID)) byteRequestOutcome = 0;
	if (pNetGame->GetGameMode() && byteRequestOutcome) {
		if (!pNetGame->GetGameMode()->OnPlayerRequestSpawn((cell)bytePlayerID)) byteRequestOutcome = 0;
	}

	RakNet::BitStream bsSpawnRequestReply;
	bsSpawnRequestReply.Write(byteRequestOutcome);
	pNetGame->SendRPC(RPC_RequestSpawn, &bsSpawnRequestReply, sender, FALSE);
}


//----------------------------------------------------
// Sent by client when they're spawning/respawning.

void Spawn(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	SACMPLAYER sender = packet->guid.systemIndex;

	if (pNetGame->GetGameState() != GAMESTATE_RUNNING) return;

	SACMPLAYER bytePlayerID = sender;
	if (!pNetGame->GetPlayerPool()->GetSlotState(bytePlayerID)) return;
	CPlayer	*pPlayer = pNetGame->GetPlayerPool()->GetAt(bytePlayerID);

	// More sanity checks for crashers.
	if (!pPlayer->m_bHasSpawnInfo) return;
	int iSpawnClass = pPlayer->m_SpawnInfo.iSkin;
	if (iSpawnClass < 0 || iSpawnClass > 300) return;

	pPlayer->Spawn();
}

//----------------------------------------------------
// Sent by client when they die.

void Death(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	SACMPLAYER sender = packet->guid.systemIndex;

	if (pNetGame->GetGameState() != GAMESTATE_RUNNING) return;

	SACMPLAYER bytePlayerID = sender;
	if (!pNetGame->GetPlayerPool()->GetSlotState(bytePlayerID)) return;
	CPlayer	*pPlayer = pNetGame->GetPlayerPool()->GetAt(bytePlayerID);

	BYTE byteDeathReason;
	SACMPLAYER byteWhoWasResponsible;

	bitStream->Read(byteDeathReason);
	bitStream->Read(byteWhoWasResponsible);

	if (pPlayer) {
		pPlayer->HandleDeath(byteDeathReason, byteWhoWasResponsible);
	}
}

//----------------------------------------------------
// Sent by client when they want to enter a
// vehicle gracefully.

void EnterVehicle(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	SACMPLAYER sender = packet->guid.systemIndex;

	if (pNetGame->GetGameState() != GAMESTATE_RUNNING) return;
	if (!pNetGame->GetPlayerPool()->GetSlotState(sender)) return;

	SACMPLAYER bytePlayerID = sender;
	CPlayer	*pPlayer = pNetGame->GetPlayerPool()->GetAt(bytePlayerID);

	SACMVEHICLE VehicleID = 0;
	BYTE bytePassenger = 0;

	bitStream->Read(VehicleID);
	bitStream->Read(bytePassenger);

	if (pPlayer) {
		if (VehicleID == 0xFFFF) {
			pNetGame->KickPlayer(bytePlayerID);
			return;
		}
		pPlayer->EnterVehicle(VehicleID, bytePassenger);
	}
}

//----------------------------------------------------
// Sent by client when they want to exit a
// vehicle gracefully.

void ExitVehicle(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	SACMPLAYER sender = packet->guid.systemIndex;

	if (pNetGame->GetGameState() != GAMESTATE_RUNNING) return;
	if (!pNetGame->GetPlayerPool()->GetSlotState(sender)) return;

	SACMPLAYER bytePlayerID = sender;
	CPlayer	*pPlayer = pNetGame->GetPlayerPool()->GetAt(bytePlayerID);

	SACMVEHICLE VehicleID;
	bitStream->Read(VehicleID);

	if (pPlayer) {
		if (VehicleID == 0xFFFF) {
			pNetGame->KickPlayer(bytePlayerID);
			return;
		}
		pPlayer->ExitVehicle(VehicleID);
	}

	// HACK by spookie - this gonna cause probs, or are they defo out of the car now?
	// comment by kyeman - No they're not, it's just an advisory for the anims.
	//pNetGame->GetVehiclePool()->GetAt(byteVehicleID)->m_byteDriverID = INVALID_ID;

	//logprintf("%u exits vehicle %u",bytePlayerID,byteVehicleID);
}

//----------------------------------------------------

void ServerCommand(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	SACMPLAYER sender = packet->guid.systemIndex;
	int iStrLen = 0;
	unsigned char* szCommand = NULL;

	if (pNetGame->GetGameState() != GAMESTATE_RUNNING) return;

	BYTE bytePlayerID = sender;

	bitStream->Read(iStrLen);

	if (iStrLen < 1) return;
	if (iStrLen > MAX_CMD_INPUT) return;

	szCommand = (unsigned char*)calloc(iStrLen + 1, 1);
	bitStream->Read((char*)szCommand, iStrLen);
	szCommand[iStrLen] = '\0';

	ReplaceBadChars((char *)szCommand);

	if (!pNetGame->GetFilterScripts()->OnPlayerCommandText(bytePlayerID, szCommand))
	{
		if (pNetGame->GetGameMode())
		{
			if (!pNetGame->GetGameMode()->OnPlayerCommandText(bytePlayerID, szCommand))
			{
				pNetGame->SendClientMessage(sender, 0xFFFFFFFF, "SERVER: Unknown command.");
			}
		}
	}

	free(szCommand);
}

//----------------------------------------------------

void UpdateScoresPingsIPs(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	SACMPLAYER sender = packet->guid.systemIndex;

	/*RakNet::BitStream bsParams;
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	BYTE bytePlayerId = sender;

	if (!pPlayerPool->GetSlotState(bytePlayerId)) return;

	for (BYTE i = 0; i < MAX_PLAYERS; i++)
	{
		if (pPlayerPool->GetSlotState(i))
		{
			bsParams.Write(i);
			bsParams.Write((DWORD)pPlayerPool->GetPlayerScore(i));
			bsParams.Write((DWORD)pRakServer->GetLastPing(pRakServer->GetSystemAddressFromIndex(i)));
		}
	}

	pNetGame->SendRPC(RPC_UpdateScoresPingsIPs, &bsParams, sender, FALSE);*/
}

//----------------------------------------------------

void SetInteriorId(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	SACMPLAYER sender = packet->guid.systemIndex;

	BYTE byteInteriorId;
	bitStream->Read(byteInteriorId);

	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
	BYTE bytePlayerId = sender;

	if (pPlayerPool->GetSlotState(bytePlayerId))
	{
		CGameMode *pGameMode = pNetGame->GetGameMode();
		CFilterScripts *pFilters = pNetGame->GetFilterScripts();

		CPlayer *pPlayer = pPlayerPool->GetAt(bytePlayerId);
		int iOldInteriorId = pPlayer->m_iInteriorId;
		pPlayer->m_iInteriorId = (int)byteInteriorId;

		if (pGameMode) pGameMode->OnPlayerInteriorChange(
			bytePlayerId, pPlayer->m_iInteriorId, iOldInteriorId);

		if (pFilters) pFilters->OnPlayerInteriorChange(
			bytePlayerId, pPlayer->m_iInteriorId, iOldInteriorId);
	}
}

//----------------------------------------------------

void ScmEvent(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	SACMPLAYER sender = packet->guid.systemIndex;

	RakNet::BitStream bsSend;
	SACMPLAYER bytePlayerID;
	int iEvent;
	DWORD dwParams1;
	DWORD dwParams2;
	DWORD dwParams3;

	bytePlayerID = sender;
	bitStream->Read(iEvent);
	bitStream->Read(dwParams1);
	bitStream->Read(dwParams2);
	bitStream->Read(dwParams3);

	BOOL bSend = TRUE;

	//printf("ScmEvent: %u %u %u %u\n",iEvent,dwParams1,dwParams2,dwParams3);

	if (pNetGame->GetGameState() != GAMESTATE_RUNNING) return;

	if (iEvent == EVENT_TYPE_CARCOMPONENT)
	{
		CVehicle*	pVehicle = pNetGame->GetVehiclePool()->GetAt((SACMVEHICLE)dwParams1);
		if (!pVehicle) return;

		BYTE*	pDataStart = (BYTE*)&pVehicle->m_CarModInfo.byteCarMod[0];
		for (int i = 0; i < 14; i++)
		{
			DWORD data = pDataStart[i];

			if (data == 0)
			{
				if (!pNetGame->GetGameMode()->OnVehicleMod(bytePlayerID, dwParams1, dwParams2) ||
					!pNetGame->GetFilterScripts()->OnVehicleMod(bytePlayerID, dwParams1, dwParams2))
				{
					bSend = FALSE;
				}

				if (bSend)
				{
					BYTE byteMod = (BYTE)(dwParams2 - 1000);

					if (byteMod >= sizeof(c_byteVehMods) ||
						(c_byteVehMods[byteMod] != NOV &&
							c_byteVehMods[byteMod] != (BYTE)(pVehicle->m_SpawnInfo.iVehicleType - 400))) {
						bSend = FALSE;
					}
					else {
						pDataStart[i] = byteMod;
					}
				}
				break;
			}
		}
		if (bSend)
		{
			bsSend.Write(bytePlayerID);
			bsSend.Write(iEvent);
			bsSend.Write(dwParams1);
			bsSend.Write(dwParams2);
			bsSend.Write(dwParams3);
			pNetGame->SendRPC(RPC_ScmEvent, &bsSend, sender, TRUE);
		}
		else
		{
			bsSend.Write((SACMVEHICLE)dwParams1);
			bsSend.Write(dwParams2);
			pNetGame->SendRPC(RPC_ScrRemoveComponent, &bsSend, sender, FALSE);
		}
	}
	else if (iEvent == EVENT_TYPE_PAINTJOB)
	{
		CVehicle*	pVehicle = pNetGame->GetVehiclePool()->GetAt((SACMVEHICLE)dwParams1);
		if (!pVehicle) return;

		if (!pNetGame->GetGameMode()->OnVehiclePaintjob(bytePlayerID, dwParams1, dwParams2) ||
			!pNetGame->GetFilterScripts()->OnVehiclePaintjob(bytePlayerID, dwParams1, dwParams2)) bSend = FALSE;
		if (bSend)
		{
			pVehicle->m_CarModInfo.bytePaintJob = (BYTE)dwParams2;

			bsSend.Write(bytePlayerID);
			bsSend.Write(iEvent);
			bsSend.Write(dwParams1);
			bsSend.Write(dwParams2);
			bsSend.Write(dwParams3);
			pNetGame->SendRPC(RPC_ScmEvent, &bsSend, sender, TRUE);
		}
	}
	else if (iEvent == EVENT_TYPE_CARCOLOR)
	{
		CVehicle*	pVehicle = pNetGame->GetVehiclePool()->GetAt((SACMVEHICLE)dwParams1);
		if (!pVehicle)
			return;

		if (!pNetGame->GetGameMode()->OnVehicleRespray(bytePlayerID, dwParams1, dwParams2, dwParams3) ||
			!pNetGame->GetFilterScripts()->OnVehicleRespray(bytePlayerID, dwParams1, dwParams2, dwParams3)) bSend = FALSE;
		if (bSend)
		{
			pVehicle->m_CarModInfo.iColor0 = (int)dwParams2;
			pVehicle->m_CarModInfo.iColor1 = (int)dwParams3;

			bsSend.Write(bytePlayerID);
			bsSend.Write(iEvent);
			bsSend.Write(dwParams1);
			bsSend.Write(dwParams2);
			bsSend.Write(dwParams3);
			pNetGame->SendRPC(RPC_ScmEvent, &bsSend, sender, TRUE);
		}
	}
	else
	{
		bsSend.Write(bytePlayerID);
		bsSend.Write(iEvent);
		bsSend.Write(dwParams1);
		bsSend.Write(dwParams2);
		bsSend.Write(dwParams3);
		pNetGame->SendRPC(RPC_ScmEvent, &bsSend, sender, TRUE);
	}
}

//----------------------------------------------------
// Sent by client when their vehicle damage
// model has changed.

void DamageVehicle(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	SACMPLAYER sender = packet->guid.systemIndex;

	SACMVEHICLE DamageVehicleUpdating;
	DWORD dwLastPanelDamageStatus;
	DWORD dwLastDoorDamageStatus;
	BYTE byteLastLightsDamageStatus;

	bitStream->Read(DamageVehicleUpdating);
	bitStream->Read(dwLastPanelDamageStatus);
	bitStream->Read(dwLastDoorDamageStatus);
	bitStream->Read(byteLastLightsDamageStatus);

	CVehicle* pVehicle = pNetGame->GetVehiclePool()->GetAt(DamageVehicleUpdating);
	if (pVehicle) {
		pVehicle->UpdateDamageStatus(dwLastPanelDamageStatus, dwLastDoorDamageStatus, byteLastLightsDamageStatus);
		pVehicle->SetHasHadUpdate(true);

		RakNet::BitStream bsVehicleDamageUpdate;
		bsVehicleDamageUpdate.Write(DamageVehicleUpdating);
		bsVehicleDamageUpdate.Write(dwLastPanelDamageStatus);
		bsVehicleDamageUpdate.Write(dwLastDoorDamageStatus);
		bsVehicleDamageUpdate.Write(byteLastLightsDamageStatus);
		pNetGame->SendRPC(RPC_DamageVehicle, &bsVehicleDamageUpdate, sender, TRUE);
	}
}

void AdminMapTeleport(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	SACMPLAYER sender = packet->guid.systemIndex;

	VECTOR vecPos;
	bitStream->Read(vecPos.X);
	bitStream->Read(vecPos.Y);
	bitStream->Read(vecPos.Z);

	BYTE bytePlayerId = sender;
	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();

	if (!pNetGame) return;
	if (pNetGame->GetGameState() != GAMESTATE_RUNNING) return;
	if (!pNetGame->m_bAdminTeleport) return;

	if (pPlayerPool->GetSlotState(bytePlayerId)) {
		CPlayer *pPlayer = pPlayerPool->GetAt(bytePlayerId);
		if (pPlayer && pPlayer->m_bCanTeleport && pPlayerPool->IsAdmin(bytePlayerId))
		{
			RakNet::BitStream bsParams;
			bsParams.Write(vecPos.X);	// X
			bsParams.Write(vecPos.Y);	// Y
			bsParams.Write(vecPos.Z);	// Z

			pNetGame->SendRPC(RPC_ScrSetPlayerPos, &bsParams, sender, FALSE);
		}
	}
}

void VehicleDestroyed(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	SACMPLAYER sender = packet->guid.systemIndex;

	SACMVEHICLE VehicleID;
	bitStream->Read(VehicleID);

	if (!pNetGame) return;
	if (pNetGame->GetGameState() != GAMESTATE_RUNNING) return;

	CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
	CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
	if (!pPlayerPool || !pVehiclePool) return;

	BYTE bytePlayerId = sender;
	if (!pPlayerPool->GetSlotState(bytePlayerId)) return;

	if (pVehiclePool->GetSlotState(VehicleID))
	{
		CVehicle* pVehicle = pVehiclePool->GetAt(VehicleID);
		if (pVehicle) pVehicle->SetDead();
	}
}

void PickedUpWeapon(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	SACMPLAYER sender = packet->guid.systemIndex;

	SACMPLAYER bytePlayerID;
	bitStream->Read(bytePlayerID);

	RakNet::BitStream bsSend;
	bsSend.Write(bytePlayerID);

	pNetGame->SendRPC(RPC_DestroyWeaponPickup, &bsSend, -1, TRUE);
}

void PickedUpPickup(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	BYTE bytePlayerID = packet->guid.systemIndex;

	int iPickup;
	bitStream->Read(iPickup);

	CGameMode *pGameMode = pNetGame->GetGameMode();
	CFilterScripts *pFilters = pNetGame->GetFilterScripts();

	if (pGameMode) pGameMode->OnPlayerPickedUpPickup(bytePlayerID, iPickup);
	if (pFilters) pFilters->OnPlayerPickedUpPickup(bytePlayerID, iPickup);
}

void MenuSelect(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	BYTE bytePlayerID = packet->guid.systemIndex;

	BYTE byteRow;
	bitStream->Read(byteRow);

	CGameMode *pGameMode = pNetGame->GetGameMode();
	CFilterScripts *pFilters = pNetGame->GetFilterScripts();

	if (pGameMode) pGameMode->OnPlayerSelectedMenuRow(bytePlayerID, byteRow);
	if (pFilters) pFilters->OnPlayerSelectedMenuRow(bytePlayerID, byteRow);
}

void MenuQuit(RakNet::BitStream *bitStream, RakNet::Packet *packet)
{
	BYTE bytePlayerID = packet->guid.systemIndex;

	CGameMode *pGameMode = pNetGame->GetGameMode();
	CFilterScripts *pFilters = pNetGame->GetFilterScripts();

	if (pGameMode) pGameMode->OnPlayerExitedMenu(bytePlayerID);
	if (pFilters) pFilters->OnPlayerExitedMenu(bytePlayerID);
}

//----------------------------------------------------

void RegisterRPCs()
{
	pRPC4Plugin->RegisterFunction(RPC_ClientJoin, ClientJoin);
	pRPC4Plugin->RegisterFunction(RPC_Chat, Chat);
	pRPC4Plugin->RegisterFunction(RPC_RequestClass, RequestClass);
	pRPC4Plugin->RegisterFunction(RPC_RequestSpawn, RequestSpawn);
	pRPC4Plugin->RegisterFunction(RPC_Spawn, Spawn);
	pRPC4Plugin->RegisterFunction(RPC_Death, Death);
	pRPC4Plugin->RegisterFunction(RPC_EnterVehicle, EnterVehicle);
	pRPC4Plugin->RegisterFunction(RPC_ExitVehicle, ExitVehicle);
	pRPC4Plugin->RegisterFunction(RPC_ServerCommand, ServerCommand);
	//pRPC4Plugin->RegisterFunction(RPC_UpdateScoresPingsIPs, UpdateScoresPingsIPs);
	pRPC4Plugin->RegisterFunction(RPC_SetInteriorId, SetInteriorId);
	pRPC4Plugin->RegisterFunction(RPC_ScmEvent, ScmEvent);
	pRPC4Plugin->RegisterFunction(RPC_AdminMapTeleport, AdminMapTeleport);
	pRPC4Plugin->RegisterFunction(RPC_VehicleDestroyed, VehicleDestroyed);
	pRPC4Plugin->RegisterFunction(RPC_PickedUpWeapon, PickedUpWeapon);
	pRPC4Plugin->RegisterFunction(RPC_PickedUpPickup, PickedUpPickup);
	pRPC4Plugin->RegisterFunction(RPC_MenuSelect, MenuSelect);
	pRPC4Plugin->RegisterFunction(RPC_MenuQuit, MenuQuit);
	pRPC4Plugin->RegisterFunction(RPC_DamageVehicle, DamageVehicle);
}

//----------------------------------------------------

void UnRegisterRPCs()
{
	pRPC4Plugin->UnregisterFunction(RPC_ClientJoin);
	pRPC4Plugin->UnregisterFunction(RPC_Chat);
	pRPC4Plugin->UnregisterFunction(RPC_RequestClass);
	pRPC4Plugin->UnregisterFunction(RPC_RequestSpawn);
	pRPC4Plugin->UnregisterFunction(RPC_Spawn);
	pRPC4Plugin->UnregisterFunction(RPC_Death);
	pRPC4Plugin->UnregisterFunction(RPC_EnterVehicle);
	pRPC4Plugin->UnregisterFunction(RPC_ExitVehicle);
	pRPC4Plugin->UnregisterFunction(RPC_ServerCommand);
	//pRPC4Plugin->UnregisterFunction(RPC_UpdateScoresPingsIPs);
	pRPC4Plugin->UnregisterFunction(RPC_SetInteriorId);
	pRPC4Plugin->UnregisterFunction(RPC_ScmEvent);
	pRPC4Plugin->UnregisterFunction(RPC_AdminMapTeleport);
	pRPC4Plugin->UnregisterFunction(RPC_VehicleDestroyed);
	pRPC4Plugin->UnregisterFunction(RPC_PickedUpWeapon);
	pRPC4Plugin->UnregisterFunction(RPC_PickedUpPickup);
	pRPC4Plugin->UnregisterFunction(RPC_MenuSelect);
	pRPC4Plugin->UnregisterFunction(RPC_MenuQuit);
	pRPC4Plugin->UnregisterFunction(RPC_DamageVehicle);
}

//----------------------------------------------------
