//----------------------------------------------------------
//
//   SA:CM Multiplayer Modification For GTA:SA
//   Copyright 2004-2006 SA:CM team
//
//----------------------------------------------------------

#include "../main.h"
extern CGame		*pGame;
extern CNetGame		*pNetGame;
extern CChatWindow  *pChatWindow;

//----------------------------------------------------

void CPickupPool::New(int iModel, float fX, float fY, float fZ, DWORD dwAmmo, SACMPLAYER fromDeadPlayer)
{
	if (m_iPickupCount >= MAX_PICKUPS) return;
	int iPickup;

	for (iPickup = 0; iPickup < MAX_PICKUPS; iPickup++)
	{
		if (m_dwHnd[iPickup] == NULL) break;
	}

	if (iPickup == MAX_PICKUPS) return;

	m_Pickups[iPickup].iModel = iModel;
	m_Pickups[iPickup].iType = 4;
	m_Pickups[iPickup].fX = fX;
	m_Pickups[iPickup].fY = fY;
	m_Pickups[iPickup].fZ = fZ;

	m_dwHnd[iPickup] = pGame->CreateWeaponPickup(iModel, dwAmmo, fX, fY, fZ);
	m_iTimer[iPickup] = NULL;

	m_droppedWeapon[iPickup].bDroppedWeapon = true;
	m_droppedWeapon[iPickup].fromPlayer = fromDeadPlayer;

	m_iPickupCount++;
}

void CPickupPool::New(PICKUP* pPickup, int iPickup)
{
	if (m_iPickupCount >= MAX_PICKUPS || iPickup < 0 || iPickup >= MAX_PICKUPS) return;
	if (m_dwHnd[iPickup] != NULL) ScriptCommand(&destroy_pickup, m_dwHnd[iPickup]);
	memcpy(&m_Pickups[iPickup], pPickup, sizeof(PICKUP));
	m_droppedWeapon[iPickup].bDroppedWeapon = false;
	m_dwHnd[iPickup] = pGame->CreatePickup(pPickup->iModel, pPickup->iType, pPickup->fX, pPickup->fY, pPickup->fZ);
	m_iPickupCount++;
}

void CPickupPool::Destroy(int iPickup)
{
	if (m_iPickupCount <= 0 || iPickup < 0 || iPickup >= MAX_PICKUPS) return;
	if (m_dwHnd[iPickup] != NULL)
	{
		ScriptCommand(&destroy_pickup, m_dwHnd[iPickup]);
		m_dwHnd[iPickup] = NULL;
		m_iTimer[iPickup] = NULL;
		m_iPickupCount--;
	}
}

void CPickupPool::DestroyDropped(SACMPLAYER fromPlayer)
{
	for (int i = 0; i < MAX_PICKUPS; i++)
	{
		if (m_dwHnd[i] != NULL && m_droppedWeapon[i].bDroppedWeapon && m_droppedWeapon[i].fromPlayer == fromPlayer)
		{
			ScriptCommand(&destroy_pickup, m_dwHnd[i]);
			m_dwHnd[i] = NULL;
			m_iTimer[i] = NULL;
			m_iPickupCount--;
		}
	}
}

void CPickupPool::PickedUp(int iPickup)
{
	if (iPickup < 0 || iPickup >= MAX_PICKUPS) return;
	if (m_dwHnd[iPickup] != NULL && m_iTimer[iPickup] == 0) {
		if (m_droppedWeapon[iPickup].bDroppedWeapon) return;

		// Allright, we've got a normal pickup;
		RakNet::BitStream bsPickup;
		bsPickup.Write(iPickup);
		pNetGame->SendRPC(RPC_PickedUpPickup, &bsPickup);
		m_iTimer[iPickup] = 15; // Ignore for about 5-10 seconds
	}
}

void CPickupPool::Process()
{
	for (int i = 0; i < MAX_PICKUPS; i++)
	{
		if (m_dwHnd[i] != NULL)
		{
			if (m_droppedWeapon[i].bDroppedWeapon || m_Pickups[i].iType == 14)
			{
				if (ScriptCommand(&is_pickup_picked_up, m_dwHnd[i]))
				{
#ifdef _DEBUG
					pChatWindow->AddDebugMessage("Picked up %u", i);
#endif

					RakNet::BitStream bsPickup;
					if (m_droppedWeapon[i].bDroppedWeapon) {
						// Other people may not have it in the same slot depending on position
						bsPickup.Write(m_droppedWeapon[i].fromPlayer);
						pNetGame->SendRPC(RPC_PickedUpWeapon, &bsPickup);
					}
					else {
						bsPickup.Write(i);
						pNetGame->SendRPC(RPC_PickedUpPickup, &bsPickup);
					}
				}
			}
			else if (m_iTimer[i] > 0)
			{
				m_iTimer[i] --;
			}
		}
	}
}

CPickupPool::~CPickupPool()
{
	for (int i = 0; i < MAX_PICKUPS; i++)
	{
		if (m_dwHnd[i] != NULL)
		{
			ScriptCommand(&destroy_pickup, m_dwHnd[i]);
		}
	}
}

//----------------------------------------------------