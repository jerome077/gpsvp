/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef DIALOGS_H
#define DIALOGS_H

#include <windows.h>
#include <Commctrl.h>
#if defined(UNDER_CE) && !defined(BARECE)
#	include <aygshell.h>
#	include <tpcshell.h>
#endif // UNDER_CE
#include <map>
#include "Common.h"

#include "Lock.h"
#include "MenuBar.h"

extern int MakeScancode(WPARAM wParam, LPARAM lParam);
extern HINSTANCE g_hInst;

LRESULT CALLBACK MADlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK MASubWindowProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

class CMADialog;
typedef CMADialog * PMADialog;
extern volatile PMADialog g_pNextDialog;
extern map<HWND, CMADialog *> g_Dialogs;

class CControl
{
protected:
	HWND m_hControl;
	int MySendMessage(UINT msg, WPARAM wp, LPARAM lp)
	{
		return SendMessage(m_hControl, msg, wp, lp);
	}
public:
	HWND HWnd() {return m_hControl;}
	void SetFocus() {::SetFocus(m_hControl);}
	void SetText(const wchar_t * wcText);
	void GetText(wchar_t * buffer, int size);
	int GetFontHeight(HWND hDlg, HFONT hFont)
	{
		TEXTMETRIC m;
		HDC h = GetDC(hDlg);
		SelectObject(h, hFont);
		GetTextMetrics(h, &m);
		ReleaseDC(hDlg, h);
		return m.tmHeight;
	}
	BOOL GetWindowRect(LPRECT prc)
	{
		return ::GetWindowRect(m_hControl, prc);
	}
	virtual BOOL SetWindowPos(int x, int y, int dx, int dy)
	{
		return ::SetWindowPos(m_hControl, 0, x, y, dx, dy, 
			SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
	}
};

class CCombo : public CControl
{
	int AddString(wchar_t * string, bool fSelect);
	HWND m_hControl2;
public:
	void Create(HWND hDlg, bool fSort);
	void AddItem(wchar_t * string, const wchar_t * select);
	int AddItem(wchar_t * string);
	void AddItem(int i, int select);
	int GetCurSel();
	void Select(int i);
	virtual BOOL SetWindowPos(int x, int y, int dx, int dy)
	{
		BOOL res = ::SetWindowPos(m_hControl, 0, x, y, dx * 9 / 10, dy, 
			SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
		res = res && ::SetWindowPos(m_hControl2, 0, x + dx * 9 / 10, y, dx - dx * 9 / 10, dy, 
			SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
		return res;
	}
	void GetText(wchar_t * buffer, int size);
};

class CEditText : public CControl
{
public:
	void Create(HWND hDlg);
	void SetInt(int i);
	int GetInt(BOOL * fTranslated);
};

class CText : public CControl
{
public:
	CText();
	CText(HWND hDlg, const wchar_t * wcText);
	void Create(HWND hDlg, const wchar_t * wcText);
};

class CListView : public CControl, public IListAcceptor, public IListAcceptor2
{
private:
	int m_iCurrent;
public:
	void Create(HWND hDlg, bool fSort)
	{
		m_hControl = CreateWindow(WC_LISTVIEW, L"ListView", 
					WS_CHILD | LVS_REPORT | LVS_NOCOLUMNHEADER | WS_BORDER | 
					WS_TABSTOP | WS_VISIBLE
					| (fSort ? LVS_SORTASCENDING : 0),
					0, 0, 100, 100, hDlg, 0, g_hInst, 0);	
	}
	virtual void AddItem(const wchar_t * wcLabel, int iId)
	{
		AddItem(wcLabel, iId, 0, iId);
	}
	virtual void UpdateCurrent(const wchar_t * wcLabel)
	{
		int iSelected = MySendMessage(LVM_GETSELECTIONMARK, 0, 0);
		LVITEM item;
		memset(&item, 0, sizeof(item));
		item.mask = LVIF_TEXT;
		item.iItem = iSelected;
		item.pszText = const_cast<wchar_t *>(wcLabel);
		MySendMessage(LVM_SETITEMTEXT, iSelected, (LPARAM)&item);
	}
	virtual int AddItem(const wchar_t * wcLabel, int iId, int iSubItem, int lParam) 
	{
		LVITEM item;
		memset(&item, 0, sizeof(item));
		item.mask = LVIF_TEXT | LVIF_PARAM;

		item.iItem = iId;
		item.iSubItem = iSubItem;
		item.lParam = lParam ? lParam : iId;
		item.pszText = const_cast<wchar_t *>(wcLabel);
		if (iSubItem)
			return MySendMessage(LVM_SETITEMTEXT, iId, (LPARAM)&item);
		else
			return MySendMessage(LVM_INSERTITEM, 0, (LPARAM)&item);
	}
	virtual void UpdateCurrent(const wchar_t * wcLabel, int iSubItem)
	{
		LVITEM item;
		memset(&item, 0, sizeof(item));
		item.mask = LVIF_TEXT;
		item.iItem = m_iCurrent;
		item.iSubItem = iSubItem;
		item.pszText = const_cast<wchar_t *>(wcLabel);
		MySendMessage(LVM_SETITEMTEXT, m_iCurrent, (LPARAM)&item);
	}
	void Clear()
	{
		MySendMessage(LVM_DELETEALLITEMS, 0, 0);
	}
	void Update(IListAcceptor2Acceptor * pAcceptor)
	{
		int iCount = MySendMessage(LVM_GETITEMCOUNT, 0, 0);
		for (m_iCurrent = 0; m_iCurrent < iCount; ++m_iCurrent)
		{
			LVITEM lv;
			ZeroMemory(&lv, sizeof(lv));
			lv.mask = LVIF_TEXT | LVIF_PARAM;
			wchar_t buff[1000];
			lv.iItem = m_iCurrent;
			lv.pszText = buff;
			lv.cchTextMax = sizeof(buff);
			MySendMessage(LVM_GETITEM, 0, LPARAM(&lv));

			pAcceptor->UpdateCurrent(lv.lParam, this);
		}
	}
	int GetCurSelParam()
	{
		int iSelected = MySendMessage(LVM_GETSELECTIONMARK, 0, 0);
		if (iSelected >= 0)
		{
			LVITEM lv;
			ZeroMemory(&lv, sizeof(lv));
			lv.mask = LVIF_TEXT | LVIF_PARAM;
			wchar_t buff[1000];
			lv.iItem = iSelected;
			lv.pszText = buff;
			lv.cchTextMax = sizeof(buff);
			MySendMessage(LVM_GETITEM, 0, LPARAM(&lv));
			iSelected = lv.lParam;
		}
		return iSelected;
	}
	void RemoveSelected()
	{
		int iSelected = MySendMessage(LVM_GETSELECTIONMARK, 0, 0);
		if (iSelected >= 0)
			MySendMessage(LVM_DELETEITEM, iSelected, 0);
	}
	void AddColumn(wchar_t * wcName, int width, int subItem)
	{
		LVCOLUMN lvcName;
		lvcName.mask = LVCF_FMT | LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
		lvcName.fmt = LVCFMT_LEFT;
		lvcName.pszText = wcName;
		lvcName.cx = width;
		lvcName.iSubItem = subItem;
		MySendMessage(LVM_INSERTCOLUMN, subItem, LPARAM(&lvcName));
	}
};

class CMADialog
{
	RECT m_rectWin;
	int m_buttonX;
	int m_buttonY;
	int m_itemY;
	enum {iHeight = 40, iWidth = 55};
	int m_iPadding;
	void AddItem(HWND hDlg, HWND hItem)
	{
		RECT rectItem;
		GetWindowRect(hItem, &rectItem);
		BOOL res = SetWindowPos(hItem, 0, m_iPadding, m_itemY, 
			m_rectWin.right - m_rectWin.left - m_iPadding * 2, 
			rectItem.bottom - rectItem.top, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
		m_itemY += rectItem.bottom - rectItem.top + m_iPadding;
	}
protected:
	CMenuBar m_MenuBar;
	HWND m_hDialog;
	WNDPROC m_pOldSubWindowProc;
public:
	CMADialog()
	{
		AutoLock l;
		m_pOldSubWindowProc = 0;
		m_iPadding = 1;
	}
	virtual void InitDialog(HWND hDlg) 
	{
	};
	virtual void WindowPosChanged(HWND hDlg) {};
	virtual void Command(HWND hDlg, int iCommand) 
	{
		if (iCommand == IDOK)
			EndDialog(hDlg, 0);
	};
	virtual int Translate(int iChar);
	LRESULT Process(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	virtual void Notify(NMHDR * pMessage)
	{
	};
	void SetSoftkeybar(HWND hDlg, UINT id)
	{
#if defined(UNDER_CE)
#if !defined(BARECE)
		SHMENUBARINFO mbi;
		ZeroMemory(&mbi, sizeof(SHMENUBARINFO));
		mbi.cbSize = sizeof(SHMENUBARINFO);
		mbi.hwndParent = hDlg;
		mbi.nToolBarId = id;
		mbi.hInstRes = g_hInst;

		BOOL fRes = SHCreateMenuBar(&mbi);
#	ifdef SMARTPHONE
		// Override back key since we have an edit control 
		SendMessage(mbi.hwndMB, SHCMBM_OVERRIDEKEY, VK_TBACK, 
            MAKELPARAM (SHMBOF_NODEFAULT | SHMBOF_NOTIFY, 
                        SHMBOF_NODEFAULT | SHMBOF_NOTIFY));
#	endif
		m_MenuBar.SetMenuBar(mbi.hwndMB);
#endif
#else
		SetMenu(hDlg, LoadMenu(g_hInst, MAKEINTRESOURCE(id)));
		m_MenuBar.SetMenu(GetMenu(hDlg));
#endif
	}
	void AddItem(HWND hDlg, int IDC)
	{
		 AddItem(hDlg, GetDlgItem(hDlg, IDC));
	}
	void AddItem(HWND hDlg, CControl & control)
	{
		RECT rectItem;
		control.GetWindowRect(&rectItem);
		control.SetWindowPos(m_iPadding, m_itemY, 
			m_rectWin.right - m_rectWin.left - m_iPadding * 2, 
			rectItem.bottom - rectItem.top);
		m_itemY += rectItem.bottom - rectItem.top + m_iPadding;
	}
	void AddList(HWND hList)
	{
		BOOL res = SetWindowPos(hList, 0, 0, m_itemY,
			m_rectWin.right - m_rectWin.left, 
			m_rectWin.bottom - m_rectWin.top - m_itemY, 
			0);
		SetFocus(hList);
	}
	void CommonInit(HWND hDlg)
	{
#ifdef UNDER_CE
#if !defined(BARECE)
		SHINITDLGINFO shidi;

		// Create a Done button and size it.  
		shidi.dwMask = SHIDIM_FLAGS;
		shidi.dwFlags = SHIDIF_DONEBUTTON | SHIDIF_SIZEDLGFULLSCREEN;
		shidi.hDlg = hDlg;
		SHInitDialog(&shidi);

		m_itemY = 0;
#endif
#else
		m_buttonX = 4;
		m_buttonY = 4;
		m_itemY = 4;
		GetClientRect(hDlg, &m_rectWin);
		WindowPosChanged(hDlg);
#endif // UNDER_CE
	}
	virtual LRESULT ProcessSubWindowMessage(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	void SetSubWindowProc(HWND hwndList)
	{
		g_Dialogs[hwndList] = this;
		m_pOldSubWindowProc = (WNDPROC)SetWindowLong(hwndList, GWL_WNDPROC, (LONG)MASubWindowProc);
	}
	void ProcessUpDown(int iItem, int iCommand);
	void ProcessUpDown(HWND hControl, int iCommand);
};

#endif // DIALOGS_H
