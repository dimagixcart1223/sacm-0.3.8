//----------------------------------------------------------
//
//   SA:CM Multiplayer Modification For GTA:SA
//   Copyright 2004-2006 SA:CM team
//
//----------------------------------------------------------

typedef struct _GTA_CONTROLSET
{
	WORD wKeys1[24];
	WORD wKeys2[24];
	BYTE bytePadding1[212];
} GTA_CONTROLSET;

/*
typedef struct _GTA_CONTROLSET
{
DWORD dwFrontPad;
WORD wKeys1[19];
DWORD dwFrontPad2;
WORD wKeys2[19];
WORD wTurnLeftRightAnalog[10];
BYTE bytePadding1[138];
BYTE byteCrouchAnalog[5];
BYTE byteIncrementer;
BYTE bytePadding2[15];
DWORD dwKeyHeld;
} GTA_CONTROLSET;
*/

//-----------------------------------------------------------

void GameKeyStatesInit();
void GameStoreLocalPlayerKeys();
void GameSetLocalPlayerKeys();
void GameStoreRemotePlayerKeys(int iPlayer, GTA_CONTROLSET *pGcsKeyStates);
void GameSetRemotePlayerKeys(int iPlayer);
GTA_CONTROLSET *GameGetInternalKeys();
GTA_CONTROLSET *GameGetLocalPlayerKeys();
GTA_CONTROLSET *GameGetPlayerKeys(int iPlayer);
void GameResetPlayerKeys(int iPlayer);
void GameResetInternalKeys();

//-----------------------------------------------------------
