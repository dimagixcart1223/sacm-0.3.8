#include "main.h"
extern CNetGame *pNetGame;

#define TRAIN_PASSENGER_LOCO			537
#define TRAIN_FREIGHT_LOCO				538
#define TRAIN_PASSENGER					569
#define TRAIN_FREIGHT					570
#define TRAIN_TRAM						449

//----------------------------------------------------------

#define PI 3.14159265

//----------------------------------------------------------

CVehicle::CVehicle( int byteModel, VECTOR *vecPos, float fRotation, int iColor1, int iColor2,int iRespawnTime)
{
	// Store the spawn info.
	memcpy(&m_vecSpawnPosition, vecPos, sizeof(VECTOR));
	m_SpawnInfo.iVehicleType = byteModel;
	m_SpawnInfo.fRotation = fRotation;
	m_SpawnInfo.aColor1 = iColor1;
	m_SpawnInfo.aColor2 = iColor2;
	m_SpawnInfo.byteDoorsLocked = 0;
	m_iRespawnDelay = iRespawnTime;

	Reset();
}

//----------------------------------------------------
// Resets all the vehicle info

void CVehicle::Reset()
{
	m_bIsActive = TRUE;
	m_bIsWasted = FALSE;
	m_bHasHadUpdate = FALSE;
	m_DriverID = INVALID_ID;

	m_SpawnInfo.vecPos = m_vecSpawnPosition;

	m_SpawnInfo.fHealth = 1000.0f;
	m_fHealth = 1000.0f;

	m_SpawnInfo.byteInterior = 0;
	m_iInterior = 0;

	m_SpawnInfo.dwPanelDamageStatus = 0;
	m_SpawnInfo.dwDoorDamageStatus = 0;
	m_SpawnInfo.byteLightDamageStatus = 0;

	m_dwPanelDamage = 0;
	m_dwDoorDamage = 0;
	m_byteLightDamage = 0;

	m_cColor1 = m_SpawnInfo.aColor1;
	m_cColor2 = m_SpawnInfo.aColor2;

	m_vecPosition = m_vecSpawnPosition;

	memset(&m_vecMoveSpeed, 0, sizeof(VECTOR));
	memset(&m_vecTurnSpeed, 0, sizeof(VECTOR));

	m_dwWasteTick = GetTickCount();
}

//----------------------------------------------------

void CVehicle::Respawn()
{
	// DESTROY
	RakNet::BitStream bsSend;
	bsSend.Write(m_VehicleID);
	pNetGame->SendRPC(RPC_WorldVehicleRemove,&bsSend,-1, TRUE);

	// RESET
	Reset();
	
	// RESPAWN
	/*for(SACMPLAYER playerId = 0; playerId < MAX_PLAYERS; playerId++)
	{
		if(pNetGame->GetPlayerPool()->GetSlotState(playerId))
		{
			SpawnForPlayer(playerId);
		}
	}*/
}

//----------------------------------------------------

void CVehicle::Update(SACMPLAYER bytePlayerID, MATRIX4X4 * matWorld, float fHealth, SACMVEHICLE TrailerID)
{
	// we should ignore any updates if it recently respawned
	if((GetTickCount() - m_dwLastRespawnedTick) < 5000) return;

	m_DriverID = bytePlayerID;
	memcpy(&m_matWorld,matWorld,sizeof(MATRIX4X4));
	m_fHealth = fHealth;

	if(TrailerID < MAX_VEHICLES) {
		CVehicle* Trailer;
		if (TrailerID) Trailer = pNetGame->GetVehiclePool()->GetAt(TrailerID);
		else Trailer = pNetGame->GetVehiclePool()->GetAt(m_TrailerID);
		if (Trailer) Trailer->SetCab(m_VehicleID);
		m_TrailerID = TrailerID;
	} else {
		m_TrailerID = 0;
	}
	m_dwLastSeenOccupiedTick = GetTickCount();
	m_bHasBeenOccupied = true;

	if(m_fHealth <= 0.0f) m_bDead = true;
}
//----------------------------------------------------------

void CVehicle::CheckForIdleRespawn()
{	
	// can't respawn an idle train or train part
	if(IsATrainPart()) return;

	if( (GetTickCount() - m_dwLastRespawnedTick) < 10000 ) {
		// We should wait at least 10 seconds after the last
		// respawn before checking. Come back later.
		return;
	}

	if(!IsOccupied()) {
		if( m_bHasBeenOccupied &&
			(GetTickCount() - m_dwLastSeenOccupiedTick) >= (DWORD)m_iRespawnDelay ) {
			//printf("Respawning idle vehicle %u\n",m_VehicleID);
			Respawn();
		}
	}
}

//----------------------------------------------------

void CVehicle::Process(float fElapsedTime)
{
	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
	
	// Check for an idle vehicle.. but don't do this
	// every ::Process because it would be too CPU intensive.
	if(!m_bDeathHasBeenNotified && m_iRespawnDelay != (-1) && ((rand() % 20) == 0)) {
		CheckForIdleRespawn();
		return;
	}
	
	// we'll check for a dead vehicle.
	if(m_bDead)
	{
		if (!m_bDeathHasBeenNotified)
		{
			m_bDeathHasBeenNotified = true;
			m_szNumberPlate[0] = 0;

			if(pNetGame->GetGameMode() && pNetGame->GetFilterScripts()) {
				pNetGame->GetFilterScripts()->OnVehicleDeath(m_VehicleID, 255);
				pNetGame->GetGameMode()->OnVehicleDeath(m_VehicleID, 255);
			}
			m_dwLastSeenOccupiedTick = GetTickCount();
		}
		if(!(rand() % 20) && GetTickCount() - m_dwLastSeenOccupiedTick > 10000)
		{
			//Respawn();
		}
	}
}

//----------------------------------------------------

void CVehicle::SpawnForPlayer(SACMPLAYER forPlayerID)
{
	RakNet::BitStream bsVehicleSpawn;

	VEHICLE_SPAWN_INFO NewVehicle = m_SpawnInfo;
	NewVehicle.VehicleId = m_VehicleID;
	bsVehicleSpawn.Write((PCHAR)&NewVehicle, sizeof(VEHICLE_SPAWN_INFO));
	
	pNetGame->SendRPC(RPC_WorldVehicleAdd,&bsVehicleSpawn,forPlayerID, FALSE);
}

//----------------------------------------------------------

void CVehicle::UpdateDamageStatus(DWORD dwPanelDamage, DWORD dwDoorDamage, BYTE byteLightDamage)
{
	if(m_dwPanelDamage != dwPanelDamage || m_dwDoorDamage != dwDoorDamage || 
		m_byteLightDamage != byteLightDamage)
	{
		m_dwPanelDamage = dwPanelDamage;
		m_dwDoorDamage = dwDoorDamage;
		m_byteLightDamage = byteLightDamage;
		m_SpawnInfo.dwPanelDamageStatus = dwPanelDamage;
		m_SpawnInfo.dwDoorDamageStatus = dwDoorDamage;
		m_SpawnInfo.byteLightDamageStatus = byteLightDamage;

		//pPawn->CallFunction("OnVehicleDamageStatusUpdate", 1, "i", m_VehicleID);
	}
}
//----------------------------------------------------------

BOOL CVehicle::IsOccupied()
{
	CPlayer *pPlayer;

	// find drivers or passengers of this vehicle
	SACMPLAYER x = 0;
	while(x < MAX_PLAYERS) {
		if(pNetGame->GetPlayerPool()->GetSlotState(x)) {
			pPlayer = pNetGame->GetPlayerPool()->GetAt(m_DriverID);

			if( pPlayer && (pPlayer->m_VehicleID == m_VehicleID) &&
				 (pPlayer->GetState() == PLAYER_STATE_DRIVER ||
				 pPlayer->GetState() == PLAYER_STATE_PASSENGER) ) {
					 return TRUE;
			}
		}
		x++;
	}

	return FALSE;
}

//----------------------------------------------------------

bool CVehicle::IsATrainPart()
{
	int nModel = m_SpawnInfo.iVehicleType;

	if(nModel == TRAIN_PASSENGER_LOCO) return true;
	if(nModel == TRAIN_PASSENGER) return true;
	if(nModel == TRAIN_FREIGHT_LOCO) return true;
	if(nModel == TRAIN_FREIGHT) return true;
	if(nModel == TRAIN_TRAM) return true;

	return false;
}

//----------------------------------------------------------

void CVehicle::GetPosition(PVECTOR vecPosition)
{
	vecPosition = (PVECTOR)&m_vecPosition;
}

//----------------------------------------------------------

void CVehicle::SetPosition(VECTOR vecPosition)
{
	m_vecPosition = vecPosition;
	m_SpawnInfo.vecPos = vecPosition;
}

//----------------------------------------------------------

void CVehicle::GetMoveSpeedVector(PVECTOR vecMoveSpeed)
{
	vecMoveSpeed = (PVECTOR)&m_vecMoveSpeed;
}

//----------------------------------------------------------

void CVehicle::SetMoveSpeedVector(VECTOR vecMoveSpeed)
{
	m_vecMoveSpeed = vecMoveSpeed;
}

//----------------------------------------------------------

void CVehicle::GetTurnSpeedVector(PVECTOR vecTurnSpeed)
{
	vecTurnSpeed = (PVECTOR)&m_vecTurnSpeed;
}

//----------------------------------------------------------

void CVehicle::SetTurnSpeedVector(VECTOR vecTurnSpeed)
{
	m_vecTurnSpeed = vecTurnSpeed;
}

//----------------------------------------------------------

float CVehicle::GetHealth()
{
	return m_fHealth;
}

//----------------------------------------------------------

void CVehicle::SetHealth(float fHealth)
{
	m_fHealth = fHealth;
	m_SpawnInfo.fHealth = fHealth;
}

//----------------------------------------------------------

bool CVehicle::GetHasHadUpdate()
{
	return m_bHasHadUpdate;
}

//----------------------------------------------------------

void CVehicle::SetHasHadUpdate(bool bHadUpdate)
{
	m_bHasHadUpdate = bHadUpdate;
}

//----------------------------------------------------------

SACMPLAYER CVehicle::GetDriver()
{
	return m_DriverID;
}

//----------------------------------------------------------

void CVehicle::SetDriver(SACMPLAYER driverId)
{
	m_DriverID = driverId;
}

//----------------------------------------------------------

void CVehicle::SetNumberPlate(PCHAR Plate)
{
	strcpy(m_szNumberPlate, Plate);
	RakNet::BitStream bsPlate;
	bsPlate.Write(m_VehicleID);
	bsPlate.Write(Plate, 9);
	pNetGame->SendRPC(RPC_ScrNumberPlate, &bsPlate, -1, FALSE);
}

//----------------------------------------------------------