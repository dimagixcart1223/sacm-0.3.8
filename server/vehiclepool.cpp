#include "main.h"

//----------------------------------------------------

CVehiclePool::CVehiclePool()
{
	// loop through and initialize all net players to null and slot states to false
	for(SACMVEHICLE VehicleID = 0; VehicleID < MAX_VEHICLES; VehicleID++) {
		m_bVehicleSlotState[VehicleID] = FALSE;
		m_pVehicles[VehicleID] = NULL;
	}
	//New(419, &pos, 15.0, -1, -1, 18000);
}

//----------------------------------------------------

CVehiclePool::~CVehiclePool()
{	
	for(SACMVEHICLE VehicleID = 0; VehicleID != MAX_VEHICLES; VehicleID++) {
		Delete(VehicleID);
	}
}

//----------------------------------------------------

SACMVEHICLE CVehiclePool::New(int iVehicleType,
					   VECTOR * vecPos, float fRotation,
					   int iColor1, int iColor2, int iRespawnDelay)
{
	for(SACMVEHICLE vehicleId = 0; vehicleId < MAX_VEHICLES; vehicleId++)
	{
		if(!GetSlotState(vehicleId))
		{
			/*logprintf("CVehiclePool::New(%u,%u,%.2f,%.2f,%.2f,%.2f,%d,%d)",
			vehicleId, byteVehicleType,vecPos->X,vecPos->Y,vecPos->Z,fRotation,iColor1,iColor2);*/

			m_pVehicles[vehicleId] = new CVehicle(iVehicleType,vecPos,fRotation,iColor1,iColor2,iRespawnDelay);

			if(m_pVehicles[vehicleId])
			{
				m_pVehicles[vehicleId]->SetID(vehicleId);
				m_bVehicleSlotState[vehicleId] = TRUE;
				return vehicleId;
			}
		}	
	}
	return 0xFFFF;
}

//----------------------------------------------------

BOOL CVehiclePool::Delete(SACMVEHICLE VehicleID)
{
	if(!GetSlotState(VehicleID) || !m_pVehicles[VehicleID])
	{
		return FALSE; // Vehicle already deleted or not used.
	}

	m_bVehicleSlotState[VehicleID] = FALSE;
	delete m_pVehicles[VehicleID];
	m_pVehicles[VehicleID] = NULL;

	return TRUE;
}

//----------------------------------------------------

void CVehiclePool::Process(float fElapsedTime)
{
	for (int i=0; i<MAX_VEHICLES; i++)
	{
		if (GetSlotState(i) == TRUE)
		{
			GetAt(i)->Process(fElapsedTime);
		}
	}
}

//----------------------------------------------------

void CVehiclePool::InitForPlayer(SACMPLAYER bytePlayerID)
{	
	for(SACMVEHICLE x = 0; x < MAX_VEHICLES; x++)
	{
		if(GetSlotState(x))
		{
			if(m_pVehicles[x]->IsActive())
			{
				m_pVehicles[x]->SpawnForPlayer(bytePlayerID);
			}
		}
	}
}

//----------------------------------------------------

void CVehiclePool::SetVehicleVirtualWorld(SACMVEHICLE VehicleID, BYTE byteVirtualWorld)
{
	if (VehicleID >= MAX_VEHICLES) return;
	
	m_byteVirtualWorld[VehicleID] = byteVirtualWorld;
	// Tell existing players it's changed
	RakNet::BitStream bsData;
	bsData.Write(VehicleID); // player id
	bsData.Write(byteVirtualWorld); // vw id
	pNetGame->SendRPC(RPC_ScrSetVehicleVirtualWorld , &bsData, -1, TRUE);
}
	
//----------------------------------------------------
