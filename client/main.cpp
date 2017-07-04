#include "main.h"
#include "game/util.h"
#include "filehooks.h"
#include <aclapi.h>

CNetGame					*pNetGame = 0;
CChatWindow					*pChatWindow = 0;
CCmdWindow					*pCmdWindow = 0;
CDeathWindow				*pDeathWindow = 0;
CSpawnScreen				*pSpawnScreen = 0;
CFontRender					*pDefaultFont = 0;

int							iGtaVersion = 0;
GAME_SETTINGS				tSettings;

BOOL						bGameInited = FALSE;
BOOL						bNetworkInited = FALSE;

BOOL						bQuitGame = FALSE;
DWORD						dwStartQuitTick = 0;

IDirect3D9					*pD3D;
IDirect3DDevice9			*pD3DDevice = NULL;

HANDLE						hInstance = 0;
CNewPlayerTags				*pNewPlayerTags = NULL;
CScoreBoard					*pScoreBoard = NULL;
CLabel						*pLabel = NULL;
CHelpDialog					*pHelpDialog = NULL;

bool						bShowDebugLabels = false;

CGame						*pGame = 0;
DWORD						dwGraphicsLoop = 0;

CFileSystem					*pFileSystem = NULL;

CDXUTDialogResourceManager	*pDialogResourceManager = NULL;
CDXUTDialog					*pGameUI = NULL;

void d3d9RestoreDeviceObjects();
void d3d9DestroyDeviceObjects();
BOOL SubclassGameWindow();
void SetupCommands();
void TheGraphicsLoop();
void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext);
LONG WINAPI exc_handler(_EXCEPTION_POINTERS* exc_inf);

DWORD dwOrgRwSetState = 0;
DWORD dwSetStateCaller = 0;
DWORD dwSetStateOption = 0;
DWORD dwSetStateParam = 0;
char dbgstr[256];

void SwitchWindowedMode();

// backwards
//----------------------------------------------------

extern void InstallGameAndGraphicsLoopHooks();
extern void InitScripting();

// polls the game until it's able to run.
void LaunchMonitor(PVOID v)
{
	pGame = new CGame();
	pGame->InitGame();

	while (1) {
		if (*(PDWORD)ADDR_ENTRY == 7) {
			pGame->StartGame();
			break;
		}
		else {
			Sleep(5);
		}
	}

	ExitThread(0);
}

//----------------------------------------------------

int fexist(char *filename) {
	struct stat buffer;
	return (stat(filename, &buffer) == 0);
}

//----------------------------------------------------

#define ARCHIVE_FILE "sacm-client.arc"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (DLL_PROCESS_ATTACH == fdwReason) {
		hInstance = hinstDLL;
		InitSettings();

		if (tSettings.bDebug || tSettings.bPlayOnline) {
			SetUnhandledExceptionFilter(exc_handler);
			dwGraphicsLoop = (DWORD)TheGraphicsLoop;

			pFileSystem = new CArchiveFS();
			if (!pFileSystem->Load(ARCHIVE_FILE) || !fexist(ARCHIVE_FILE)) {
				MessageBox(0,
					"I can't find archive file.\nPlease put 'sacm-client.arc' in GTA San Andreas dir",
					"Archive", MB_OK | MB_ICONEXCLAMATION);
				ExitProcess(1);
			}

			InstallFileSystemHooks();

			_beginthread(LaunchMonitor, 0, NULL);
		}
	} else if (DLL_PROCESS_DETACH == fdwReason) {
		if (tSettings.bDebug || tSettings.bPlayOnline) {
			UninstallFileSystemHooks();
		}
	}

	return TRUE;
}

//----------------------------------------------------

DWORD dwFogEnabled = 0;
DWORD dwFogColor = 0x00FF00FF;
BOOL gDisableAllFog = FALSE;

void SetupD3DFog(BOOL bEnable)
{
	float fFogStart = 500.0f;
	float fFogEnd = 700.0f;

	if (gDisableAllFog) bEnable = FALSE;

	if (pD3DDevice) {
		pD3DDevice->SetRenderState(D3DRS_FOGENABLE, bEnable);
		//pD3DDevice->SetRenderState(D3DRS_FOGCOLOR, dwFogColor);
		pD3DDevice->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
		pD3DDevice->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
		//pD3DDevice->SetRenderState(D3DRS_FOGSTART, *(DWORD*)(&fFogStart));
		//pD3DDevice->SetRenderState(D3DRS_FOGEND, *(DWORD*)(&fFogEnd));
	}
}

//----------------------------------------------------

void _declspec(naked) RwRenderStateSetHook()
{
	_asm mov eax, [esp]
		_asm mov dwSetStateCaller, eax
	_asm mov eax, [esp + 4]
		_asm mov dwSetStateOption, eax
	_asm mov eax, [esp + 8]
		_asm mov dwSetStateParam, eax

	if (dwSetStateOption == 14) {
		if (dwSetStateParam) {
			SetupD3DFog(TRUE);
			dwFogEnabled = 1;
		}
		else {
			SetupD3DFog(FALSE);
			dwFogEnabled = 0;
		}
		_asm mov[esp + 8], 0; no fog
	}

	_asm mov eax, dwOrgRwSetState
	_asm jmp eax
}

//----------------------------------------------------

void HookRwRenderStateSet()
{
	DWORD dwNewRwSetState = (DWORD)RwRenderStateSetHook;

	_asm mov ebx, 0xC97B24
	_asm mov eax, [ebx]
		_asm mov edx, [eax + 32]
		_asm mov dwOrgRwSetState, edx
	_asm mov edx, dwNewRwSetState
	_asm mov[eax + 32], edx

#ifdef _DEBUG
	sprintf(dbgstr, "HookRwRenderStateSet(0x%X)", dwOrgRwSetState);
	OutputDebugString(dbgstr);
#endif

}

//----------------------------------------------------

void CallRwRenderStateSet(int state, int option)
{
	_asm push option
	_asm push state
	_asm mov ebx, 0xC97B24
	_asm mov eax, [ebx]
		_asm call dword ptr[eax + 32]
		_asm add esp, 8
}

//----------------------------------------------------

void SetupGameUI()
{
	if (pGameUI) SAFE_DELETE(pGameUI);

	pGameUI = new CDXUTDialog();
	pGameUI->Init(pDialogResourceManager);

	int iFontSize;
	if (pGame->GetScreenWidth() < 1024) {
		iFontSize = 16;
	}
	else if (pGame->GetScreenWidth() == 1024) {
		iFontSize = 18;
	}
	else {
		iFontSize = 20;
	}
	pGameUI->SetFont(0, "Arial", iFontSize, FW_BOLD);

	pGameUI->SetCallback(OnGUIEvent);
	pGameUI->SetSize(pGame->GetScreenWidth(), pGame->GetScreenHeight());

	if (pChatWindow) pChatWindow->ResetDialogControls(pGameUI);
	if (pCmdWindow) pCmdWindow->ResetDialogControls(pGameUI);

}

//----------------------------------------------------

bool bCursor = false;

void DoInitStuff()
{
	// GAME INIT
	if (!bGameInited)
	{
		timeBeginPeriod(1); // increases the accuracy of Sleep()
		SubclassGameWindow();

		// Grab the real IDirect3D9 * from the game.
		pD3D = (IDirect3D9 *)pGame->GetD3D();

		// Grab the real IDirect3DDevice9 * from the game.
		pD3DDevice = (IDirect3DDevice9 *)pGame->GetD3DDevice();
		*(IDirect3DDevice9Hook**)ADDR_ID3D9DEVICE = new IDirect3DDevice9Hook();

		// Create instances of the chat and input classes.
		pDefaultFont = new CFontRender(pD3DDevice);
		pChatWindow = new CChatWindow(pD3DDevice, pDefaultFont->m_pD3DFont);
		pCmdWindow = new CCmdWindow(pD3DDevice);

		// DXUT GUI INITIALISATION
		pDialogResourceManager = new CDXUTDialogResourceManager();
		pDialogResourceManager->OnCreateDevice(pD3DDevice);
		pDialogResourceManager->OnResetDevice();

		SetupGameUI();

		if (tSettings.bPlayOnline) {
			pDeathWindow = new CDeathWindow(pD3DDevice);
			pSpawnScreen = new CSpawnScreen(pD3DDevice);
			pNewPlayerTags = new CNewPlayerTags(pD3DDevice);
			pScoreBoard = new CScoreBoard(pD3DDevice, TRUE);
			pHelpDialog = new CHelpDialog(pD3DDevice);

			pGame->ToggleThePassingOfTime(0);
		}

		pLabel = new CLabel(pD3DDevice, "Verdana", false);


		// Setting up the commands.
		SetupCommands();
		HookRwRenderStateSet();

		if (tSettings.bDebug) {
			CCamera *pGameCamera = pGame->GetCamera();
			pGameCamera->Restore();
			pGameCamera->SetBehindPlayer();
			pGame->DisplayHud(TRUE);

			if (pChatWindow) {
				pChatWindow->AddDebugMessage("SA:CM " SACM_VERSION " Debug Started");
			}
		}

		bGameInited = TRUE;
		return;
	}

	// NET GAME INIT
	//////////////////////////////////////////////////////////////////////////////////
	if (!bNetworkInited && tSettings.bPlayOnline) {
		pNetGame = new CNetGame(tSettings.szConnectHost, atoi(tSettings.szConnectPort),
			tSettings.szNickName, tSettings.szConnectPass);

		bNetworkInited = TRUE;
		return;
	}
}

//----------------------------------------------------

DWORD dwLastFrameRateCheck = 0;
DWORD dwFrameTime = 0;

/*IDirect3DSurface9* mouseImage = NULL;
bool bMouseLoaded = false;*/

bool bFirst = true;
POINT pCursor;
bool bCursorPosSet = false;

void TheGraphicsLoop()
{
	_asm pushad // because we're called from a hook

	DoInitStuff();

	SetupD3DFog(TRUE);

	// Process the netgame if it's active.
	if (pNetGame) {
		pNetGame->Process();
		pGame->ForceFrameLimiterOn();
	}

	// We have to call the real Render2DStuff
	// because we overwrote its call to get here.
	_asm popad

	_asm mov edx, ADDR_RENDER2DSTUFF
	_asm call edx

	_asm pushad

	if (bQuitGame) {
		if ((GetTickCount() - dwStartQuitTick) > 1000) {
			if (pNetGame)
			{
				delete pNetGame;
				pNetGame = NULL;
			}
			ExitProcess(0);
		}
		_asm popad
		return;
	}

	if (pGame) {
		pGame->ProcessInputDisabling();
	}

	_asm popad
}

//----------------------------------------------------

void QuitGame()
{
	if (pNetGame && pNetGame->GetGameState() == GAMESTATE_CONNECTED) {
		pNetGame->GetRakClient()->Shutdown(500);
	}
	bQuitGame = TRUE;
	dwStartQuitTick = GetTickCount();
}

//----------------------------------------------------

void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext)
{
	switch (nControlID)
	{
	case IDC_CMDEDIT:
	{
		if (nEvent == EVENT_EDITBOX_STRING) {
			if (pCmdWindow) pCmdWindow->ProcessInput();
		}
		break;
	}
	}
	return;
}

//----------------------------------------------------

void InitSettings()
{
	PCHAR szCmdLine = GetCommandLineA();

	OutputDebugString(szCmdLine);
	OutputDebugString("\n");

	memset(&tSettings, 0, sizeof(GAME_SETTINGS));

	while (*szCmdLine) {

		if (*szCmdLine == '-' || *szCmdLine == '/') {
			szCmdLine++;
			switch (*szCmdLine) {
			case 'd':
				tSettings.bDebug = TRUE;
				tSettings.bPlayOnline = FALSE;
				break;
			case 'c':
				tSettings.bPlayOnline = TRUE;
				tSettings.bDebug = FALSE;
				break;
			case 'z':
				szCmdLine++;
				SetStringFromCommandLine(szCmdLine, tSettings.szConnectPass);
				break;
				/*
				// We'll do this using ALT+ENTER
				case 'w':
					tSettings.bWindowedMode = TRUE;
					break;
				*/
			case 'h':
				szCmdLine++;
				SetStringFromCommandLine(szCmdLine, tSettings.szConnectHost);
				break;
			case 'p':
				szCmdLine++;
				SetStringFromCommandLine(szCmdLine, tSettings.szConnectPort);
				break;
			case 'n':
				szCmdLine++;
				SetStringFromCommandLine(szCmdLine, tSettings.szNickName);
				break;
			}
		}

		szCmdLine++;
	}
}


//----------------------------------------------------

void SetStringFromCommandLine(char *szCmdLine, char *szString)
{
	while (*szCmdLine == ' ') szCmdLine++;
	while (*szCmdLine &&
		*szCmdLine != ' ' &&
		*szCmdLine != '-' &&
		*szCmdLine != '/')
	{
		*szString = *szCmdLine;
		szString++; szCmdLine++;
	}
	*szString = '\0';
}

//----------------------------------------------------

void d3d9DestroyDeviceObjects()
{
	pDialogResourceManager->OnDestroyDevice();

	if (pNewPlayerTags) {
		pNewPlayerTags->DeleteDeviceObjects();
	}

	if (pScoreBoard) {
		pScoreBoard->DeleteDeviceObjects();
	}

	if (pLabel) {
		pLabel->DeleteDeviceObjects();
	}

	if (pDefaultFont) {
		pDefaultFont->DeleteDeviceObjects();
	}

	if (pSpawnScreen) {
		pSpawnScreen->DeleteDeviceObjects();
	}

	if (pDeathWindow && pDeathWindow->m_pD3DFont) {
		pDeathWindow->m_pD3DFont->OnLostDevice();
	}

	if (pDeathWindow && pDeathWindow->m_pWeaponFont) {
		pDeathWindow->m_pWeaponFont->OnLostDevice();
	}

	if (pDeathWindow && pDeathWindow->m_pSprite) {
		pDeathWindow->m_pSprite->OnLostDevice();
	}

	if (pChatWindow && pChatWindow->m_pChatTextSprite) {
		pChatWindow->m_pChatTextSprite->OnLostDevice();
	}

	pDialogResourceManager->OnLostDevice();
}

void d3d9RestoreDeviceObjects()
{
	if (pDialogResourceManager) {
		pDialogResourceManager->OnResetDevice();
	}

	if (pNewPlayerTags) {
		pNewPlayerTags->RestoreDeviceObjects();
	}

	if (pScoreBoard) {
		pScoreBoard->RestoreDeviceObjects();
	}

	if (pLabel) {
		pLabel->RestoreDeviceObjects();
	}

	if (pDefaultFont) {
		pDefaultFont->RestoreDeviceObjects();
	}

	if (pSpawnScreen) {
		pSpawnScreen->RestoreDeviceObjects();
	}

	if (pDeathWindow && pDeathWindow->m_pD3DFont) {
		pDeathWindow->m_pD3DFont->OnResetDevice();
	}

	if (pDeathWindow && pDeathWindow->m_pWeaponFont) {
		pDeathWindow->m_pWeaponFont->OnResetDevice();
	}

	if (pDeathWindow && pDeathWindow->m_pSprite) {
		pDeathWindow->m_pSprite->OnResetDevice();
	}

	if (pChatWindow && pChatWindow->m_pChatTextSprite) {
		pChatWindow->m_pChatTextSprite->OnResetDevice();
	}

	pDialogResourceManager->OnCreateDevice(pD3DDevice);
}

//----------------------------------------------------