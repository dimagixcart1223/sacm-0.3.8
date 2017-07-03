//----------------------------------------------------------
//
//   SA:CM Multiplayer Modification For GTA:SA
//   Copyright 2004-2006 SA:CM team
//
//----------------------------------------------------------

#pragma once

class CRemoteActor
{
public:
	class CActorPed		*m_pActorPed;
	class CVehicle		*m_pCurrentVehicle;
	BYTE				m_byteUpdateFromNetwork;
	BYTE				m_byteState;
	SACMVEHICLE			m_VehicleID;
	BYTE				m_byteSeatID;
	float				m_fReportedHealth;
	float				m_fReportedArmour;
	DWORD				m_dwWaitForEntryExitAnims;

	class CActorPed * GetAtPed() { return m_pActorPed; };

	void SetState(BYTE byteState) {	
		if(byteState != m_byteState) {
			//StateChange(byteState,m_byteState);
			m_byteState = byteState;	
		}
	};

	BYTE GetState() { return m_byteState; };

	CRemoteActor();
	~CRemoteActor();

	void Process();

	void EnterVehicle(SACMVEHICLE VehicleID, int iSeat);
	void ExitVehicle();

	void MoveTo(VECTOR vecPos, int iMoveType);

	void HandleActorPedStreaming();
	void HandleVehicleEntryExit();

	BOOL Spawn(int ActorID, int iSkin, VECTOR * vecPos, float fRotation);

};

//----------------------------------------------------