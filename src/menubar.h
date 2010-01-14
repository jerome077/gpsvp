/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef MENUBAR_H_INCLUDED
#define MENUBAR_H_INCLUDED

#include "Menu.h"
#include "ResourceCommon.h"
#include <map>

struct HWND__;
typedef HWND__ * HWND;

class CMenuBar
{
	HWND m_hwnd;
	HMENU m_hMenu;
	CMenu m_mMenu;
	std::map<int, int> m_Commands;
public:
	void SetMenuBar(HWND hwnd)
	{
#if defined(UNDER_CE) && !defined(BARECE)
		m_hwnd = hwnd;
		HMENU hMenu = (HMENU)SendMessage(m_hwnd, SHCMBM_GETSUBMENU, 0, IDC_RIGHT);
		DeleteMenu(hMenu, 0, MF_BYPOSITION);
		SetMenu(hMenu);
#endif // UNDER_CE
	}
	void SetItemLabelAndCommand(int iItem, wchar_t * wcLabel, int iCommand)
	{
#if defined(UNDER_CE) && UNDER_CE >= 0x0500 || defined(SMARTPHONE)
		TBBUTTONINFO tbbi;
		ZeroMemory(&tbbi, sizeof(tbbi));
		tbbi.cbSize = sizeof(tbbi);
		tbbi.dwMask = TBIF_TEXT;
		tbbi.pszText = wcLabel;
		SendMessage(m_hwnd, TB_SETBUTTONINFO, iItem, LPARAM(&tbbi));
		if (iCommand)
			m_Commands[iItem] = iCommand;
#else // UNDER_CE
		if (iCommand)
			m_mMenu.CreateItem(wcLabel, iCommand);
#endif // UNDER_CE
	}
	int CheckCommand(int iItem)
	{
		if (m_Commands.find(iItem) != m_Commands.end())
			return m_Commands[iItem];
		return iItem;
	}
	void SetMenu(HMENU hMenu)
	{
		m_hMenu = hMenu;
		m_mMenu.Init(hMenu);
	}
	CMenu & GetMenu() { return m_mMenu; }
	CKeymap & GetKeymap() {return m_mMenu.GetKeymap(); }
	CScreenButtons & GetButtons() { return m_mMenu.GetButtons(); }
};

#endif // MENUBAR_H_INCLUDED
