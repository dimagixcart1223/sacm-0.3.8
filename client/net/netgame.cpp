#include "../main.h"
#include "../game/util.h"
#include "../mod.h"

extern CGame		 *pGame;
extern CChatWindow   *pChatWindow;
extern CCmdWindow	 *pCmdWindow;

int iExceptMessageDisplayed = 0;

int iVehiclesBench = 0;
int iPlayersBench = 0;
int iPicksupsBench = 0;
int iMenuBench = 0;
int iObjectBench = 0;
int iTextDrawBench = 0;

int iVehiclePoolProcessFlag = 0;
int iPickupPoolProcessFlag = 0;
extern int iSyncedRandomNumber;

//----------------------------------------------------

CNetGame::CNetGame(PCHAR szHostOrIp, int iPort, PCHAR szPlayerName, PCHAR szPass)
{
	strcpy(m_szHostName, "San Andreas Cooperative Mode");
	strncpy(m_szHostOrIp, szHostOrIp, sizeof(m_szHostOrIp));
	m_iPort = iPort;
	strncpy(m_szPassword, szPass, sizeof(m_szPassword));

	// Setup player pool
	m_pPlayerPool = new CPlayerPool();
	m_pPlayerPool->SetLocalPlayerName(szPlayerName);

	m_pVehiclePool = new CVehiclePool();
	m_pPickupPool = new CPickupPool();
	m_pObjectPool = new CObjectPool();
	m_pMenuPool = new CMenuPool();
	m_pTextDrawPool = new CTextDrawPool();
	m_pGangZonePool = new CGangZonePool();
	m_pActorPool = new CActorPool();

	pRakClient = RakNet::RakPeerInterface::GetInstance();
	pRPC4Plugin = RakNet::RPC4::GetInstance();
	pRakClient->AttachPlugin(pRPC4Plugin);

	RegisterRPCs();
	RegisterScriptRPCs();

	pRakClient->Startup(1, &RakNet::SocketDescriptor(), 1);

	m_dwLastConnectAttempt = GetTickCount();
	m_iGameState = GAMESTATE_WAIT_CONNECT;

	m_iSpawnsAvailable = 0;
	m_byteWorldTime = 12;
	m_byteWorldMinute = 0;
	m_byteWeather = 10;
	m_fGravity = (float)0.008000000;
	m_iDeathDropMoney = 0;
	m_bLanMode = FALSE;
	m_bNameTagLOS = true;
	m_byteHoldTime = 1;
	m_bUseCJWalk = FALSE;
	m_bDisableEnterExits = false;
	m_fNameTagDrawDistance = 70.0f;

	for (int i = 0; i < 32; i++) {
		m_dwMapIcon[i] = NULL;
	}

	m_byteFriendlyFire = 1;
	pGame->EnableClock(0); // Hide the clock by default
	pGame->EnableZoneNames(0);
	m_bZoneNames = FALSE;
	m_bInstagib = FALSE;
	m_iCheckLoadedStuff = 0;

	if (pChatWindow) pChatWindow->AddDebugMessage("SA:CM {FFFFFF}" SACM_VERSION "{A9C4E4} Started");
}

//----------------------------------------------------

CNetGame::~CNetGame()
{
	pRakClient->Shutdown(0);
	UnRegisterRPCs();
	UnRegisterScriptRPCs();	// Unregister server-side scripting RPCs.
	SAFE_DELETE(m_pPlayerPool);
	SAFE_DELETE(m_pVehiclePool);
	SAFE_DELETE(m_pPickupPool);
	SAFE_DELETE(m_pObjectPool);
	SAFE_DELETE(m_pMenuPool);
	SAFE_DELETE(m_pTextDrawPool);
	SAFE_DELETE(m_pGangZonePool);
}

//----------------------------------------------------

void CNetGame::ShutdownForGameModeRestart()
{
	m_byteWorldTime = 12;
	m_byteWorldMinute = 0;
	m_byteWeather = 10;
	m_byteHoldTime = 1;
	m_bNameTagLOS = true;
	m_bUseCJWalk = FALSE;
	m_fGravity = (float)0.008000000;
	m_iDeathDropMoney = 0;
	pGame->ToggleEnterExits(true);
	pGame->SetGravity(m_fGravity);
	pGame->SetWantedLevel(0);
	pGame->EnableClock(0);
	m_bDisableEnterExits = false;
	m_fNameTagDrawDistance = 70.0f;

	for (SACMPLAYER playerId = 0; playerId < MAX_PLAYERS; playerId++) {
		CRemotePlayer* pPlayer = m_pPlayerPool->GetAt(playerId);
		if (pPlayer) {
			pPlayer->SetTeam(NO_TEAM);
			pPlayer->ResetAllSyncAttributes();
		}
	}

	CLocalPlayer *pLocalPlayer = m_pPlayerPool->GetLocalPlayer();
	if (pLocalPlayer) {
		pLocalPlayer->ResetAllSyncAttributes();
		pLocalPlayer->ToggleSpectating(FALSE);
	}

	m_iGameState = GAMESTATE_RESTARTING;

	pChatWindow->AddInfoMessage("Game mode restarting..");

	// Disable the ingame players and reset the vehicle pool.
	m_pPlayerPool->DeactivateAll();

	// Process the pool one last time
	m_pPlayerPool->Process();

	ResetVehiclePool();
	ResetPickupPool();
	ResetObjectPool();
	ResetMenuPool();
	ResetTextDrawPool();
	ResetGangZonePool();
	ResetActorPool();
	ResetMapIcons();

	CPlayerPed *pPlayerPed = pGame->FindPlayerPed();
	if (pPlayerPed) {
		pPlayerPed->SetInterior(0);
		pPlayerPed->SetDead();
		pPlayerPed->SetArmour(0.0f);
	}

	pGame->ToggleCheckpoints(FALSE);
	pGame->ToggleRaceCheckpoints(FALSE);
	pGame->ResetLocalMoney();
	pGame->EnableZoneNames(0);
	m_bZoneNames = FALSE;

	GameResetRadarColors();
}

//----------------------------------------------------

void CNetGame::InitGameLogic()
{
	//GameResetRadarColors();

	m_WorldBounds[0] = 20000.0f;
	m_WorldBounds[1] = -20000.0f;
	m_WorldBounds[2] = 20000.0f;
	m_WorldBounds[3] = -20000.0f;
}

//----------------------------------------------------

void CNetGame::Process()
{
	UpdateNetwork();

	if (m_byteHoldTime) {
		pGame->SetWorldTime(m_byteWorldTime, m_byteWorldMinute);
	}

	// Keep the weather fixed at m_byteWeather so it doesnt desync
	pGame->SetWorldWeather(m_byteWeather);

	// KEEP THE FOLLOWING ANIMS LOADED DURING THE NETGAME
	if (!pGame->IsAnimationLoaded("PARACHUTE")) pGame->RequestAnimation("PARACHUTE");
	//
	if (!pGame->IsAnimationLoaded("BAR")) pGame->RequestAnimation("BAR");
	if (!pGame->IsAnimationLoaded("GANGS")) pGame->RequestAnimation("GANGS");
	if (!pGame->IsAnimationLoaded("PAULNMAC")) pGame->RequestAnimation("PAULNMAC");
	//
	//if(!pGame->IsAnimationLoaded("DANCING")) pGame->RequestAnimation("DANCING");
	//if(!pGame->IsAnimationLoaded("GFUNK")) pGame->RequestAnimation("GFUNK");
	//if(!pGame->IsAnimationLoaded("RUNNINGMAN"))	pGame->RequestAnimation("RUNNINGMAN");
	//if(!pGame->IsAnimationLoaded("WOP")) pGame->RequestAnimation("WOP");
	//if(!pGame->IsAnimationLoaded("STRIP")) pGame->RequestAnimation("STRIP");

	if (!pGame->IsModelLoaded(OBJECT_PARACHUTE)) {
		pGame->RequestModel(OBJECT_PARACHUTE);
	}

	// keep the throwable weapon models loaded
	if (!pGame->IsModelLoaded(WEAPON_MODEL_TEARGAS))
		pGame->RequestModel(WEAPON_MODEL_TEARGAS);
	if (!pGame->IsModelLoaded(WEAPON_MODEL_GRENADE))
		pGame->RequestModel(WEAPON_MODEL_GRENADE);
	if (!pGame->IsModelLoaded(WEAPON_MODEL_MOLTOV))
		pGame->RequestModel(WEAPON_MODEL_MOLTOV);

	// cellphone
	if (!pGame->IsModelLoaded(330)) pGame->RequestModel(330);

	if (GetGameState() == GAMESTATE_CONNECTED) {

		DWORD dwStartTick = GetTickCount();

		if (m_pPlayerPool) m_pPlayerPool->Process();
		if (m_pActorPool) m_pActorPool->Process();

		iPlayersBench += GetTickCount() - dwStartTick;

		if (m_pVehiclePool && iVehiclePoolProcessFlag > 5) {
			dwStartTick = GetTickCount();

			try { m_pVehiclePool->Process(); }
			catch (...) {
				if (!iExceptMessageDisplayed) {
					pChatWindow->AddDebugMessage("Warning: Error processing vehicle pool");
					iExceptMessageDisplayed++;
				}
			}
			iVehiclesBench += GetTickCount() - dwStartTick;
			iVehiclePoolProcessFlag = 0;
		}
		else {
			iVehiclePoolProcessFlag++;
		}

		if (m_pPickupPool && iPickupPoolProcessFlag > 10) {

			dwStartTick = GetTickCount();

			try { m_pPickupPool->Process(); }
			catch (...) {
				if (!iExceptMessageDisplayed) {
					pChatWindow->AddDebugMessage("Warning: Error processing pickup pool");
					iExceptMessageDisplayed++;
				}
			}
			iPicksupsBench += GetTickCount() - dwStartTick;
			iPickupPoolProcessFlag = 0;
		}
		else
		{
			iPickupPoolProcessFlag++;
		}

		if (m_pObjectPool) {
			dwStartTick = GetTickCount();
			try { m_pObjectPool->Process(); }
			catch (...) {
				if (!iExceptMessageDisplayed) {
					pChatWindow->AddDebugMessage("Warning: Error processing object pool");
					iExceptMessageDisplayed++;
				}
			}
			iObjectBench += GetTickCount() - dwStartTick;
		}

		if (m_pMenuPool) {
			dwStartTick = GetTickCount();
			try { m_pMenuPool->Process(); }
			catch (...) {
				if (!iExceptMessageDisplayed) {
					pChatWindow->AddDebugMessage("Warning: Error processing menu pool");
					iExceptMessageDisplayed++;
				}
			}
			iMenuBench += GetTickCount() - dwStartTick;
		}
	}
	else {
		CPlayerPed *pPlayer = pGame->FindPlayerPed();
		CCamera *pCamera = pGame->GetCamera();

		if (pPlayer && pCamera) {
			if (pPlayer->IsInVehicle()) {
				pPlayer->RemoveFromVehicleAndPutAt(1500.0f, -887.0979f, 32.56055f);
			}
			else {
				pPlayer->TeleportTo(1500.0f, -887.0979f, 32.56055f);
			}

			pCamera->SetPosition(1093.0f, -2036.0f, 90.0f, 0.0f, 0.0f, 0.0f);
			pCamera->LookAtPoint(384.0f, -1557.0f, 20.0f, 2);
			pGame->SetWorldWeather(1);

			pGame->DisplayHud(FALSE);
		}
	}

	if (GetGameState() == GAMESTATE_WAIT_CONNECT &&
		(GetTickCount() - m_dwLastConnectAttempt) > 3000)
	{
		if (pChatWindow) pChatWindow->AddDebugMessage("Connecting to {FFFFFF}%s:%d{A9C4E4}...", m_szHostOrIp, m_iPort);
		pRakClient->Connect(m_szHostOrIp, m_iPort, m_szPassword, strlen(m_szPassword));
		m_dwLastConnectAttempt = GetTickCount();
		SetGameState(GAMESTATE_CONNECTING);
	}
}

//----------------------------------------------------
// UPDATE NETWORK
//----------------------------------------------------

void CNetGame::UpdateNetwork()
{
	RakNet::Packet* pkt = NULL;
	while (pkt = GetRakClient()->Receive()) {
		switch (pkt->data[0]) {
		case ID_INCOMPATIBLE_PROTOCOL_VERSION:
			Packet_IncompatibleProtocolVersion(pkt);
		case ID_CONNECTION_BANNED:
			Packet_ConnectionBanned(pkt);
			break;
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			Packet_NoFreeIncomingConnections(pkt);
			break;
		case ID_DISCONNECTION_NOTIFICATION:
			Packet_DisconnectionNotification(pkt);
			break;
		case ID_CONNECTION_LOST:
			Packet_ConnectionLost(pkt);
			break;
		case ID_INVALID_PASSWORD:
			Packet_InvalidPassword(pkt);
			break;
		case ID_CONNECTION_ATTEMPT_FAILED:
			Packet_ConnectAttemptFailed(pkt);
			break;
		case ID_CONNECTION_REQUEST_ACCEPTED:
			Packet_ConnectionSucceeded(pkt);
			break;
		case ID_PLAYER_SYNC:
			Packet_PlayerSync(pkt);
			break;
		case ID_VEHICLE_SYNC:
			Packet_VehicleSync(pkt);
			break;
		case ID_PASSENGER_SYNC:
			Packet_PassengerSync(pkt);
			break;
		case ID_AIM_SYNC:
			Packet_AimSync(pkt);
			break;
		case ID_TRAILER_SYNC:
			Packet_TrailerSync(pkt);
			break;
		case ID_UNOCCUPIED_SYNC:
			Packet_UnoccupiedSync(pkt);
			break;
		}

		pRakClient->DeallocatePacket(pkt);
	}

}

//----------------------------------------------------
// PACKET HANDLERS INTERNAL
//----------------------------------------------------

void CNetGame::Packet_PlayerSync(RakNet::Packet *p)
{
	CRemotePlayer * pPlayer;
	RakNet::BitStream bsPlayerSync((p->data + 1), (p->length - 1), false);
	ONFOOT_SYNC_DATA ofSync;
	SACMPLAYER playerId;

	if (GetGameState() != GAMESTATE_CONNECTED) return;

	memset(&ofSync, 0, sizeof(ONFOOT_SYNC_DATA));

	bsPlayerSync.Read(playerId);

	bsPlayerSync.Read((PCHAR)&ofSync, sizeof(ONFOOT_SYNC_DATA));

	if (m_pPlayerPool) {
		pPlayer = m_pPlayerPool->GetAt(playerId);
		if (pPlayer) {
			pPlayer->StoreOnFootFullSyncData(&ofSync);
		}
	}
}

//----------------------------------------------------

void CNetGame::Packet_UnoccupiedSync(RakNet::Packet *p)
{
	RakNet::BitStream bsUnocSync((p->data + 1), (p->length - 1), false);
	UNOCCUPIED_SYNC_DATA unocSync;
	SACMPLAYER playerId;

	if (GetGameState() != GAMESTATE_CONNECTED) return;

	memset(&unocSync, 0, sizeof(UNOCCUPIED_SYNC_DATA));

	bsUnocSync.Read(playerId);

	bsUnocSync.Read((char *)&unocSync, sizeof(UNOCCUPIED_SYNC_DATA));

	if (m_pPlayerPool) {
		CRemotePlayer *pPlayer = m_pPlayerPool->GetAt(playerId);
		if (pPlayer) {
			pPlayer->StoreUnoccupiedSyncData(&unocSync);
		}
	}
}

//----------------------------------------------------


void CNetGame::Packet_AimSync(RakNet::Packet *p)
{
	CRemotePlayer * pPlayer;
	RakNet::BitStream bsAimSync((p->data + 1), (p->length - 1), false);
	AIM_SYNC_DATA aimSync;
	SACMPLAYER playerId;

	if (GetGameState() != GAMESTATE_CONNECTED) return;

	bsAimSync.Read(playerId);
	bsAimSync.Read((PCHAR)&aimSync, sizeof(AIM_SYNC_DATA));

	if (m_pPlayerPool) {
		pPlayer = m_pPlayerPool->GetAt(playerId);
		if (pPlayer) {
			pPlayer->UpdateAimFromSyncData(&aimSync);
		}
	}
}

//----------------------------------------------------


void CNetGame::Packet_VehicleSync(RakNet::Packet *p)
{
	CRemotePlayer * pPlayer;
	RakNet::BitStream bsSync((p->data + 1), (p->length - 1), false);
	SACMPLAYER playerId;
	INCAR_SYNC_DATA icSync;

	if (GetGameState() != GAMESTATE_CONNECTED) return;

	memset(&icSync, 0, sizeof(INCAR_SYNC_DATA));

	bsSync.Read(playerId);

	bsSync.Read((PCHAR)&icSync, sizeof(INCAR_SYNC_DATA));

	if (m_pPlayerPool) {
		pPlayer = m_pPlayerPool->GetAt(playerId);
		if (pPlayer) {
			pPlayer->StoreInCarFullSyncData(&icSync);
		}
	}
}

//----------------------------------------------------

void CNetGame::Packet_PassengerSync(RakNet::Packet *p)
{
	CRemotePlayer * pPlayer;
	RakNet::BitStream bsPassengerSync((p->data + 1), (p->length - 1), false);
	SACMPLAYER	playerId;
	PASSENGER_SYNC_DATA psSync;

	if (GetGameState() != GAMESTATE_CONNECTED) return;

	bsPassengerSync.Read(playerId);
	bsPassengerSync.Read((PCHAR)&psSync, sizeof(PASSENGER_SYNC_DATA));

	if (m_pPlayerPool) {
		pPlayer = m_pPlayerPool->GetAt(playerId);
		//OutputDebugString("Getting Passenger Packets");
		if (pPlayer) {
			pPlayer->StorePassengerFullSyncData(&psSync);
		}
	}
}

//----------------------------------------------------

void CNetGame::Packet_TrailerSync(RakNet::Packet *p)
{
	CRemotePlayer * pPlayer;
	RakNet::BitStream bsSpectatorSync((p->data + 1), (p->length - 1), false);

	if (GetGameState() != GAMESTATE_CONNECTED) return;

	SACMPLAYER playerId;
	TRAILER_SYNC_DATA trSync;

	bsSpectatorSync.Read(playerId);
	bsSpectatorSync.Read((PCHAR)&trSync, sizeof(TRAILER_SYNC_DATA));

	if (m_pPlayerPool) {
		pPlayer = m_pPlayerPool->GetAt(playerId);
		if (pPlayer) {
			pPlayer->StoreTrailerFullSyncData(&trSync);
		}
	}
}

//----------------------------------------------------

void CNetGame::Packet_IncompatibleProtocolVersion(RakNet::Packet* packet)
{
	pChatWindow->AddDebugMessage("You're using an invalid version of RakNet.");
}

//----------------------------------------------------

void CNetGame::Packet_RSAPublicKeyMismatch(RakNet::Packet* packet)
{
	pChatWindow->AddDebugMessage("Failed to initialize encryption.");
}

//----------------------------------------------------

void CNetGame::Packet_ConnectionBanned(RakNet::Packet* packet)
{
	pChatWindow->AddDebugMessage("You're banned from this server.");
}

//----------------------------------------------------

void CNetGame::Packet_ConnectionRequestAccepted(RakNet::Packet* packet)
{
	pChatWindow->AddDebugMessage("Server has accepted the connection.");
}

//----------------------------------------------------

void CNetGame::Packet_NoFreeIncomingConnections(RakNet::Packet* packet)
{
	pChatWindow->AddDebugMessage("The server is full. Retrying...");
	SetGameState(GAMESTATE_WAIT_CONNECT);
}

//----------------------------------------------------

void CNetGame::Packet_DisconnectionNotification(RakNet::Packet* packet)
{
	pChatWindow->AddDebugMessage("Server closed the connection.");
	pRakClient->Shutdown(0);
}

//----------------------------------------------------

void CNetGame::Packet_ConnectionLost(RakNet::Packet* packet)
{
	pChatWindow->AddDebugMessage("Lost connection to the server. Reconnecting..");
	ShutdownForGameModeRestart();
	SetGameState(GAMESTATE_WAIT_CONNECT);
}

//----------------------------------------------------

void CNetGame::Packet_InvalidPassword(RakNet::Packet* packet)
{
	pChatWindow->AddDebugMessage("Wrong server password.");
	pRakClient->Shutdown(0);
}

//----------------------------------------------------

void CNetGame::Packet_ModifiedPacket(RakNet::Packet* packet)
{
#ifdef _DEBUG
	char szBuffer[256];
	sprintf(szBuffer, "Packet was modified, sent by id: %d, ip: %s",
		(unsigned int)packet->systemIndex, packet->systemAddress.ToString());
	pChatWindow->AddDebugMessage(szBuffer);
	//pRakClient->Disconnect(0);
#endif
}

//----------------------------------------------------
// RST

void CNetGame::Packet_ConnectAttemptFailed(RakNet::Packet* packet)
{
	pChatWindow->AddDebugMessage("The server didn't respond. Retrying..");
	SetGameState(GAMESTATE_WAIT_CONNECT);
}

//----------------------------------------------------
// Connection Success

void CNetGame::Packet_ConnectionSucceeded(RakNet::Packet *p)
{
	if (pChatWindow) {
		pChatWindow->AddDebugMessage("Connected. Joining the game...");
	}

	m_iGameState = GAMESTATE_AWAIT_JOIN;

	int iVersion = NETGAME_VERSION;
	BYTE byteNameLen = (BYTE)strlen(m_pPlayerPool->GetLocalPlayerName());

	RakNet::BitStream bsSend;
	bsSend.Write(iVersion);
	bsSend.Write(byteNameLen);
	bsSend.Write(m_pPlayerPool->GetLocalPlayerName(), byteNameLen);
	SendRPC(RPC_ClientJoin, &bsSend);
}

//----------------------------------------------------

void CNetGame::UpdatePlayerScoresAndPings()
{
	/*static DWORD dwLastUpdateTick = 0;

	if ((GetTickCount() - dwLastUpdateTick) > 3000) {
		dwLastUpdateTick = GetTickCount();
		RakNet::BitStream bsParams;
		SendRPC(RPC_UpdateScoresPingsIPs, &bsParams);
	}*/
}

//----------------------------------------------------

void CNetGame::ResetVehiclePool()
{
	if (m_pVehiclePool) {
		delete m_pVehiclePool;
	}
	m_pVehiclePool = new CVehiclePool();
}

//----------------------------------------------------

void CNetGame::ResetPlayerPool()
{
	if (m_pPlayerPool) {
		delete m_pPlayerPool;
	}
	m_pPlayerPool = new CPlayerPool();
}

//----------------------------------------------------

void CNetGame::ResetPickupPool()
{
	if (m_pPickupPool) {
		delete m_pPickupPool;
	}
	m_pPickupPool = new CPickupPool();
}

//----------------------------------------------------

void CNetGame::ResetMenuPool()
{
	if (m_pMenuPool) {
		delete m_pMenuPool;
	}
	m_pMenuPool = new CMenuPool();
}

//----------------------------------------------------

void CNetGame::ResetTextDrawPool()
{
	if (m_pTextDrawPool) {
		delete m_pTextDrawPool;
	}
	m_pTextDrawPool = new CTextDrawPool();
}

//----------------------------------------------------

void CNetGame::ResetObjectPool()
{
	if (m_pObjectPool) {
		delete m_pObjectPool;
	}
	m_pObjectPool = new CObjectPool();
}

//----------------------------------------------------

void CNetGame::ResetGangZonePool()
{
	if (m_pGangZonePool) {
		delete m_pGangZonePool;
	}
	m_pGangZonePool = new CGangZonePool();
}

//-----------------------------------------------------------

void CNetGame::ResetActorPool()
{
	if (m_pActorPool) {
		delete m_pActorPool;
	}
	m_pActorPool = new CActorPool();
}

//-----------------------------------------------------------
// Puts a personal marker using any of the radar icons on the map

void CNetGame::SetMapIcon(BYTE byteIndex, float fX, float fY, float fZ, BYTE byteIcon, DWORD dwColor)
{
	if (byteIndex >= 32) return;
	if (m_dwMapIcon[byteIndex] != NULL) DisableMapIcon(byteIndex);
	//ScriptCommand(&create_radar_marker_without_sphere, fX, fY, fZ, byteIcon, &m_dwMapIcon);
	m_dwMapIcon[byteIndex] = pGame->CreateRadarMarkerIcon(byteIcon, fX, fY, fZ, dwColor);
}

//-----------------------------------------------------------
// Removes the Map Icon

void CNetGame::DisableMapIcon(BYTE byteIndex)
{
	if (byteIndex >= 32) return;
	ScriptCommand(&disable_marker, m_dwMapIcon[byteIndex]);
	m_dwMapIcon[byteIndex] = NULL;
}

//----------------------------------------------------

void CNetGame::ResetMapIcons()
{
	BYTE i;
	for (i = 0; i < 32; i++)
	{
		if (m_dwMapIcon[i] != NULL) DisableMapIcon(i);
	}
}

//----------------------------------------------------

void CNetGame::SendRPC(char *szPacket, RakNet::BitStream *bsData) {
	GetRPC()->Call(szPacket, bsData, IMMEDIATE_PRIORITY, RELIABLE, NULL, RakNet::UNASSIGNED_SYSTEM_ADDRESS, TRUE);
}

//----------------------------------------------------

RakNet::RakPeerInterface *CNetGame::GetRakClient()
{ 
	return pRakClient; 
}

//----------------------------------------------------

RakNet::RPC4 *CNetGame::GetRPC()
{ 
	return pRPC4Plugin; 
}

//----------------------------------------------------
// EOF
