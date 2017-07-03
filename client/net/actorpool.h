//----------------------------------------------------------
//
//   SA:CM Multiplayer Modification For GTA:SA
//   Copyright 2004-2006 SA:CM team
//
//----------------------------------------------------------

#pragma once

#include "../main.h"

#define INVALID_ACTOR_ID	0xFF

typedef struct _ACTOR_SPAWN_INFO
{
	BYTE byteTeam;
	int iSkin;
	VECTOR vecPos;
	float fRotation;
} ACTOR_SPAWN_INFO;

//----------------------------------------------------

class CActorPool
{
private:
	
	BOOL				m_bActorSlotState[MAX_ACTORS];
	CRemoteActor		*m_pActors[MAX_ACTORS];
	CHAR				m_szActorNames[MAX_ACTORS][MAX_ACTOR_NAME+1];
	

public:

	PED_TYPE			*m_pGTAPed[MAX_ACTORS]; // pointers to actual ingame actors.

	ACTOR_SPAWN_INFO	m_SpawnInfo[MAX_ACTORS];
	
	CActorPool();
	~CActorPool();

	BOOL New(SACMACTOR ActorID, int iSkin, VECTOR * vecPos, float fRotation, PCHAR szName);
	BOOL Delete(SACMACTOR ActorID);
	PCHAR GetAtName(SACMACTOR ActorID) { return m_szActorNames[ActorID]; };
	
	// Retrieve an actor
	CRemoteActor* GetAt(SACMACTOR ActorID) {
		if(ActorID >= MAX_ACTORS || !m_bActorSlotState[ActorID]) { return NULL; }
		return m_pActors[ActorID];
	};

	// Find out if the slot is inuse.
	BOOL GetSlotState(SACMACTOR ActorID) {
		if(ActorID >= MAX_ACTORS) { return FALSE; }
		return m_bActorSlotState[ActorID];
	};

	BOOL Spawn(SACMACTOR ActorID);

	SACMACTOR FindIDFromGtaPtr(PED_TYPE * pGtaActor);
	int FindGtaIDFromID(int iID);
	int FindGtaIDFromGtaPtr(PED_TYPE * pGtaActor);
	void Process();
	int FindNearestToLocalPlayerPed();
};

//----------------------------------------------------
