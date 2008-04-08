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
