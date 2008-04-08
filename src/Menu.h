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