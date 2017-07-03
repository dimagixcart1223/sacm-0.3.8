/*

	SA:CM Multiplayer Modification
	Copyright 2004-2005 SA:CM Team

	file:
		vehicle.h
	desc:
		Vehicle handling header file.

    Version: $Id: vehicle.h,v 1.3 2006/04/12 19:26:45 mike Exp $

*/

#ifndef SAMPSRV_VEHICLE_H
#define SAMPSRV_VEHICLE_H

class CVehicle
{
public:

	SACMVEHICLE				m_VehicleID;
	SACMVEHICLE				m_TrailerID;
	SACMVEHICLE				m_CabID;
	SACMPLAYER				m_DriverID;
	SACMPLAYER				m_bytePassengers[7];
	BOOL					m_bIsActive;
	BOOL					m_bIsWasted;
	VECTOR					m_vecSpawnPosition;
	VEHICLE_SPAWN_INFO		m_SpawnInfo;
	VECTOR					m_vecPosition;
	MATRIX4X4				m_matWorld;
	VECTOR					m_vecMoveSpeed;
	VECTOR					m_vecTurnSpeed;
	float					m_fHealth;
	bool					m_bDead;
	CAR_MOD_INFO			m_CarModInfo;
	CHAR					m_szNumberPlate[9];
	bool					m_bDeathHasBeenNotified;
	bool					m_bHasBeenOccupied;
	DWORD					m_dwLastSeenOccupiedTick;
	DWORD					m_dwLastRespawnedTick;
	int						m_iRespawnDelay;
	bool					m_bHasHadUpdate;
	int						m_iInterior;
	char					m_cColor1;
	char					m_cColor2;
	DWORD					m_dwWasteTick;
	DWORD					m_dwPanelDamage;
	DWORD					m_dwDoorDamage;
	BYTE					m_byteLightDamage;

	void Process(float fElapsedTime);

	CVehicle(int iModel,VECTOR *vecPos,float fRotation,int iColor1,int iColor2, int iRespawnTime);
	~CVehicle(){};

	BOOL IsActive() { return m_bIsActive; };
	BOOL IsWasted() { return m_bIsWasted; };

	void SetID(SACMVEHICLE VehicleID) { m_VehicleID = VehicleID; };
	void SetCab(SACMVEHICLE VehicleID) { m_CabID = VehicleID; };
	VEHICLE_SPAWN_INFO * GetSpawnInfo() { return &m_SpawnInfo; };

	void UpdateDamageStatus(DWORD dwPanelDamage, DWORD dwDoorDamage, BYTE byteLightDamage);

	void SpawnForPlayer(SACMPLAYER byteForPlayerID);
	void SetVehicleInterior(int iIntSet) { m_SpawnInfo.byteInterior = iIntSet; };
	void SetDead() { m_bDead = true; }; // Respawns due to death in ~10s
	void SetNumberPlate(PCHAR Plate);
	void CheckForIdleRespawn();
	void Respawn();
	BOOL IsOccupied();
	bool IsATrainPart();

	void GetPosition(PVECTOR vecPosition);
	void SetPosition(VECTOR vecPosition);

	void GetMoveSpeedVector(PVECTOR vecMoveSpeed);
	void SetMoveSpeedVector(VECTOR vecMoveSpeed);

	void GetTurnSpeedVector(PVECTOR vecTurnSpeed);
	void SetTurnSpeedVector(VECTOR vecTurnSpeed);

	float GetHealth();
	void SetHealth(float fHealth);

	bool GetHasHadUpdate();
	void SetHasHadUpdate(bool bHadUpdate);

	SACMPLAYER GetDriver();
	void SetDriver(SACMPLAYER driverId);

	void Reset();

	void Update(SACMPLAYER bytePlayerID, MATRIX4X4 * matWorld, float fHealth, SACMVEHICLE TrailerID);
};

#endif
