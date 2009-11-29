/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


// VisualPainterSP.cpp : Defines the entry point for the application.
//

#include <windows.h>
#include <windowsx.h>
#include <aygshell.h>
#include "resource.h"

#include "MapApp.h"

CMapApp app;

HINSTANCE	g_hInst = NULL;  // Local copy of hInstance
HWND		g_hwndCB = NULL;

#define ARRAYSIZE(a)   (sizeof(a)/sizeof(*a))

const TCHAR* g_szAppWndClass = TEXT("HelloApp");

void Quit()
{
	app.Exit();
	PostQuitMessage(0);
}

/**************************************************************************************

   OnCreate

 **************************************************************************************/
LRESULT OnCreate(
    HWND hwnd,
    CREATESTRUCT* lParam
    )
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
        return(-1);
    }

	g_hwndCB = mbi.hwndMB;

    // Do other window creation related things here.

    return(0); // continue creation of the window
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


/**************************************************************************************

   WndProc

 **************************************************************************************/
LRESULT CALLBACK WndProc(
    HWND hwnd,
    UINT msg,
    WPARAM wp,
    LPARAM lp
    )
{
    LRESULT lResult = TRUE;

	if (msg == WM_KEYDOWN)
	{
		int iChar = MakeScancode(wp, lp);
		int iCommand = app.GetKeymap().Translate(iChar);
		if (iCommand >= 0)
		{
			msg = WM_COMMAND;
			wp = iCommand;
		}
	}

    switch(msg)
    {
		case WM_ACTIVATE:
			{
				bool fActive = (LOWORD(wp) != WA_INACTIVE);
				app.SetActive(fActive);
			}
			break;
        case WM_CREATE:
			{
				lResult = OnCreate(hwnd, (CREATESTRUCT*)lp);
				app.SetMenu(g_hwndCB);
				app.Create(hwnd, L"/My Documents/VSMapViewer");

				SetTimer(hwnd, 1, 1000, 0);
			}
            break;
		case WM_TIMER:
			app.OnTimer();
			break;
        case WM_COMMAND:
			if (!app.ProcessCommand(wp))
				Quit();
			break;

		case WM_PAINT:
			app.Paint();
			break; 

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

		case WM_COPYDATA:
			app.ProcessCmdLine((wchar_t *)(((COPYDATASTRUCT*)(lp))->lpData));
			break;

#ifdef UNDER_CE
		case WM_HIBERNATE:
			app.ProcessWMHIBERNATE();
			break;
#endif // UNDER_CE

		default:
            lResult = DefWindowProc(hwnd, msg, wp, lp);
            break;
    }

    return(lResult);
}

/****************************************************************************

   ActivatePreviousInstance

  ****************************************************************************/
HRESULT ActivatePreviousInstance(
    const TCHAR* pszClass,
    const TCHAR* pszTitle,
    BOOL* pfActivated,
	HWND hwndOwn,
	wchar_t * wstrParam
    )
{
    HRESULT hr = S_OK;
    int cTries;
    HANDLE hMutex = NULL;

    *pfActivated = FALSE;
    cTries = 5;
    while(cTries > 0)
    {
        hMutex = CreateMutex(NULL, FALSE, pszClass); // NOTE: We don't want to own the object.
        if(NULL == hMutex)
        {
            // Something bad happened, fail.
            hr = E_FAIL;
            goto Exit;
        }

        if(GetLastError() == ERROR_ALREADY_EXISTS)
        {
            HWND hwnd;

            CloseHandle(hMutex);
            hMutex = NULL;

            // There is already an instance of this app
            // running.  Try to bring it to the foreground.

            hwnd = FindWindow(pszClass, pszTitle);
            if(NULL == hwnd)
            {
                // It's possible that the other window is in the process of being created...
                Sleep(500);
                hwnd = FindWindow(pszClass, pszTitle);
            }

            if(NULL != hwnd) 
            {
                // Set the previous instance as the foreground window

                // The "| 0x01" in the code below activates
                // the correct owned window of the
                // previous instance's main window.
                SetForegroundWindow((HWND) (((ULONG) hwnd) | 0x01));

                // We are done.
                *pfActivated = TRUE;

				if (wstrParam)
				{
					COPYDATASTRUCT cds;
					cds.cbData = (wcslen(wstrParam) + 1) * sizeof(*wstrParam);
					cds.lpData = wstrParam;
					SendMessage(hwnd, WM_COPYDATA, WPARAM(hwndOwn), LPARAM(&cds));
				}

                break;
            }

            // It's possible that the instance we found isn't coming up,
            // but rather is going down.  Try again.
            cTries--;
        }
        else
        {
            // We were the first one to create the mutex
            // so that makes us the main instance.  'leak'
            // the mutex in this function so it gets cleaned
            // up by the OS when this instance exits.
            break;
        }
    }

    if(cTries <= 0)
    {
        // Someone else owns the mutex but we cannot find
        // their main window to activate.
        hr = E_FAIL;
        goto Exit;
    }

Exit:
    return(hr);
}


/*****************************************************************************

  WinMain

  ***************************************************************************/

int WINAPI WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPWSTR lpCmdLine,
    int nCmdShow
    )
{
    MSG msg;
    HWND hwnd = NULL;
    BOOL fActivated;
    WNDCLASS wc;
    HWND hwndMain;
    TCHAR * szAppTitle = app.GetTitle();

    g_hInst = hInstance;

	app.m_wstrCmdLine = lpCmdLine;
	wchar_t wcBuffer[1000];
	GetModuleFileName(0, wcBuffer, 1000);
	app.m_wstrProgName = wcBuffer;

    if(FAILED(ActivatePreviousInstance(g_szAppWndClass, szAppTitle, &fActivated, 0, lpCmdLine)) ||
            fActivated)
    {
        return(0);
    }

    // Register our main window's class.
    ZeroMemory(&wc, sizeof(wc));
    wc.style = CS_HREDRAW | CS_VREDRAW ;
    wc.lpfnWndProc = (WNDPROC)WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hIcon = NULL;
    wc.hInstance = g_hInst;
    wc.hCursor = NULL;
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = g_szAppWndClass;
    if(!RegisterClass(&wc))
    {
        return(0);
    }

    // Create the main window.    
    hwndMain = CreateWindow(g_szAppWndClass, szAppTitle,
            WS_CLIPCHILDREN, // Setting this to 0 gives a default style we don't want.  Use a benign style bit instead.
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            NULL, NULL, g_hInst, NULL );
    if(!hwndMain)
    {
        return(0);
    }

    ShowWindow(hwndMain, nCmdShow);
    UpdateWindow(hwndMain);

    // Pump messages until a PostQuitMessage.
    while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage (&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}

// end VisualPainterSP.cpp
