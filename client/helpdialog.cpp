#include "main.h"
#include <stdio.h>

extern CGame* pGame;
extern GAME_SETTINGS tSettings;
extern CChatWindow *pChatWindow;
extern CFontRender *pDefaultFont;

CHelpDialog::CHelpDialog(IDirect3DDevice9 *pD3DDevice)
{
	m_pD3DDevice = pD3DDevice;
}

void CHelpDialog::Draw()
{
	RECT rect;
	rect.top		= 10;
	rect.right		= pGame->GetScreenWidth() - 150;
	rect.left		= 10;
	rect.bottom		= rect.top + 16;

	pDefaultFont->RenderText("--- SA:CM 0.3.8 ---",rect,0xFFFFFFFF); rect.top += 16; rect.bottom += 16;
	pDefaultFont->RenderText("F1: Помощь",rect,0xFFFFFFFF); rect.top += 16; rect.bottom += 16;
	pDefaultFont->RenderText("TAB: Таб игрока. Просмотры сколько игроков и пинг с счётами",rect,0xFFFFFFFF); rect.top += 16; rect.bottom += 16;
	pDefaultFont->RenderText("F4: Позволяет менять класс в следующий раз",rect,0xFFFFFFFF); rect.top += 16; rect.bottom += 16;
	pDefaultFont->RenderText("F5: Статистика",rect,0xFFFFFFFF); rect.top += 16; rect.bottom += 16;
	pDefaultFont->RenderText("F7: Скрыть/Видить чат",rect,0xFFFFFFFF); rect.top += 16; rect.bottom += 16;
	pDefaultFont->RenderText("F8: Зделать скриншот",rect,0xFFFFFFFF); rect.top += 16; rect.bottom += 16;
	pDefaultFont->RenderText("F9: Переключить deathwindow",rect,0xFFFFFFFF); rect.top += 16; rect.bottom += 16;
	pDefaultFont->RenderText("T/F6: Писать в чат",rect,0xFFFFFFFF); rect.top += 16; rect.bottom += 16;
	pDefaultFont->RenderText("Recruit Key: Enter vehicle as passenger",rect,0xFFFFFFFF); rect.top += 16; rect.bottom += 16;
}
П
