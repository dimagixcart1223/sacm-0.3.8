#include "acghost.h"
#include "net/netgame.h"
#include <windows.h>
#include <wininet.h>

#define DOWNLOAD_URL		"https://auth.sa-mp.com/eg/"
#define DOWNLOAD_N_URL		"https://auth%d.sa-mp.com/eg/"
#define DOWNLOAD_MAX_URL	5
#define DOWNLOAD_AGENT		"SAMP/0.2.2"
#define DOWNLOAD_HEADERS	"User-Agent: " DOWNLOAD_AGENT

extern CNetGame *pNetGame;
extern HANDLE hInstance;

typedef void (*InitializeGhost_t)(RakPeerInterface* pRak);
typedef void (*InitializeGhost2_t)(RakPeerInterface* pRak, HANDLE hSampHandle, DWORD dwServerIndex);

CACGhost::CACGhost()
{
	m_bEnabled = FALSE;
	m_dwCount = 0;
	m_pGhosts = NULL;
	m_dwServerIndex = 0;
}

CACGhost::~CACGhost()
{
	for(DWORD i=0; i<m_dwCount; i++)
	{
		if (m_pGhosts[i].hGhostModule && m_pGhosts[i].bAsDLLModule)
			MemoryFreeLibrary(m_pGhosts[i].hGhostModule);
	}
	delete[] m_pGhosts;
}

void CACGhost::InitializeAsArchive(RakNet::BitStream &bs)
{
	if (m_dwCount)
		return;

	DWORD dwLength = bs.GetNumberOfBytesUsed();
	PBYTE pbData = bs.GetData();

	CArchiveFS* afsArchive = new CArchiveFS(4, 0);
	if (afsArchive->Load((PBYTE)pbData, dwLength))
	{
		DWORD dwIndex = afsArchive->GetFileIndex(0x67c673b8);		// "pl"
		if (dwIndex != FS_INVALID_FILE) {
			BYTE *pFileData = afsArchive->GetFileData(dwIndex);

			m_dwCount = 1;
			m_pGhosts = new AC_GHOST_ENTRY[m_dwCount];
			m_pGhosts[0].hGhostModule = MemoryLoadLibrary(pFileData);
			m_pGhosts[0].bAsDLLModule = true;

			afsArchive->UnloadData(dwIndex);

			InitializeGhost2_t InitGhost2 = (InitializeGhost2_t)MemoryGetProcAddress(m_pGhosts[0].hGhostModule, "Init2");
			if (InitGhost2 != NULL)
			{
				InitGhost2(pNetGame->GetRakClient(), hInstance, m_dwServerIndex);
			}
			else
			{
				InitializeGhost_t InitGhost = (InitializeGhost_t)MemoryGetProcAddress(m_pGhosts[0].hGhostModule, "Init");
				if (InitGhost != NULL)
					InitGhost(pNetGame->GetRakClient());
			}

		}
	}
	delete afsArchive;
}

BOOL CACGhost::DownloadAndInitialize()
{
	
#ifdef _DEBUG
	
	HANDLE hACFile = CreateFile("sampac.dll", 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
	if (hACFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hACFile);

		HMODULE hMod = LoadLibrary("sampac.dll");

		InitializeGhost2_t InitGhost2 = (InitializeGhost2_t)GetProcAddress(hMod, "Init2");
		if (InitGhost2 != NULL)
		{
			InitGhost2(pNetGame->GetRakClient(), hInstance, 1);
		}
		else
		{
			InitializeGhost_t InitGhost = (InitializeGhost_t)GetProcAddress(hMod, "Init");
			if (InitGhost != NULL)
				InitGhost(pNetGame->GetRakClient());
		}

		return TRUE;
	}
	else
	{

#endif

	RakNet::BitStream bs;
	BYTE pbBuffer[1024];
	DWORD dwBytesRead = 0;
	char szDownloadURL[64];

	// Download stuff
	HINTERNET hInt = InternetOpen(DOWNLOAD_AGENT, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, NULL);		/* INTERNET_FLAG_ASYNC */
	if (!hInt || hInt == INVALID_HANDLE_VALUE)
		return FALSE;

	for(int i=1; i<=DOWNLOAD_MAX_URL; i++)
	{
		if (i == 1)
			strcpy(szDownloadURL, DOWNLOAD_URL);
		else
			sprintf(szDownloadURL, DOWNLOAD_N_URL, i);

		HINTERNET hFile = InternetOpenUrl(hInt, szDownloadURL, DOWNLOAD_HEADERS, -1L, 
							INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | 
							INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_NO_COOKIES | INTERNET_FLAG_NO_UI |
							INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD | INTERNET_FLAG_SECURE, NULL);

		if (!hFile || hFile == INVALID_HANDLE_VALUE)
		{
			// Failed, lets just try the next
			InternetCloseHandle(hInt);
		}
		else
		{
			// Succcess, lets download the data, and break out of the loop
			m_dwServerIndex = i;

			do
			{
				InternetReadFile(hFile, pbBuffer, sizeof(pbBuffer), &dwBytesRead);
				if (dwBytesRead)
				{
					bs.Write((const char*)pbBuffer, dwBytesRead);
				}
			} while(dwBytesRead != 0);
			
			InternetCloseHandle(hFile);

			break;
		}
	}

	InternetCloseHandle(hInt);

	// Initialize it

	DWORD dwLength = (DWORD)bs.GetNumberOfBytesUsed();
	if (dwLength > 4)
	{	
		InitializeAsArchive(bs);

		return (m_dwCount > 0);
	}
	else
	{
		return FALSE;
	}

#ifdef _DEBUG

	}	// (hACFile != INVALID_HANDLE_VALUE) else block

#endif

}