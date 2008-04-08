/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef MENU_H
#define MENU_H

#include "Keymap.h"
#include "ScreenButtons.h"

class CMenu : protected IListAcceptor
{
private:
	CMenu(const CMenu &);
	CMenu & operator = (const CMenu &);
	HMENU m_hMenu;
	bool m_fCreated;
	list<CMenu*> m_listSubMenus;
	CKeymap m_Keymap;
	CScreenButtons m_buttons;
	CMenu * m_pOwner;
public:
	CMenu()
	{
		m_fCreated = false;
		m_hMenu = 0;
		m_pOwner = 0;
	}
	~CMenu()
	{
		if (m_fCreated && m_hMenu)
		{
			DestroyMenu(m_hMenu);
			m_hMenu = 0;
			m_fCreated = false;
		}
	}
	void Init(HMENU hMenu)
	{
		m_hMenu = hMenu;
	}
	void Init()
	{
		m_fCreated = true;
		m_hMenu = CreatePopupMenu();
	}
	CMenu & CreateSubMenu(const wchar_t * wcLabel)
	{
		HMENU hSubMenu = CreatePopupMenu();
		AppendMenu(m_hMenu, MF_STRING | MF_POPUP | MF_ENABLED, UINT(hSubMenu), wcLabel);
		m_listSubMenus.push_back(new CMenu);
		m_listSubMenus.back()->Init(hSubMenu);
		m_listSubMenus.back()->SetOwner(this);
		return *m_listSubMenus.back();
	}
	void CreateItem(const wchar_t * wcLabel, int iCommand)
	{
		if (iCommand >= 0)
		{
			GetKeymap().AddAction(iCommand, wcLabel);
			GetButtons().AddAction(iCommand, wcLabel);
			AppendMenu(m_hMenu, MF_STRING | MF_ENABLED, iCommand, wcLabel);
		}
		else
			AppendMenu(m_hMenu, MF_STRING | MF_GRAYED, 0, wcLabel);
	}
	void CreateBreak()
	{
		AppendMenu(m_hMenu, MF_SEPARATOR, 0, 0);
	}
	int Popup(int x, int y, HWND hWnd)
	{
		POINT p;
		p.x = x;
		p.y = y;
		::ClientToScreen(hWnd, &p);
		return TrackPopupMenu(m_hMenu, TPM_RETURNCMD | TPM_TOPALIGN, p.x, p.y, 0, hWnd, 0);
	}
	CKeymap & GetKeymap()
	{
		if (m_pOwner)
			return m_pOwner->GetKeymap();
		return m_Keymap;
	}
	CScreenButtons & GetButtons()
	{
		if (m_pOwner)
			return m_pOwner->GetButtons();
		return m_buttons;
	}
	void SetOwner(CMenu * pOwner)
	{
		m_pOwner = pOwner;
	}
	void CheckMenuItem(int iId, bool fCheck)
	{
		::CheckMenuItem(m_hMenu, iId, MF_BYCOMMAND | (fCheck ? MF_CHECKED : MF_UNCHECKED));
	}
	void EnableMenuItem(int iId, bool fCheck)
	{
		::EnableMenuItem(m_hMenu, iId, MF_BYCOMMAND | (fCheck ? MF_ENABLED : MF_GRAYED));
	}
protected:
	virtual void AddItem(const wchar_t * wcLabel, int iId)
	{
		CreateItem(wcLabel, iId);
	}
	virtual void UpdateCurrent(const wchar_t * wcLabel)
	{
	}
public:
	IListAcceptor * GetListAcceptor()
	{
		return this;
	}
};

#endif // MENU_H