/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "Dialogs.h"
#include <resource.h>

#ifdef SMARTPHONE
#	define USE_SPIN_CONTROL
#endif

#ifndef WC_EDIT
#	define WC_EDIT T("edit")
#endif
#ifndef WC_STATIC
#	define WC_STATIC T("Static")
#endif
#ifndef WC_LISTBOX
#	define WC_LISTBOX T("ListBox")
#endif

volatile PMADialog g_pNextDialog = 0;
std::map<HWND, CMADialog *> g_Dialogs;

LRESULT CMADialog::ProcessSubWindowMessage(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_CHAR)
		return TRUE;
	if (message == WM_KEYDOWN)
	{
		int cmd = Translate(MakeScancode(wParam, lParam));
		if (cmd)
		{
			PostMessage(m_hDialog, WM_COMMAND, cmd, 0);
			return TRUE;
		}
	}
	if (m_pOldSubWindowProc)
		return CallWindowProc(m_pOldSubWindowProc, hDlg, message, wParam, lParam);
	else
		return TRUE;
}

int CMADialog::Translate(int iChar)
{
	switch(iChar)
	{
	case '7':
		return IDC_PAGEUP;
	case '9':
		return IDC_PAGEDOWN;
	case 0x0d:
		return IDC_ACTION;
	}
	return 0;
}


void CMADialog::ProcessUpDown(int iItem, int iCommand)
{
	ProcessUpDown(GetDlgItem(m_hDialog, iItem), iCommand);
}

void CMADialog::ProcessUpDown(HWND hControl, int iCommand)
{
	if (iCommand == IDC_PAGEDOWN)
		SendMessage(hControl, WM_KEYDOWN, VK_NEXT, 0);
	else if (iCommand == IDC_PAGEUP)
		SendMessage(hControl, WM_KEYDOWN, VK_PRIOR, 0);
}

LRESULT CMADialog::Process(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
#if defined(UNDER_CE) && !defined(BARECE)
	case WM_HOTKEY:
	/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	Back key will come to use this way because it was requested in
	WM_INITDIALOG
	+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
		{
		   if(VK_TBACK == HIWORD(lParam))
			  {
			  SHSendBackToFocusWindow(message, wParam, lParam);
			  }

	   }
#endif
	case WM_NOTIFY:
		{
			Notify((NMHDR*)lParam);
			return TRUE;
		}
	case WM_INITDIALOG:
		{
			m_hDialog = hDlg;
			CommonInit(hDlg);
			InitDialog(hDlg);
			return TRUE; 
		}
	case WM_WINDOWPOSCHANGED:
		{
			GetClientRect(hDlg, &m_rectWin);
			WindowPosChanged(hDlg);
			return TRUE;
		}
	case WM_COMMAND:
		{
			Command(hDlg, m_MenuBar.CheckCommand(LOWORD(wParam)));
			return TRUE;
		}
	case WM_SETTINGCHANGE:
		{
			//CommonInit(hDlg);
			ResetVScrollBar(hDlg);
			return TRUE;
		}
	case WM_CLOSE:
		{
			EndDialog(hDlg, 0);
			return TRUE;
		}
	case WM_VSCROLL:
		{
			VScroll(hDlg, LOWORD(wParam));
			return TRUE;
		}
	}
	return FALSE;
}

void CMADialog::VScroll(HWND hDlg, int iScrollParam)
{
#ifndef SMARTPHONE
	// Get vertical scroll bar information
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask  = SIF_ALL;
	GetScrollInfo(hDlg, SB_VERT, &si);
	int iCurrentVertPos = si.nPos;

	switch (iScrollParam)
	{
	case SB_TOP:
		si.nPos = si.nMin;
		break;
           
	case SB_BOTTOM:
		si.nPos = si.nMax;
		break;
           
	case SB_LINEUP:
		si.nPos -= 1;
		break;
           
	case SB_LINEDOWN:
		si.nPos += 1;
		break;
           
	case SB_PAGEUP:
		si.nPos -= si.nPage;
		break;
           
	case SB_PAGEDOWN:
		si.nPos += si.nPage;
		break;
           
	case SB_THUMBTRACK:
		si.nPos = si.nTrackPos;
		break;
           
	default:
		break;
	}
	// Set the position and then retrieve it (Windows might change it)
	si.fMask = SIF_RANGE | SIF_POS | SIF_PAGE | SIF_TRACKPOS;
	SetScrollInfo(hDlg, SB_VERT, &si, TRUE);
	GetScrollInfo(hDlg, SB_VERT, &si);

	// Scroll the window
	if (si.nPos != iCurrentVertPos)
	{
	#ifndef UNDER_CE
		ScrollWindow(hDlg, 0, (iCurrentVertPos - si.nPos), NULL, NULL);
	#else
		ScrollWindowEx(hDlg, 0, (iCurrentVertPos - si.nPos), NULL, NULL, NULL, NULL, SW_SCROLLCHILDREN);
	#endif
		UpdateWindow(hDlg);
	}
#endif
	return;
}

void CMADialog::ResetVScrollBar(HWND hDlg)
{
	SCROLLINFO si;
	si.cbSize = sizeof(si);
	si.fMask  = SIF_ALL;
	GetScrollInfo (hDlg, SB_VERT, &si);
	int iCurrentVertPos = si.nPos;

#ifdef SMARTPHONE
	SendMessage(hDlg, DM_RESETSCROLL, false, true);
#else
	si.fMask = SIF_RANGE | SIF_POS | SIF_PAGE | SIF_TRACKPOS;
	si.nMax = m_itemY;
	si.nPos = 0;
	RECT rectItem;
	GetClientRect(hDlg, &rectItem);
	si.nPage = rectItem.bottom - rectItem.top; 
	SetScrollInfo(hDlg, SB_VERT, &si, TRUE);
#endif
	
	// Retrieve the position again (Windows might change it) and scroll the content
	GetScrollInfo(hDlg, SB_VERT, &si);
	if (si.nPos != iCurrentVertPos)
	{
	#ifndef UNDER_CE
		ScrollWindow(hDlg, 0, (iCurrentVertPos - si.nPos), NULL, NULL);
	#else
		ScrollWindowEx(hDlg, 0, (iCurrentVertPos - si.nPos), NULL, NULL, NULL, NULL, SW_SCROLLCHILDREN);
	#endif
		UpdateWindow(hDlg);
	}
}

void CEditText::Create(HWND hDlg)
{
	HFONT f = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0);
	m_hControl = CreateWindow(/*WS_EX_CLIENTEDGE, */WC_EDIT, T("EditText"), 
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | ES_NOHIDESEL | 
		ES_AUTOHSCROLL | WS_TABSTOP,
		0, 0, 100, GetFontHeight(hDlg, f) + 2, hDlg, 0, g_hInst, 0);
	MySendMessage(WM_SETFONT, WPARAM(f), 0);
}
void CControl::SetText(const tchar_t * wcText)
{
	SetWindowText(m_hControl, wcText);
}
void CControl::GetText(tchar_t * buffer, int size)
{
	GetWindowText(m_hControl, buffer, size);
}
void CEditText::SetInt(int i)
{
	SetWindowText(m_hControl, IntToText(i).c_str());
}
int CEditText::GetInt(BOOL * fTranslated)
{
	tchar_t buffer[100];
	tchar_t *end;
	GetWindowText(m_hControl, buffer, 100);
	int i = tcstol(buffer, &end, 10);
	*fTranslated = (buffer[0] != 0) && (*end == 0);
	return i;
}

void CCombo::GetText(tchar_t * buffer, int size)
{
#ifdef USE_SPIN_CONTROL
	int iSel = MySendMessage(LB_GETCURSEL, 0, 0);
	MySendMessage(LB_GETTEXT, iSel, LPARAM(buffer));
#else
	CControl::GetText(buffer, size);
#endif
}

void CCombo::Create(HWND hDlg, bool fSort)
{
	HFONT f = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0);
#ifdef USE_SPIN_CONTROL
	m_hControl = CreateWindow (TEXT("listbox"), NULL, WS_VISIBLE | 
								  WS_BORDER | WS_TABSTOP, 10,17,32,GetFontHeight(hDlg, f)*2 - 1, hDlg,
								  0, g_hInst, 0L);

	m_hControl2 = CreateWindow (UPDOWN_CLASS, NULL, WS_VISIBLE | UDS_HORZ |
									UDS_ALIGNRIGHT | UDS_ARROWKEYS |
									UDS_SETBUDDYINT | UDS_WRAP | UDS_EXPANDABLE,
									0, 0, 0, 0, hDlg, 0, g_hInst, 0L);

	SendMessage (m_hControl2, UDM_SETBUDDY, (WPARAM)m_hControl, 0);

#else
	
	m_hControl = CreateWindow(T("combobox"), 0, 
		WS_CHILD | WS_VISIBLE | WS_BORDER  | WS_VSCROLL | WS_TABSTOP | (fSort ? CBS_SORT : 0) |
		CBS_DROPDOWNLIST,
		10,17,32,GetFontHeight(hDlg, f) * 8, hDlg, 0, g_hInst, 0);

#endif

	MySendMessage(WM_SETFONT, WPARAM(f), 0);
}

void CCombo::Select(int i)
{
#ifdef USE_SPIN_CONTROL
	MySendMessage(LB_SETCURSEL, i, 0);
#else
	MySendMessage(CB_SETCURSEL, i, 0);
#endif
}

int CCombo::AddString(tchar_t * string, bool fSelect)
{
#ifdef USE_SPIN_CONTROL
	int index = MySendMessage(LB_ADDSTRING, 0, LPARAM(string));
#else
	int index = MySendMessage(CB_ADDSTRING, 0, LPARAM(string));
#endif
	if (fSelect)
		Select(index);
	return index;
}

CText::CText()
{
}

CText::CText(HWND hDlg, const tchar_t * wcText)
{
	Create(hDlg, wcText);
}
void CText::Create(HWND hDlg, const tchar_t * wcText)
{
	HFONT f = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0);
	m_hControl = CreateWindow(WC_STATIC, T("Text"), 
		SS_LEFT | WS_CHILD | WS_VISIBLE, 
		0, 0, 100, GetFontHeight(hDlg, f), 
		hDlg, 0, g_hInst, 0);
	SetWindowText(m_hControl, wcText);
	MySendMessage(WM_SETFONT, WPARAM(f), 0);
}

int CCombo::AddItem(tchar_t * buffer)
{
	return AddString(buffer, false);
}

void CCombo::AddItem(int value, int select)
{
	tchar_t buffer[100];
	stprintf(buffer, 100, T("%d"), value);
	AddString(buffer, value == select);
}

void CCombo::AddItem(tchar_t * string, const tchar_t * select)
{
	AddString(string, !wcscmp(string, select));
}

int CCombo::GetCurSel()
{
#ifdef USE_SPIN_CONTROL
	return MySendMessage(LB_GETCURSEL, 0, 0);
#else
	return MySendMessage(CB_GETCURSEL, 0, 0);
#endif
}
