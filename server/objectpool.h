/*

SA:CM Multiplayer Modification
Copyright 2004-2005 SA:CM Team

Version: $Id: vehiclepool.h,v 1.8 2006/04/12 19:26:45 mike Exp $

*/

#ifndef SAMPSRV_OBJECTPOOL_H
#define SAMPSRV_OBJECTPOOL_H

//----------------------------------------------------

class CObjectPool
{
private:

	BOOL m_bObjectSlotState[MAX_OBJECTS];
	CObject *m_pObjects[MAX_OBJECTS];
	
	BOOL m_bPlayerObjectSlotState[MAX_PLAYERS][MAX_OBJECTS];
	BOOL m_bPlayersObject[MAX_OBJECTS];
	CObject *m_pPlayerObjects[MAX_PLAYERS][MAX_OBJECTS];
public:
	CObjectPool();
	~CObjectPool();

	SACMOBJECT New(int iModel, VECTOR * vecPos, VECTOR * vecRot);
	SACMOBJECT New(int iPlayer, int iModel, VECTOR* vecPos, VECTOR* vecRot);
	
	BOOL Delete(SACMOBJECT byteObjectID);	
	BOOL DeleteForPlayer(SACMPLAYER bytePlayerID, SACMOBJECT byteObjectID);
	
	void Process(float fElapsedTime);

	// Retrieve an object by id
	CObject* GetAt(SACMPLAYER byteObjectID)
	{
		if(byteObjectID > MAX_OBJECTS) { return NULL; }
		return m_pObjects[byteObjectID];
	};
	
	CObject* GetAtIndividual(SACMPLAYER bytePlayerID, SACMOBJECT byteObjectID)
	{
		if (byteObjectID > MAX_OBJECTS || bytePlayerID > MAX_PLAYERS) { return NULL; }
		return m_pPlayerObjects[bytePlayerID][byteObjectID];
	};


	// Find out if the slot is inuse.
	BOOL GetSlotState(SACMOBJECT byteObjectID)
	{
		if(byteObjectID > MAX_OBJECTS) { return FALSE; }
		return m_bObjectSlotState[byteObjectID];
	};
	
	// Find out if the slot is inuse by an individual (and not global).
	BOOL GetPlayerSlotState(SACMPLAYER bytePlayerID, SACMOBJECT byteObjectID)
	{
		if (byteObjectID > MAX_OBJECTS || bytePlayerID > MAX_PLAYERS) { return FALSE; }
		//if (m_pObjects[byteObjectID]) return TRUE; // Can't use global slots
		if (!m_bPlayersObject[byteObjectID]) { return FALSE; } // Can use empty ones
		return m_bPlayerObjectSlotState[bytePlayerID][byteObjectID];
	};

	void InitForPlayer(SACMPLAYER bytePlayerID);
};

//----------------------------------------------------

#endif

