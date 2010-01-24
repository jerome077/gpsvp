/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


// VisualPainterPPC.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "VisualPainterPPC.h"
#include <commctrl.h>
#include <aygshell.h>
#include <sipapi.h>

#include "MapApp.h"

#include "NMEAParser.h"
#include "Devload.h"

struct IPropertyBag;

typedef struct tagNMNEWMENU {
  NMHDR hdr;
  TCHAR szReg[80];
  HMENU hMenu;
  CLSID clsid;
  IPropertyBag** pppropbag;
} NMNEWMENU, *PNMNEWMENU;

CMapApp app;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE			g_hInst = 0;				// The current instance
HWND				g_hwndCB;					// The command bar handle

static SHACTIVATEINFO s_sai;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass	(HINSTANCE, LPTSTR);
BOOL				InitInstance	(HINSTANCE, int);
LRESULT CALLBACK	WndProc			(HWND, UINT, WPARAM, LPARAM);
HWND				CreateRpCommandBar(HWND);

void Quit()
{
	app.m_fExiting = true;
	WaitForSingleObject(app.m_hPortThread, INFINITE);
	CloseHandle(app.m_hPortThread);
	CommandBar_Destroy(g_hwndCB);
	PostQuitMessage(0);
}


int WINAPI WinMain(	HINSTANCE hInstance,
					HINSTANCE hPrevInstance,
					LPTSTR    lpCmdLine,
					int       nCmdShow)
{
	MSG msg;

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    It is important to call this function so that the application 
//    will get 'well formed' small icons associated with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance, LPTSTR szWindowClass)
{
	WNDCLASS	wc;

    wc.style			= CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc		= (WNDPROC) WndProc;
    wc.cbClsExtra		= 0;
    wc.cbWndExtra		= 0;
    wc.hInstance		= hInstance;
    wc.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VISUALPAINTERPPC));
    wc.hCursor			= 0;
    wc.hbrBackground	= (HBRUSH) GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName		= 0;
    wc.lpszClassName	= szWindowClass;

	return RegisterClass(&wc);
}

//
//  FUNCTION: InitInstance(HANDLE, int)
//
//  PURPOSE: Saves instance handle and creates main window
//
//  COMMENTS:
//
//    In this function, we save the instance handle in a global variable and
//    create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	TCHAR	* szTitle;			// The title bar text
	TCHAR	szWindowClass[MAX_LOADSTRING];		// The window class name

	g_hInst = hInstance;		// Store instance handle in our global variable
	// Initialize global strings
	LoadString(hInstance, IDC_VISUALPAINTERPPC, szWindowClass, MAX_LOADSTRING);
	szTitle = app.GetTitle();

	//If it is already running, then focus on the window
	HWND hWnd = FindWindow(szWindowClass, szTitle);	
	if (hWnd) 
	{
		// set focus to foremost child window
		// The "| 0x01" is used to bring any owned windows to the foreground and
		// activate them.
		SetForegroundWindow((HWND)((ULONG) hWnd | 0x00000001));
		return 0;
	} 

	MyRegisterClass(hInstance, szWindowClass);
	
	hWnd = CreateWindow(szWindowClass, szTitle, WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
	if (!hWnd)
	{	
		return FALSE;
	}
	//When the main window is created using CW_USEDEFAULT the height of the menubar (if one
	// is created is not taken into account). So we resize the window after creating it
	// if a menubar is present
	if (g_hwndCB)
    {
		RECT rc;
        RECT rcMenuBar;

		GetWindowRect(hWnd, &rc);
        GetWindowRect(g_hwndCB, &rcMenuBar);
		rc.bottom -= (rcMenuBar.bottom - rcMenuBar.top);
		
		MoveWindow(hWnd, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, FALSE);
	}

	SIPINFO si;
	si.cbSize = sizeof(si);
	SHSipInfo(SPI_GETSIPINFO, 0, &si, 0);
	si.fdwFlags &= !SIPF_ON;
	si.fdwFlags |= SIPF_OFF;
	SHSipInfo(SPI_SETSIPINFO, 0, &si, 0);


	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

int MakeScancode(WPARAM wParam, LPARAM lParam)
{
	return wParam;
}

std::wstring FormatKey(int nScancode)
{
	wchar_t buff[10];
	if (isdigit(nScancode))
		swprintf(buff, L"%c", nScancode);
	else if (nScancode == 38)
		swprintf(buff, L"up");
	else if (nScancode == 40)
		swprintf(buff, L"down");
	else if (nScancode == 37)
		swprintf(buff, L"left");
	else if (nScancode == 39)
		swprintf(buff, L"right");
	else if (nScancode == 13)
		swprintf(buff, L"action");
	else if (nScancode == 0)
		swprintf(buff, L"");
	else if (nScancode == 119)
		swprintf(buff, L"*");
	else if (nScancode == 120)
		swprintf(buff, L"#");
	else
		swprintf(buff, L"#%d", nScancode);
	return buff;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId;

	if (message == WM_KEYDOWN)
	{
		if (lParam < 0x1000000)
		{
			int iCommand = app.GetKeymap().Translate(MakeScancode(wParam, lParam));
			if (iCommand >= 0)
			{
				message = WM_COMMAND;
				wParam = iCommand;
			}
		}
	}
	
	switch (message) 
	{
		case WM_LBUTTONDOWN:
			{
				SHRGINFO info;
				info.cbSize = sizeof(info);
				info.hwndClient = hWnd;
				info.ptDown.x = LOWORD(lParam);
				info.ptDown.y = HIWORD(lParam);
				info.dwFlags = SHRG_RETURNCMD;
				if (SHRecognizeGesture(&info) == GN_CONTEXTMENU)
				{
					app.ContextMenu(ScreenPoint(LOWORD(lParam), HIWORD(lParam)));
					break;
				};
				app.OnLButtonDown(ScreenPoint(LOWORD(lParam), HIWORD(lParam)));
				break;
			}
		case WM_LBUTTONUP:
			app.OnLButtonUp(ScreenPoint(LOWORD(lParam), HIWORD(lParam)));
			break;
		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			if (!app.ProcessCommand(wmId))
				Quit();
			break;
		case WM_ACTIVATE:
			{
				// Notify shell of our activate message
				SHHandleWMActivate(hWnd, wParam, lParam, &s_sai, FALSE);
				bool fActive = (LOWORD(wParam) != WA_INACTIVE);
				app.SetActive(fActive);
			}
			break;
		case WM_CREATE:
			{
				g_hwndCB = CreateRpCommandBar(hWnd);
				memset (&s_sai, 0, sizeof (s_sai));
				s_sai.cbSize = sizeof (s_sai);
				HWND hMenuMB = (HWND)SHFindMenuBar(hWnd);
				app.SetMenu(hMenuMB);
				app.Create(hWnd, L"/My Documents/VSMapViewer/");
				SetTimer(hWnd, 1, 1000, 0);
#ifdef UNDER_CE
#	if UNDER_CE >= 0x0500
				app.RegisterRoamNotify();
#	endif
#endif
			}
			break;
#ifdef UNDER_CE
#	if UNDER_CE >= 0x0500
		case UM_REGNOTIFY:
			app.OnRoamNotify();
			break;
#	endif
#endif
		case WM_TIMER:
			app.OnTimer();
			break;
		case WM_PAINT:
			app.Paint();
			break; 
		case WM_DESTROY:
#ifdef UNDER_CE
#	if UNDER_CE >= 0x0500
			app.UnregisterRoamNotify();
#	endif
#endif
			Quit();
			break;
		case WM_SETTINGCHANGE:
			SHHandleWMSettingChange(hWnd, wParam, lParam, &s_sai);
     		break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

HWND CreateRpCommandBar(HWND hwnd)
{
    // create the menu bar
    SHMENUBARINFO mbi;
    ZeroMemory(&mbi, sizeof(SHMENUBARINFO));
    mbi.cbSize = sizeof(SHMENUBARINFO);
    mbi.hwndParent = hwnd;
    mbi.nToolBarId = IDR_TEMPLATE_MENUBAR_MENU;
    mbi.hInstRes = g_hInst;
    if(!SHCreateMenuBar(&mbi))
    {
        // Couldn't create the menu bar.  Fail creation of the window.
        return NULL;
    }

	return mbi.hwndMB;
}