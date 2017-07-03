//----------------------------------------------------------
//
//   SA:MP Multiplayer Modification For GTA:SA
//   Copyright 2004-2006 SA:MP team
//
//----------------------------------------------------------

#pragma once

#ifndef ARCTOOL
	#include "../main.h"
#else
	#include <windows.h>
#endif

#include "CryptoContext.h"

class CHasher
{
private:
	static DWORD ms_dwHashAlgorithm;
	
	HCRYPTHASH m_hCryptHash;
	CCryptoContext* m_pContext;

public:
	CHasher(CCryptoContext* pContext);
	~CHasher(void);

	void AddData(DWORD dwDataLength, BYTE* pbData);
	HCRYPTHASH GetContainer();

};
