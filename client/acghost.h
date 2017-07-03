#pragma once

#include "main.h"
#include "memorymodule/MemoryModule.h"

typedef struct _AC_GHOST_ENTRY
{
	bool bAsDLLModule;
	HMEMORYMODULE hGhostModule;
} AC_GHOST_ENTRY;

class CACGhost
{
private:
	BOOL m_bEnabled;
	DWORD m_dwCount;
	AC_GHOST_ENTRY *m_pGhosts;
	DWORD m_dwServerIndex;

	void InitializeAsArchive(RakNet::BitStream &bs);

public:
	CACGhost();
	~CACGhost();

	BOOL DownloadAndInitialize();
};