/*

	SA:CM Multiplayer Modification
	Copyright 2004-2005 SA:CM Team

    Version: $Id: vehiclepool.h,v 1.8 2006/04/12 19:26:45 mike Exp $

*/

#ifndef SAMPSRV_VEHICLEPOOL_H
#define SAMPSRV_VEHICLEPOOL_H

//----------------------------------------------------

class CVehiclePool
{
private:
	
	BOOL m_bVehicleSlotState[MAX_VEHICLES];
	CVehicle *m_pVehicles[MAX_VEHICLES];
	BYTE m_byteVirtualWorld[MAX_VEHICLES];

public:
	CVehiclePool();
	~CVehiclePool();

	SACMVEHICLE New(int iVehicleType, VECTOR * vecPos, float fRotation, int iColor1, int iColor2, int iRespawnDelay);

	BOOL Delete(SACMVEHICLE VehicleID);	
		
	// Retrieve a vehicle by id
	CVehicle* GetAt(SACMVEHICLE VehicleID)
	{
		if(VehicleID >= MAX_VEHICLES) { return NULL; }
		return m_pVehicles[VehicleID];
	};

	// Find out if the slot is inuse.
	BOOL GetSlotState(SACMVEHICLE VehicleID)
	{
		if(VehicleID > MAX_VEHICLES) { return FALSE; }
		return m_bVehicleSlotState[VehicleID];
	};

	void InitForPlayer(SACMPLAYER bytePlayerID);

	void Process(float fElapsedTime);

	void SetVehicleVirtualWorld(SACMVEHICLE VehicleID, BYTE byteVirtualWorld);
	
	BYTE GetVehicleVirtualWorld(SACMVEHICLE VehicleID) {
		if (VehicleID >= MAX_VEHICLES) { return 0; }
		return m_byteVirtualWorld[VehicleID];		
	};

};

//----------------------------------------------------


#endif

