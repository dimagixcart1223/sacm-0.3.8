#ifndef PLAYERPOOL_H
#define PLAYERPOOL_H

#define INVALID_PLAYER_ID 255
#define NO_TEAM 255

#define VALID_KILL		  1
#define TEAM_KILL		  2
#define SELF_KILL		  3


//----------------------------------------------------

class CPlayerPool
{
private:
	
	BOOL	m_bPlayerSlotState[MAX_PLAYERS];
	CPlayer *m_pPlayers[MAX_PLAYERS];
	CHAR	m_szPlayerName[MAX_PLAYERS][MAX_PLAYER_NAME+1];
	int 	m_iPlayerScore[MAX_PLAYERS];
	int		m_iPlayerMoney[MAX_PLAYERS];
	DWORD	m_dwPlayerAmmo[MAX_PLAYERS];
	BOOL	m_bIsAnAdmin[MAX_PLAYERS];
	BYTE	m_byteVirtualWorld[MAX_PLAYERS];
	int		m_iPlayerCount;

public:
	
	CPlayerPool();
	~CPlayerPool();

	BOOL Process(float fElapsedTime);
	BOOL New(SACMPLAYER PlayerId, PCHAR szPlayerName);
	BOOL Delete(SACMPLAYER PlayerId, BYTE byteReason);
		
	// Retrieve a player
	CPlayer* GetAt(SACMPLAYER PlayerId) {
		if (PlayerId >= MAX_PLAYERS) { return NULL; }
		return m_pPlayers[PlayerId];
	};

	// Find out if the slot is inuse.
	BOOL GetSlotState(SACMPLAYER PlayerId) {
		if(PlayerId >= MAX_PLAYERS) { return FALSE; }
		return m_bPlayerSlotState[PlayerId];
	};

	PCHAR GetPlayerName(SACMPLAYER PlayerId) {
		if(PlayerId >= MAX_PLAYERS) { return FALSE; }
		return m_szPlayerName[PlayerId];
	};

	int GetPlayerScore(SACMPLAYER PlayerId) {
		if(PlayerId >= MAX_PLAYERS) { return FALSE; }
		return m_iPlayerScore[PlayerId];
	};

	void SetPlayerScore(SACMPLAYER PlayerId, int iScore) {
		if(PlayerId >= MAX_PLAYERS) return;
		m_iPlayerScore[PlayerId] = iScore;
	};

	void SetPlayerName(SACMPLAYER PlayerId, PCHAR szName) {
		strcpy(m_szPlayerName[PlayerId], szName);
	}

	int GetPlayerMoney(SACMPLAYER PlayerId) {
		if(PlayerId >= MAX_PLAYERS) { return FALSE; }
		return m_iPlayerMoney[PlayerId];
	};

	void SetPlayerMoney(SACMPLAYER PlayerId, int iMoney) {
		if(PlayerId >= MAX_PLAYERS) return;
		m_iPlayerMoney[PlayerId] = iMoney;
	};

	DWORD GetPlayerAmmo(SACMPLAYER PlayerId) {
		if(PlayerId >= MAX_PLAYERS) { return FALSE; }
		return m_dwPlayerAmmo[PlayerId];
	};

	void SetPlayerAmmo(SACMPLAYER PlayerId, DWORD dwAmmo) {
		if(PlayerId >= MAX_PLAYERS) return;
		m_dwPlayerAmmo[PlayerId] = dwAmmo;
	};

	void ResetPlayerScoresAndMoney() {
		memset(&m_iPlayerScore[0],0,sizeof(int) * MAX_PLAYERS);
		memset(&m_iPlayerMoney[0],0,sizeof(int) * MAX_PLAYERS);	
		memset(&m_byteVirtualWorld[0],0,sizeof(BYTE) * MAX_PLAYERS);	
	};
	
	void SetPlayerVirtualWorld(SACMPLAYER PlayerId, BYTE byteVirtualWorld);
	
	BYTE GetPlayerVirtualWorld(SACMPLAYER PlayerId) {
		if (PlayerId >= MAX_PLAYERS) { return 0; }
		return m_byteVirtualWorld[PlayerId];		
	};

	void SetAdmin(SACMPLAYER PlayerId) { m_bIsAnAdmin[PlayerId] = TRUE; };
	BOOL IsAdmin(SACMPLAYER PlayerId) { return m_bIsAnAdmin[PlayerId]; };

	void InitPlayersForPlayer(SACMPLAYER PlayerId);
	void InitSpawnsForPlayer(SACMPLAYER PlayerId);

	BYTE GetKillType(BYTE byteWhoKilled, BYTE byteWhoDied);

	float GetDistanceFromPlayerToPlayer(BYTE bytePlayer1, BYTE bytePlayer2);
	float GetDistanceSquaredFromPlayerToPlayer(BYTE bytePlayer1, BYTE bytePlayer2);
	BOOL  IsNickInUse(PCHAR szNick);

	int GetPlayerCount() { return m_iPlayerCount; };

	void DeactivateAll();

};

//----------------------------------------------------

#endif