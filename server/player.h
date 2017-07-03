#ifndef PLAYER_H
#define PLAYER_H

#define PLAYER_STATE_NONE						0
#define PLAYER_STATE_ONFOOT						1
#define PLAYER_STATE_DRIVER						2
#define PLAYER_STATE_PASSENGER					3
#define PLAYER_STATE_EXIT_VEHICLE				4
#define PLAYER_STATE_ENTER_VEHICLE_DRIVER		5
#define PLAYER_STATE_ENTER_VEHICLE_PASSENGER	6
#define PLAYER_STATE_WASTED						7
#define PLAYER_STATE_SPAWNED					8
#define PLAYER_STATE_SPECTATING					9

#define UPDATE_TYPE_NONE		0
#define UPDATE_TYPE_ONFOOT		1
#define UPDATE_TYPE_INCAR		2
#define UPDATE_TYPE_PASSENGER	3

#define SPECTATE_TYPE_NONE		0
#define SPECTATE_TYPE_PLAYER	1
#define SPECTATE_TYPE_VEHICLE	2

#define SPECIAL_ACTION_NONE				0
#define SPECIAL_ACTION_USEJETPACK		2
#define SPECIAL_ACTION_DANCE1			5
#define SPECIAL_ACTION_DANCE2			6
#define SPECIAL_ACTION_DANCE3			7
#define SPECIAL_ACTION_DANCE4			8
#define SPECIAL_ACTION_HANDSUP			10
#define SPECIAL_ACTION_USECELLPHONE		11
#define SPECIAL_ACTION_SITTING			12
#define SPECIAL_ACTION_STOPUSECELLPHONE 13

// Packs vehicle health into a byte
#define PACK_VEHICLE_HEALTH(f)		(BYTE)(f / 4)
#define UNPACK_VEHICLE_HEALTH(b)	(float)b * 4

class CPlayer
{
private:
	SACMPLAYER				m_bytePlayerID;
	BYTE					m_byteUpdateFromNetwork;

	// Information that is synced.
	ONFOOT_SYNC_DATA		m_ofSync;
	INCAR_SYNC_DATA			m_icSync;
	PASSENGER_SYNC_DATA		m_psSync;
	AIM_SYNC_DATA			m_aimSync;
	SPECTATOR_SYNC_DATA		m_spSync;
	TRAILER_SYNC_DATA		m_trSync;

	WORD					m_wLastKeys;

	BOOL					m_bHasAimUpdates;
	BOOL					m_bHasTrailerUpdates;
	BYTE					m_byteSeatID;
	BYTE					m_byteState;

	VECTOR					m_vecCheckpoint;
	float					m_fCheckpointSize;
	BOOL					m_bInCheckpoint;

	VECTOR					m_vecRaceCheckpoint;
	VECTOR					m_vecRaceNextCheckpoint;
	BYTE					m_byteRaceCheckpointType;
	float					m_fRaceCheckpointSize;
	BOOL					m_bInRaceCheckpoint;

	BYTE					m_byteFightingStyle;
public:
	PLAYER_SPAWN_INFO		m_SpawnInfo;
	BOOL					m_bHasSpawnInfo;
	BYTE					m_byteWantedLevel;

	SACMVEHICLE				m_VehicleID;
	DWORD					m_dwColor;
	BOOL					m_bCheckpointEnabled;
	BOOL					m_bRaceCheckpointEnabled;
	int						m_iInteriorId;
		
	// Weapon data
	DWORD					m_dwSlotAmmo[13];
	BYTE					m_byteSlotWeapon[13];
	
	BYTE					m_byteTime; // Uses
	float					m_fGameTime; // Time in seconds (game minutes)

	BYTE					m_byteSpectateType;
	DWORD					m_SpectateID; // Vehicle or player id

	ONFOOT_SYNC_DATA* GetOnFootSyncData() { return &m_ofSync; }
	INCAR_SYNC_DATA* GetInCarSyncData() { return &m_icSync; }
	PASSENGER_SYNC_DATA* GetPassengerSyncData() { return &m_psSync; }
	AIM_SYNC_DATA* GetAimSyncData() { return &m_aimSync; }
	SPECTATOR_SYNC_DATA* GetSpectatorSyncData() { return &m_spSync; }

	void SetState(BYTE byteState);
	BYTE GetState() { return m_byteState; };

	CPlayer();
	~CPlayer() {};

	float	m_fHealth;
	float	m_fArmour;
	VECTOR  m_vecPos;
	float	m_fRotation;
	BOOL	m_bCanTeleport;

	BOOL IsActive() { 
		if( m_byteState != PLAYER_STATE_NONE && m_byteState != PLAYER_STATE_SPECTATING ) { return TRUE; }
		return FALSE;
	};
	
	void Deactivate() {	
		m_bHasSpawnInfo = FALSE;
		memset(&m_SpawnInfo,0,sizeof(PLAYER_SPAWN_INFO));
		m_dwColor = 0;
		m_byteState = PLAYER_STATE_NONE;
		m_byteTime = 0;
		m_fGameTime = 720.0f; // 12 o'clock in minutes	
		m_fHealth = 0.0f;
		m_fArmour = 0.0f;
	};

	void UpdatePosition(float x, float y, float z);

	// Process this player during the server loop.
	void Process(float fElapsedTime);
	void BroadcastSyncData();
	void Say(unsigned char * szText, BYTE byteTextLength);
	void SetID(SACMPLAYER bytePlayerID) { m_bytePlayerID = bytePlayerID; };
	
	void StoreOnFootFullSyncData(ONFOOT_SYNC_DATA * pofSync);
	void StoreInCarFullSyncData(INCAR_SYNC_DATA * picSync);
	void StorePassengerFullSyncData(PASSENGER_SYNC_DATA *ppsSync);
	void StoreSpectatorFullSyncData(SPECTATOR_SYNC_DATA *pspSync);
	void StoreAimSyncData(AIM_SYNC_DATA *paimSync);
	void StoreTrailerFullSyncData(TRAILER_SYNC_DATA* trSync);
	void SetSpawnInfo(PLAYER_SPAWN_INFO *pSpawn);

	PLAYER_SPAWN_INFO * GetSpawnInfo() { return &m_SpawnInfo; };

	void HandleDeath(BYTE byteReason, SACMPLAYER byteWhoWasResponsible);
	void Spawn();
	void SpawnForWorld(BYTE byteTeam, int iSkin, VECTOR * vecPos, float fRotation);
	void SpawnForPlayer(SACMPLAYER byteForPlayerID);

	void EnterVehicle(SACMVEHICLE VehicleID,BYTE bytePassenger);
	void ExitVehicle(SACMVEHICLE VehicleID);

	void SetPlayerColor(DWORD dwColor);
	DWORD GetPlayerColor() { return m_dwColor; };

	void SetCheckpoint(float fX, float fY, float fZ, float fSize);
	void ToggleCheckpoint(BOOL bEnabled);
	void SetRaceCheckpoint(int iType, float fX, float fY, float fZ, float fNX, float fNY, float fNZ, float fSize);
	void ToggleRaceCheckpoint(BOOL bEnabled);

	BOOL IsInCheckpoint() { return m_bInCheckpoint; };
	BOOL IsInRaceCheckpoint() { return m_bInRaceCheckpoint; };
	BYTE GetTeam() { return m_SpawnInfo.byteTeam; };
	BYTE GetCurrentWeapon() { return m_ofSync.byteCurrentWeapon; };
	
	//WEAPON_SLOT_TYPE* GetWeaponSlotsData();
	void SetWeaponSlot(BYTE byteSlot, DWORD dwWeapon, DWORD dwAmmo);
	
	DWORD GetSlotWeapon(BYTE bSlot) { return m_byteSlotWeapon[bSlot]; };
	DWORD GetSlotAmmo(BYTE bSlot) { return m_dwSlotAmmo[bSlot]; };
	void SetCurrentWeaponAmmo(DWORD dwAmmo);
	void SetWantedLevel(BYTE byteLevel) { m_byteWantedLevel = byteLevel; };
	BYTE GetWantedLevel() { return m_byteWantedLevel; };
	
	void SetTime(BYTE byteHour, BYTE byteMinute);
	void SetClock(BYTE byteClock);

	BYTE CheckWeapon(BYTE weapon);
	void CheckKeyUpdatesForScript(WORD wKeys);

	BYTE GetSpecialAction() {
		if(GetState() == PLAYER_STATE_ONFOOT) return m_ofSync.byteSpecialAction;
		return SPECIAL_ACTION_NONE;
	};
};

#endif

//----------------------------------------------------
// EOF

