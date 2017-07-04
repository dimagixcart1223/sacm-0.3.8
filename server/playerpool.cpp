#include "main.h"
extern CNetGame *pNetGame;

//----------------------------------------------------

CPlayerPool::CPlayerPool()
{
	// loop through and initialize all net players to null and slot states to false
	for (SACMPLAYER PlayerId = 0; PlayerId < MAX_PLAYERS; PlayerId++) {
		m_bPlayerSlotState[PlayerId] = FALSE;
		m_pPlayers[PlayerId] = NULL;
	}
	m_iPlayerCount = 0;
}

//----------------------------------------------------

CPlayerPool::~CPlayerPool()
{
	for (SACMPLAYER PlayerId = 0; PlayerId < MAX_PLAYERS; PlayerId++) {
		Delete(PlayerId, 0);
	}
	m_iPlayerCount = 0;
}

//----------------------------------------------------

BOOL CPlayerPool::New(SACMPLAYER PlayerId, PCHAR szPlayerName)
{
	if (PlayerId > MAX_PLAYERS) return FALSE;
	if (strlen(szPlayerName) > MAX_PLAYER_NAME) return FALSE;

	m_pPlayers[PlayerId] = new CPlayer();

	if (m_pPlayers[PlayerId])
	{
		strcpy(m_szPlayerName[PlayerId], szPlayerName);
		m_pPlayers[PlayerId]->SetID(PlayerId);
		m_bPlayerSlotState[PlayerId] = TRUE;
		m_iPlayerScore[PlayerId] = 0;
		m_iPlayerMoney[PlayerId] = 0;
		m_bIsAnAdmin[PlayerId] = FALSE;
		m_byteVirtualWorld[PlayerId] = 0;
		pNetGame->GetMenuPool()->SetPlayerMenu(PlayerId, 255);

		// Notify all the other players of a newcommer with
		// a 'ServerJoin' join RPC 
		RakNet::BitStream bsSend;
		bsSend.Write(PlayerId);
		BYTE namelen = strlen(m_szPlayerName[PlayerId]);
		bsSend.Write(namelen);
		bsSend.Write(m_szPlayerName[PlayerId], namelen);

		pNetGame->SendRPC(RPC_ServerJoin, &bsSend, PlayerId, TRUE);

		logprintf("[join] %s has joined the server (%s)", GetPlayerName(PlayerId), pNetGame->GetRakServer()->GetSystemAddressFromIndex(PlayerId).ToString(true));

		pNetGame->GetFilterScripts()->OnPlayerConnect(PlayerId);
		CGameMode *pGameMode = pNetGame->GetGameMode();
		if (pGameMode) {
			pGameMode->OnPlayerConnect(PlayerId);
		}

		m_iPlayerCount++;

		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

//----------------------------------------------------

BOOL CPlayerPool::Delete(SACMPLAYER PlayerId, BYTE byteReason)
{
	if (!GetSlotState(PlayerId) || !m_pPlayers[PlayerId])
	{
		return FALSE; // Player already deleted or not used.
	}

	pNetGame->GetFilterScripts()->OnPlayerDisconnect(PlayerId, byteReason);
	CGameMode *pGameMode = pNetGame->GetGameMode();
	if (pGameMode) {
		pGameMode->OnPlayerDisconnect(PlayerId, byteReason);
	}

	m_bPlayerSlotState[PlayerId] = FALSE;
	delete m_pPlayers[PlayerId];
	m_pPlayers[PlayerId] = NULL;
	m_bIsAnAdmin[PlayerId] = FALSE;

	// Notify all the other players that this client is quiting.
	RakNet::BitStream bsSend;
	bsSend.Write(PlayerId);
	bsSend.Write(byteReason);
	pNetGame->SendRPC(RPC_ServerQuit, &bsSend, PlayerId, TRUE);

	CObjectPool* pObjectPool = pNetGame->GetObjectPool();
	for (BYTE i = 0; i < MAX_OBJECTS; i++)
	{
		// Remove all personal objects (checking done by the function)
		pObjectPool->DeleteForPlayer(PlayerId, i);
	}

	m_iPlayerCount--;

	return TRUE;
}

//----------------------------------------------------

BOOL CPlayerPool::Process(float fElapsedTime)
{
	// Process all CPlayers
	for (SACMPLAYER PlayerId = 0; PlayerId < MAX_PLAYERS; PlayerId++)
	{
		if (TRUE == m_bPlayerSlotState[PlayerId])
		{
			m_pPlayers[PlayerId]->Process(fElapsedTime);
		}
	}
	return TRUE;
}

//----------------------------------------------------

void CPlayerPool::InitPlayersForPlayer(SACMPLAYER PlayerId)
{
	BYTE lp = 0;
	RakNet::BitStream bsExistingClient;
	RakNet::BitStream bsPlayerVW;

	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
	bool send = false;

	for (SACMPLAYER lp = 0; lp < MAX_PLAYERS; lp++)
	{
		if ((GetSlotState(lp) == TRUE) && (lp != PlayerId)) {
			BYTE namelen = (BYTE)strlen(GetPlayerName(lp));

			bsExistingClient.Write(lp);
			bsExistingClient.Write(namelen);
			bsExistingClient.Write(GetPlayerName(lp), namelen);

			pNetGame->SendRPC(RPC_ServerJoin, &bsExistingClient, PlayerId, FALSE);
			bsExistingClient.Reset();

			CPlayer *pSpawnPlayer = GetAt(lp);

			pSpawnPlayer->SpawnForPlayer(PlayerId);

			// Send all the VW data in one lump
			bsPlayerVW.Write(lp);
			bsPlayerVW.Write(GetPlayerVirtualWorld(lp));
			pNetGame->SendRPC(RPC_ScrSetPlayerVirtualWorld, &bsPlayerVW, PlayerId, FALSE);
			bsPlayerVW.Reset();
			send = true;
		}
	}
}

//----------------------------------------------------

void CPlayerPool::InitSpawnsForPlayer(SACMPLAYER PlayerId)
{
	SACMPLAYER x = 0;
	CPlayer *pSpawnPlayer;

	for (x = 0; x < MAX_PLAYERS; x++) {
		if ((GetSlotState(x) == TRUE) && (x != PlayerId)) {
			pSpawnPlayer = GetAt(x);
			if (pSpawnPlayer->IsActive()) {
				pSpawnPlayer->SpawnForPlayer(PlayerId);
			}
		}
	}
}

//----------------------------------------------------
// Return constant describing the type of kill.

BYTE CPlayerPool::GetKillType(BYTE byteWhoKilled, BYTE byteWhoDied)
{

	if (byteWhoKilled != INVALID_PLAYER_ID &&
		byteWhoKilled < MAX_PLAYERS &&
		byteWhoDied < MAX_PLAYERS) {

		if (m_bPlayerSlotState[byteWhoKilled] && m_bPlayerSlotState[byteWhoDied]) {
			if (GetAt(byteWhoKilled)->GetTeam() == NO_TEAM || GetAt(byteWhoDied)->GetTeam() == NO_TEAM) {
				return VALID_KILL;
			}
			if (GetAt(byteWhoKilled)->GetTeam() != GetAt(byteWhoDied)->GetTeam()) {
				return VALID_KILL;
			}
			else {
				return TEAM_KILL;
			}
		}
		return SELF_KILL;
	}

	if (byteWhoKilled == INVALID_PLAYER_ID && byteWhoDied < MAX_PLAYERS)
	{
		return SELF_KILL;
	}

	return SELF_KILL;
}

//----------------------------------------------------

float CPlayerPool::GetDistanceFromPlayerToPlayer(BYTE bytePlayer1, BYTE bytePlayer2)
{
	VECTOR	*vecFromPlayer;
	VECTOR	*vecThisPlayer;
	float	fSX, fSY, fSZ;

	CPlayer * pPlayer1 = GetAt(bytePlayer1);
	CPlayer * pPlayer2 = GetAt(bytePlayer2);

	vecFromPlayer = &pPlayer1->m_vecPos;
	vecThisPlayer = &pPlayer2->m_vecPos;

	fSX = (vecThisPlayer->X - vecFromPlayer->X) * (vecThisPlayer->X - vecFromPlayer->X);
	fSY = (vecThisPlayer->Y - vecFromPlayer->Y) * (vecThisPlayer->Y - vecFromPlayer->Y);
	fSZ = (vecThisPlayer->Z - vecFromPlayer->Z) * (vecThisPlayer->Z - vecFromPlayer->Z);

	return (float)sqrt(fSX + fSY + fSZ);
}

//----------------------------------------------------

float CPlayerPool::GetDistanceSquaredFromPlayerToPlayer(BYTE bytePlayer1, BYTE bytePlayer2)
{
	VECTOR	*vecFromPlayer;
	VECTOR	*vecThisPlayer;
	float	fSX, fSY, fSZ;

	CPlayer * pPlayer1 = GetAt(bytePlayer1);
	CPlayer * pPlayer2 = GetAt(bytePlayer2);

	vecFromPlayer = &pPlayer1->m_vecPos;
	vecThisPlayer = &pPlayer2->m_vecPos;

	fSX = (vecThisPlayer->X - vecFromPlayer->X) * (vecThisPlayer->X - vecFromPlayer->X);
	fSY = (vecThisPlayer->Y - vecFromPlayer->Y) * (vecThisPlayer->Y - vecFromPlayer->Y);
	fSZ = (vecThisPlayer->Z - vecFromPlayer->Z) * (vecThisPlayer->Z - vecFromPlayer->Z);

	return (float)(fSX + fSY + fSZ);
}

//----------------------------------------------------

BOOL CPlayerPool::IsNickInUse(PCHAR szNick)
{
	int x = 0;
	while (x != MAX_PLAYERS) {
		if (GetSlotState((BYTE)x)) {
			if (!stricmp(GetPlayerName((BYTE)x), szNick)) {
				return TRUE;
			}
		}
		x++;
	}
	return FALSE;
}

//----------------------------------------------------

void CPlayerPool::DeactivateAll()
{
	CGameMode* pGameMode = pNetGame->GetGameMode();
	CFilterScripts* pFilterScripts = pNetGame->GetFilterScripts();
	for (SACMPLAYER PlayerId = 0; PlayerId < MAX_PLAYERS; PlayerId++) {
		if (TRUE == m_bPlayerSlotState[PlayerId]) {
			m_pPlayers[PlayerId]->Deactivate();
			pGameMode->OnPlayerDisconnect(PlayerId, 1);
			pFilterScripts->OnPlayerDisconnect(PlayerId, 1);
		}
		m_byteVirtualWorld[PlayerId] = 0;
	}
}

//----------------------------------------------------

void CPlayerPool::SetPlayerVirtualWorld(SACMPLAYER PlayerId, BYTE byteVirtualWorld)
{
	if (PlayerId >= MAX_PLAYERS) return;

	m_byteVirtualWorld[PlayerId] = byteVirtualWorld;
	// Tell existing players it's changed
	RakNet::BitStream bsData;
	bsData.Write(PlayerId); // player id
	bsData.Write(byteVirtualWorld); // vw id
	pNetGame->SendRPC(RPC_ScrSetPlayerVirtualWorld, &bsData, -1, TRUE);
}

//----------------------------------------------------
