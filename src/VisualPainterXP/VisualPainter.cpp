/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


// gpsVP.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "resource.h"
#include "Commdlg.h"
#include "MapApp.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE g_hInst;								// current instance
TCHAR * szTitle;								// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];								// The title bar text

// Foward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);

CMapApp app;

int APIENTRY wWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPWSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG msg;

	setlocale(LC_ALL, ".ACP");
	setlocale(LC_NUMERIC, "C");

	// Initialize global strings
	szTitle = app.GetTitle();
	LoadString(hInstance, IDC_GPSVP, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	app.m_wstrCmdLine = lpCmdLine;

	wchar_t wcBuffer[1000];
	GetModuleFileName(0, wcBuffer, 1000);
	app.m_wstrProgName = wcBuffer;

	
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
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_GPSVP);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPWSTR)IDR_TEMPLATE_MENUBAR_MENU;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   g_hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

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
	if (isalnum(nScancode))
		swprintf(buff, 10, L"%c", nScancode);
	else if (nScancode == 0)
		swprintf(buff, 10, L"");
	else
		swprintf(buff, 10, L"#%d", nScancode);
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
	int wmId, wmEvent;

	if (message == WM_KEYDOWN)
	{
		int iCommand = app.GetKeymap().Translate(MakeScancode(wParam, lParam));
		if (iCommand >= 0)
		{
			message = WM_COMMAND;
			wParam = iCommand;
		}
	}

	switch (message) 
	{
		case WM_CREATE:
			app.SetMenu(GetMenu(hWnd));
			app.Create(hWnd);
			SetTimer(hWnd, 1, 1000, 0);
			break;
		case WM_TIMER:
			app.OnTimer();
			break;
		case WM_LBUTTONDOWN:
			app.OnLButtonDown(ScreenPoint(LOWORD(lParam), HIWORD(lParam)));
			break;
		case WM_MOUSEMOVE:
			app.OnMouseMove(ScreenPoint(LOWORD(lParam), HIWORD(lParam)));
			break;
		case WM_LBUTTONUP:
			app.OnLButtonUp(ScreenPoint(LOWORD(lParam), HIWORD(lParam)));
			break;
		case WM_RBUTTONDOWN:
			app.ContextMenu(ScreenPoint(LOWORD(lParam), HIWORD(lParam)));
			break;
		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			// Parse the menu selections:
			app.ProcessCommand(wmId);
			break;
		case WM_PAINT:
			app.Paint();
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_MOUSEWHEEL:
			{
				ScreenPoint pt(LOWORD(lParam), HIWORD(lParam));
				ScreenToClient(hWnd, &pt);
				
				int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
				if (zDelta > 0)
					app.m_painter.ZoomIn(pt);
				if (zDelta < 0)
					app.m_painter.ZoomOut(pt);
			}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

