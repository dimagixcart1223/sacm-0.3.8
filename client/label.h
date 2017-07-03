//----------------------------------------------------------
//
//   SA:CM Multiplayer Modification For GTA:SA
//   Copyright 2004-2006 SA:CM team
//
//----------------------------------------------------------

#pragma once

#include <d3d9.h>
#include <d3dx9.h>

class CLabel
{
private:
	IDirect3DDevice9* m_pDevice;
	ID3DXFont* m_pFont;

	char* m_szFontFace;
	bool m_bFontBold;
public:
	CLabel(IDirect3DDevice9* pDevice, char* szFontFace, bool bFontBold);
	~CLabel();

	void Draw(D3DXVECTOR3* ppos, char* szText, DWORD dwColor);
	void DeleteDeviceObjects();
	void RestoreDeviceObjects();
};