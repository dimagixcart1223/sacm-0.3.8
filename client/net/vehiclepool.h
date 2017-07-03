//----------------------------------------------------------
//
//   SA:CM Multiplayer Modification For GTA:SA
//   Copyright 2004-2006 SA:CM team
//
//----------------------------------------------------------

#pragma once

#include "../main.h"

#define INVALID_VEHICLE_ID			0xFFFF
#define UNOCCUPIED_SYNC_RADIUS		70.0f
#define MAX_VEHICLE_WAITING_SLOTS	100

class CVehiclePool
{
private:
	
	BOOL				m_bVehicleSlotState[MAX_VEHICLES];
	CVehicle			*m_pVehicles[MAX_VEHICLES];
	VEHICLE_TYPE		*m_pGTAVehicles[MAX_VEHICLES];
	BOOL				m_bIsActive[MAX_VEHICLES];
	BOOL				m_bIsWasted[MAX_VEHICLES];
	int					m_iRespawnDelay[MAX_VEHICLES];
	SACMPLAYER			m_LastUndrivenID[MAX_VEHICLES];

	VEHICLE_SPAWN_INFO	m_NewVehicleWaiting[MAX_VEHICLE_WAITING_SLOTS];
	BOOL				m_bWaitingSlotState[MAX_VEHICLE_WAITING_SLOTS];

public:

	VECTOR				m_vecSpawnPos[MAX_VEHICLES];
	
	CVehiclePool();
	~CVehiclePool();

	BOOL New(VEHICLE_SPAWN_INFO *pNewVehicle);

	BOOL Delete(SACMVEHICLE VehicleID);	
	
	// Retrieve a vehicle
	CVehicle* GetAt(SACMVEHICLE VehicleID) {
		if(VehicleID >= MAX_VEHICLES || !m_bVehicleSlotState[VehicleID]) { return NULL; }
		return m_pVehicles[VehicleID];
	};

	// Find out if the slot is inuse.
	BOOL GetSlotState(SACMVEHICLE VehicleID) {
		if(VehicleID >= MAX_VEHICLES) { return FALSE; }
		return m_bVehicleSlotState[VehicleID];
	};

	SACMPLAYER GetLastUndrivenID(SACMVEHICLE VehicleID) {
		return m_LastUndrivenID[VehicleID];
	};

	void SetLastUndrivenID(SACMVEHICLE VehicleID, SACMPLAYER PlayerId) {
		m_LastUndrivenID[VehicleID] = PlayerId;
	};

	SACMVEHICLE FindIDFromGtaPtr(VEHICLE_TYPE * pGtaVehicle);

	int FindGtaIDFromID(int iID);
	int FindGtaIDFromGtaPtr(VEHICLE_TYPE * pGtaVehicle);
	void LinkToInterior(SACMVEHICLE VehicleID, int iInterior);

	void ProcessUnoccupiedSync(SACMVEHICLE VehicleID, CVehicle *pVehicle);
	void Process();
	
	void NotifyVehicleDeath(SACMVEHICLE VehicleID);

	int FindNearestToLocalPlayerPed();
	void AssignSpecialParamsToVehicle(SACMVEHICLE VehicleID, BYTE byteObjective, BYTE byteDoorsLocked);

	void ProcessWaitingList();
	void NewWhenModelLoaded(VEHICLE_SPAWN_INFO *pNewVehicle);

};

//----------------------------------------------------
