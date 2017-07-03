/*

SA:CM Multiplayer Modification
Copyright 2004-2005 SA:CM Team

Version: $Id: gangzonepool.cpp,v 1.0 2007/05/25 19:26:45 Y_Less Exp $

Based on original hook by Peter

*/

#include "main.h"
extern CNetGame *pNetGame;

#define RGBA_ABGR(n) (((n >> 24) & 0x000000FF) | ((n >> 8) & 0x0000FF00) | ((n << 8) & 0x00FF0000) | ((n << 24) & 0xFF000000))

//----------------------------------------------------

CGangZonePool::CGangZonePool()
{
	for (WORD wZone = 0; wZone < MAX_GANG_ZONES; wZone++)
	{
		m_bSlotState[wZone] = FALSE;
	}
}

WORD CGangZonePool::New(float fMinX, float fMinY, float fMaxX, float fMaxY)
{
	WORD wZone = 0;
	while (wZone < MAX_GANG_ZONES)
	{
		if (!m_bSlotState[wZone]) break;
		wZone++;
	}
	if (wZone == MAX_GANG_ZONES) return 0xFFFF;
	m_fGangZone[wZone][0] = fMinX;
	m_fGangZone[wZone][1] = fMinY;
	m_fGangZone[wZone][2] = fMaxX;
	m_fGangZone[wZone][3] = fMaxY;
	m_bSlotState[wZone] = TRUE;
	return wZone;
}

void CGangZonePool::Delete(WORD wZone)
{
	m_bSlotState[wZone] = FALSE;
	RakNet::BitStream bsParams;
	bsParams.Write(wZone);
	pNetGame->SendRPC(RPC_ScrRemoveGangZone, &bsParams, -1, TRUE);
}

void CGangZonePool::ShowForPlayer(BYTE bytePlayer, WORD wZone, DWORD dwColor)
{
	RakNet::BitStream bsParams;
	bsParams.Write(wZone);
	bsParams.Write(m_fGangZone[wZone][0]);
	bsParams.Write(m_fGangZone[wZone][1]);
	bsParams.Write(m_fGangZone[wZone][2]);
	bsParams.Write(m_fGangZone[wZone][3]);
	dwColor = RGBA_ABGR(dwColor);
	bsParams.Write(dwColor);
	pNetGame->SendRPC(RPC_ScrAddGangZone, &bsParams, bytePlayer, FALSE);
}

void CGangZonePool::ShowForAll(WORD wZone, DWORD dwColor)
{
	RakNet::BitStream bsParams;
	bsParams.Write(wZone);
	bsParams.Write(m_fGangZone[wZone][0]);
	bsParams.Write(m_fGangZone[wZone][1]);
	bsParams.Write(m_fGangZone[wZone][2]);
	bsParams.Write(m_fGangZone[wZone][3]);
	dwColor = RGBA_ABGR(dwColor);
	bsParams.Write(dwColor);
	pNetGame->SendRPC(RPC_ScrAddGangZone, &bsParams, -1, TRUE);
}

void CGangZonePool::HideForPlayer(BYTE bytePlayer, WORD wZone)
{
	RakNet::BitStream bsParams;
	bsParams.Write(wZone);
	pNetGame->SendRPC(RPC_ScrRemoveGangZone, &bsParams, bytePlayer, FALSE);
}

void CGangZonePool::HideForAll(WORD wZone)
{
	RakNet::BitStream bsParams;
	bsParams.Write(wZone);
	pNetGame->SendRPC(RPC_ScrRemoveGangZone, &bsParams, -1, TRUE);
}

void CGangZonePool::FlashForPlayer(BYTE bytePlayer, WORD wZone, DWORD dwColor)
{
	RakNet::BitStream bsParams;
	bsParams.Write(wZone);
	dwColor = RGBA_ABGR(dwColor);
	bsParams.Write(dwColor);
	pNetGame->SendRPC(RPC_ScrFlashGangZone, &bsParams, bytePlayer, FALSE);
}

void CGangZonePool::FlashForAll(WORD wZone, DWORD dwColor)
{
	RakNet::BitStream bsParams;
	bsParams.Write(wZone);
	dwColor = RGBA_ABGR(dwColor);
	bsParams.Write(dwColor);
	pNetGame->SendRPC(RPC_ScrFlashGangZone, &bsParams, -1, TRUE);
}

void CGangZonePool::StopFlashForPlayer(BYTE bytePlayer, WORD wZone)
{
	RakNet::BitStream bsParams;
	bsParams.Write(wZone);
	pNetGame->SendRPC(RPC_ScrStopFlashGangZone, &bsParams, bytePlayer, FALSE);
}

void CGangZonePool::StopFlashForAll(WORD wZone)
{
	RakNet::BitStream bsParams;
	bsParams.Write(wZone);
	pNetGame->SendRPC(RPC_ScrStopFlashGangZone, &bsParams, -1, TRUE);
}
