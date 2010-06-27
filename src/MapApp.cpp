/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#define INITGUID
#ifndef UNDER_WINE
#	include <winsock2.h>
#	include <ws2tcpip.h>
#endif // UNDEX_WINE
#ifdef UNDER_CE
#	ifndef BARECE
#		include <connmgr.h>
#	endif
#endif

#ifndef IDC_OK
#	include <resource.h>
#endif
#include "MapApp.h"
#ifdef UNDER_CE
#	include <devload.h>
#endif
#include "FileDialogs.h"
#if defined(UNDER_CE) && !defined(BARECE)
#	include <tpcshell.h>
#	include <bthutil.h>
#else
#	include <winuser.h>
#	include <shlobj.h>
#endif

#include <stdio.h>

#include "Commands.h"
#include "Dialogs.h"

#include "menubar.h"

#include "HttpClient.h"
#include "GoogleMaps/GMPainter.h"
#include "VersionNumber.h"
#include "EGM96Geoid.h"

const DWORD MIN_CPUMON_STEP_MS = 900;

inline int sign(int i)
{
	return i ? ((i < 0) ? -1 : 1) : 0;
}

CMenuBar m_MenuBar;

extern HINSTANCE g_hInst;

extern int MakeScancode(WPARAM wParam, LPARAM lParam);

const CVersionNumber g_gpsVPVersion(0, 4, 21);

class CNmeaCommandsDlg : public CMADialog
{
	enum
	{
		dmcSave = 1000
	};
	CListView m_list;
	void FillList()
	{
		m_list.Clear();
		app.m_NMEAParser.GetList(&m_list);
	}
	virtual void InitDialog(HWND hDlg)
	{
		SetWindowText(hDlg, L("NMEA commands"));
		m_list.Create(hDlg, true);
		m_list.AddColumn(L("Command"), 400, 0);
		FillList();
		SetSoftkeybar(hDlg, IDR_TEMPLATE_MENUBAR_2);
		m_MenuBar.SetItemLabelAndCommand(IDC_LEFT, L("Done"), IDOK);
		m_MenuBar.SetItemLabelAndCommand(IDC_RIGHT, L("Save"), dmcSave);
		SetSubWindowProc(m_list.HWnd());
	}
	virtual void WindowPosChanged(HWND hDlg)
	{
		ReinitItemY();
		AddList(m_list.HWnd());
	}
	virtual void Command(HWND hDlg, int iCommand)
	{
		switch (iCommand)
		{
		case IDOK:
			{
				EndDialog(hDlg, 0);
				return;
			}
		case dmcSave:
			{
				wchar_t strFile[MAX_PATH + 1] = {0};
				OPENFILENAME of;
				app.FillOpenFileName(&of, hDlg, L("Text\0*.txt\0"), strFile, false,false);
				if (GetSaveFileName(&of))
				{
					app.m_NMEAParser.SaveCommands(strFile);
				}
				return;
			}
		case IDC_PAGEUP:
		case IDC_PAGEDOWN:
			ProcessUpDown(m_list.HWnd(), iCommand);
			return;
		}
	}
};

class CUnknownPointTypesDlg : public CMADialog
{
	CListView m_list;
	void FillList()
	{
		m_list.Clear();
		app.m_painter.GetUnknownTypes(&m_list);
	}
	virtual void InitDialog(HWND hDlg)
	{
		SetWindowText(hDlg, L("Unknown point types"));
		m_list.Create(hDlg, true);
		m_list.AddColumn(L("Type"), 200, 0);
		FillList();
		SetSoftkeybar(hDlg, IDR_TEMPLATE_MENUBAR_2);
		m_MenuBar.SetItemLabelAndCommand(IDC_LEFT, L("Done"), IDOK);
		SetSubWindowProc(m_list.HWnd());
	}
	virtual void WindowPosChanged(HWND hDlg)
	{
		ReinitItemY();
		AddList(m_list.HWnd());
	}
	virtual void Command(HWND hDlg, int iCommand)
	{
		switch (iCommand)
		{
		case IDOK:
			EndDialog(hDlg, 0);
			return;
		case IDC_PAGEUP:
		case IDC_PAGEDOWN:
			ProcessUpDown(m_list.HWnd(), iCommand);
			return;
		}
	}
};

// Window to display a single property of a waypoint
class CWaypointPropertyDlg : public CMADialog
{
	enum
	{
		dmcCancel = 1000
	};
	std::wstring m_wstrLatitude;
	std::wstring m_wstrLongitude;
	CEditText m_name;
	CEditText m_value;
public:
	CWaypoints::enumWaypointPropNameSpace m_iType;
	std::wstring m_strName;
	std::wstring m_strValue;
	virtual void InitDialog(HWND hDlg)
	{
		SetWindowText(hDlg, L("Property"));

		m_name.Create(hDlg);
		m_value.Create(hDlg);

		switch (m_iType)
		{
		case CWaypoints::nsGPX:
			AddItem(hDlg, CText(hDlg, L("Type: Standard GPX")));
			break;
		case CWaypoints::nsVP:
			AddItem(hDlg, CText(hDlg, L("Type: Specific gpsVP")));
			break;
		case CWaypoints::nsOSM:
			AddItem(hDlg, CText(hDlg, L("Type: OpenStreetMap Tag")));
			break;
		}
		AddItem(hDlg, CText(hDlg, L("Name:")));
		AddItem(hDlg, m_name);
		AddItem(hDlg, CText(hDlg, L("Value:")));
		AddItem(hDlg, m_value);
		
		m_name.SetText(m_strName.c_str());
		m_value.SetText(m_strValue.c_str());
		SetSoftkeybar(hDlg, IDR_TEMPLATE_MENUBAR_2);
		m_MenuBar.SetItemLabelAndCommand(IDC_LEFT, L("Ok"), IDOK);
		m_MenuBar.SetItemLabelAndCommand(IDC_RIGHT, L("Cancel"), dmcCancel);
	}
	virtual void Command(HWND hDlg, int iCommand)
	{
		if (iCommand == IDOK)
		{
			const Int cnMaxStr = 1000;
			wchar_t buff[cnMaxStr] = {0};
			m_name.GetText(buff, cnMaxStr);
			m_strName = buff;
			m_value.GetText(buff, cnMaxStr);
			m_strValue = buff;
			EndDialog(hDlg, TRUE);
		}
		if (iCommand == dmcCancel)
		{
			EndDialog(hDlg, FALSE);
		}
	}
};

// This Window list all the waypoint properties
class CWaypointPropertiesDlg : public CMADialog
{
	std::wstring m_wstrLatitude;
	std::wstring m_wstrLongitude;
	CWaypoints::CPoint m_ClonedPoint;
	std::vector<CText> m_TextControls;
	std::vector<CEditText> m_EditTextControls;
public:
	int m_iWaypoint;

	void SaveControlsToPoint(CWaypoints::CPoint& point)
	{
		CWaypoints::CPointEditor wptEditor = point.GetEditor();
		for(int i=0, iEnd=wptEditor.GetPropertyCount(); i<iEnd; i++)
		{
			std::auto_ptr<CWaypoints::CPointProp> prop (wptEditor.GetPropertyByIndex(i));
			const Int cnMaxStr = 1000;
			wchar_t buff[cnMaxStr] = {0};
			m_EditTextControls[i].GetText(buff, cnMaxStr);
			prop->SetValue(buff);
		}
		wptEditor.Commit();
	}
	void ClearControls()
	{
		for(std::vector<CText>::iterator iter = m_TextControls.begin();
 			iter != m_TextControls.end();
			iter++)
		{
			iter->Destroy();
		}
		for(std::vector<CEditText>::iterator iter = m_EditTextControls.begin();
			iter != m_EditTextControls.end();
			iter++)
		{
			iter->Destroy();
		}
		m_TextControls.clear();
		m_EditTextControls.clear();
		ReinitItemY();
	}
	void RecreateControls(HWND hDlg, CWaypoints::CPoint& point, bool bScrollToBottom)
	{
		ClearControls();
		CWaypoints::CPointEditor wptEditor = point.GetEditor();
		for(int i=0, iEnd=wptEditor.GetPropertyCount(); i<iEnd; i++)
		{
			std::auto_ptr<CWaypoints::CPointProp> prop (wptEditor.GetPropertyByIndex(i));
			std::wstring name = (CWaypoints::nsOSM == prop->Namespace()) ? L"osm:"+prop->Name() : prop->Name();
			m_TextControls.push_back(CText(m_hDialog, name.c_str()));
			AddItem(m_hDialog, m_TextControls.back());
			m_EditTextControls.push_back(CEditText());
			m_EditTextControls.back().Create(m_hDialog);
			AddItem(m_hDialog, m_EditTextControls.back());
			m_EditTextControls.back().SetText(prop->Value().c_str());
		}

		// Activate first or last control:
		if (bScrollToBottom)
			SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)m_EditTextControls.back().HWnd(), TRUE);
		else
		{
			SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)m_EditTextControls.front().HWnd(), TRUE);
			SendMessage(m_EditTextControls.front().HWnd(), EM_SETSEL, 0, -1); // Select all
		}
	}
	virtual void InitDialog(HWND hDlg)
	{
		SetWindowText(hDlg, L("Waypoint properties"));
#ifndef UNDER_CE
		ShowScrollBar(hDlg,SB_VERT,TRUE);
#endif
		CWaypoints::CPoint & p = app.GetWaypoints().ById(m_iWaypoint);
		m_ClonedPoint.Assign(p);
		RecreateControls(hDlg, m_ClonedPoint, false);

		CreateMenu(hDlg, app.GetWaypoints().IsGPX());
	}	
	enum
	{
		dmcCancel = 1000,
		dmcOK,
		dmcWaypointNewOSMTag,
		dmcWaypointDeleteProp,
		dmcWaypointCopyTagsFirst = 2000,
		// 2000-2099 Reserved for waypoint models
		dmcWaypointCopyTagsLast = 2099

	};
	void CreateSubMenuCopyTags(CMenu & basemenu)
	{
		CMenu & mmCopyTags = basemenu.CreateSubMenu(L("Copy Tags"));
		int modelsCount = app.GetWaypoints().GetWaypointModelCount();
		if (0 == modelsCount)
		{
			std::wstring itemName = L("No wpt beginning with ")+WPT_MODEL_PREFIX;
			mmCopyTags.CreateItem(itemName.c_str(), -1);
		}
		else
		{
			for(int i=0; i<modelsCount; i++)
			{
				if (dmcWaypointCopyTagsFirst+i > dmcWaypointCopyTagsLast) break;
				CWaypoints::CPoint & modelPoint = app.GetWaypoints().GetWaypointModel(i);
				mmCopyTags.CreateItem(modelPoint.GetLabel().c_str(), dmcWaypointCopyTagsFirst+i);
			}
		}
	}
	void CreateMenu(HWND hDlg, bool bWithOSM)
	{
		if (!bWithOSM)
		{
			SetSoftkeybar(hDlg, IDR_TEMPLATE_MENUBAR_2);
			m_MenuBar.SetItemLabelAndCommand(IDC_LEFT, L("Ok"), dmcOK);
			m_MenuBar.SetItemLabelAndCommand(IDC_RIGHT, L("Cancel"), dmcCancel);
		}
		else
		{
			SetSoftkeybar(hDlg, IDR_TEMPLATE_MENUBAR_MENU);
			m_MenuBar.SetItemLabelAndCommand(IDC_LEFT, L("Ok"), dmcOK);
			m_MenuBar.SetItemLabelAndCommand(IDC_RIGHT, L("Menu"), 0);
			CMenu & menu = m_MenuBar.GetMenu();
			menu.CreateItem(L("Cancel"), dmcCancel);
			menu.CreateItem(L("Delete tag"), dmcWaypointDeleteProp);
			menu.CreateItem(L("New OSM tag"), dmcWaypointNewOSMTag);
			CreateSubMenuCopyTags(menu);
		}
	}
	void EnableMenu()
	{
		// Not used because I don't know how to detect focus change.
		//LONG idx = GetCurSelItem();
		//bool bDeleteAllowed = false;
		//if (idx >= 0)
		//{
		//	std::auto_ptr<CWaypoints::CPointProp> prop (m_ClonedPoint.GetPropertyByIndex(idx));
		//	bDeleteAllowed = prop->DeleteAllowed();
		//}
		//m_MenuBar.GetMenu().EnableMenuItem(dmcWaypointDeleteProp, bDeleteAllowed);
	}
	virtual int Translate(int iChar)
	{
		switch (iChar)
		{
		case '1':
			return dmcCancel;
		case '2':
			return dmcWaypointDeleteProp;
		case '3':
			return dmcWaypointNewOSMTag;
		}
		return CMADialog::Translate(iChar);
	}
	int GetCurSelItem()
	{
		HWND hFocusedWnd = ::GetFocus();
		for(int i = 0, iEnd = m_EditTextControls.size(); i < iEnd; i++)
		{
			if (m_EditTextControls[i].HWnd() == hFocusedWnd) return i;
		}
		return -1;
	}
	virtual void Command(HWND hDlg, int iCommand)
	{
		switch (iCommand)
		{
		case dmcOK: // OK clicked => Apply changes
			{
				// Block all writes until the end of this code block
				CWaypoints::UpdateZone updateZone(app.GetWaypoints().UpdateZoneForCodeBlock());

				SaveControlsToPoint(m_ClonedPoint);

				// Change the original point
				CWaypoints::CPoint & p = app.GetWaypoints().ById(m_iWaypoint);
				p.Assign(m_ClonedPoint);
				app.GetWaypoints().Write();

				EndDialog(hDlg, TRUE);
				return;
			}
		case dmcCancel:
			{
				EndDialog(hDlg, FALSE);
				return;
			}
		case dmcWaypointDeleteProp:
			{
				LONG idx = GetCurSelItem();
				if (idx >= 0)
				{
					SaveControlsToPoint(m_ClonedPoint);
					CWaypoints::CPointEditor wptEditor = m_ClonedPoint.GetEditor();
					if (!wptEditor.RemovePropertyByIndex(idx))
						MessageBox(0, L("Only OSM Tags can be deleted"), L("Delete Tag"), MB_ICONINFORMATION);
					wptEditor.Commit();
					RecreateControls(hDlg, m_ClonedPoint, false);
				}
				return;
			}
		case dmcWaypointNewOSMTag:
			{
				SaveControlsToPoint(m_ClonedPoint);
				CWaypoints::CPointEditor wptEditor = m_ClonedPoint.GetEditor();
				static CWaypointPropertyDlg dlg;
				g_pNextDialog = &dlg;
				CWaypoints::CPointProp& prop = wptEditor.AddOSMProp();
				dlg.m_iType = prop.Namespace();
				dlg.m_strName = prop.Name();
				dlg.m_strValue = prop.Value();
				if (TRUE == DialogBox(g_hInst, (LPCTSTR)IDD_TEMPLATE, hDlg, (DLGPROC)MADlgProc))
				{
					// change the value in the cloned point
					prop.SetValue(dlg.m_strValue);
					prop.SetName(dlg.m_strName);
				}
				wptEditor.Commit();
				RecreateControls(hDlg, m_ClonedPoint, true);
				return;
			}
		default:
			{
				if ((dmcWaypointCopyTagsFirst <= iCommand) && (iCommand <= dmcWaypointCopyTagsLast))
				{
					CWaypoints::CPoint & modelPoint = app.GetWaypoints().GetWaypointModel(iCommand-dmcWaypointCopyTagsFirst);
					m_ClonedPoint.AssignOSM(modelPoint);
					RecreateControls(hDlg, m_ClonedPoint, false);
				}
				return;
			}
		}
	}
};


class CTeamSettingsDlg : public CMADialog
{
	enum
	{
		dmcCancel = 1000
	};
	CEditText m_channel;
	CEditText m_name;
public:
	virtual void InitDialog(HWND hDlg)
	{
		m_channel.Create(hDlg);
		m_name.Create(hDlg);

		AddItem(hDlg, CText(hDlg, L("Secret channel:")));
		AddItem(hDlg, m_channel);
		AddItem(hDlg, CText(hDlg, L("Your name:")));
		AddItem(hDlg, m_name);
		
		m_channel.SetText(app.m_team.GetChannel().c_str());
		m_name.SetText(app.m_team.GetName().c_str());
		m_channel.SetFocus();
		SetSoftkeybar(hDlg, IDR_TEMPLATE_MENUBAR_2);
		m_MenuBar.SetItemLabelAndCommand(IDC_LEFT, L("Ok"), IDOK);
		m_MenuBar.SetItemLabelAndCommand(IDC_RIGHT, L("Cancel"), dmcCancel);
	}
	virtual void Command(HWND hDlg, int iCommand)
	{
		if (iCommand == IDOK)
		{
			const Int cnMaxStr = 1000;
			wchar_t buff[cnMaxStr] = {0};
			m_channel.GetText(buff, cnMaxStr);
			app.m_team.SetChannel(buff);
			m_name.GetText(buff, cnMaxStr);
			app.m_team.SetName(buff);

			EndDialog(hDlg, TRUE);
		}
		if (iCommand == dmcCancel)
		{
			EndDialog(hDlg, FALSE);
		}
	}
};

class CMapsDlg : public CMADialog
{
	enum
	{
		dmcUnload = 1000,
		dmcToggleActive,
		dmcCenter,
		dmcInfo,
		dmcUnloadAll,
		dmcDump,
	};
	CListView m_list;
	void FillList()
	{
		m_list.Clear();
		app.GetAtlas().GetList(&m_list);
	}
	virtual void InitDialog(HWND hDlg)
	{
		SetWindowText(hDlg, L("Maps"));
		m_list.Create(hDlg, true);
		m_list.AddColumn(L("Filename"), 500, 0);

		FillList();
		SetSoftkeybar(hDlg, IDR_TEMPLATE_MENUBAR_MENU);
		SetSubWindowProc(m_list.HWnd());
		m_MenuBar.SetItemLabelAndCommand(IDC_LEFT, L("Done"), IDOK);
		m_MenuBar.SetItemLabelAndCommand(IDC_RIGHT, L("Menu"), 0);
		CMenu & menu = m_MenuBar.GetMenu();
		menu.CreateItem(L("Unload"), dmcUnload);
		menu.CreateItem(L("Activate/deactivate"), dmcToggleActive);
		menu.CreateItem(L("Center on map"), dmcCenter);
		menu.CreateItem(L("Information"), dmcInfo);
		menu.CreateItem(L("Unload all"), dmcUnloadAll);
		if (app.m_Options[mcoDebugMode])
			menu.CreateItem(L("Dump"), dmcDump);
	}
	virtual void WindowPosChanged(HWND hDlg)
	{
		ReinitItemY();
		AddList(m_list.HWnd());
	}
	virtual int Translate(int iChar)
	{
		switch (iChar)
		{
		case '1':
			return dmcUnload;
		case '2':
			return dmcToggleActive;
		case '3':
			return dmcCenter;
		case '4':
			return dmcInfo;
		case '5':
			return dmcUnloadAll;
		}
		return CMADialog::Translate(iChar);
	}

	virtual void Command(HWND hDlg, int iCommand)
	{
		LONG iSelected = m_list.GetCurSelParam();

		switch (iCommand)
		{
		case IDOK:
			{
				EndDialog(hDlg, 0);
				return;
			}
		case dmcUnload:
			if (iSelected >= 0)
			{
				app.GetAtlas().CloseMapByID(iSelected);
				app.m_painter.Redraw();
				m_list.RemoveSelected();
			}
			return;
		case dmcToggleActive:
			if (iSelected >= 0)
			{
				app.GetAtlas().ToggleActiveByID(iSelected);
				app.m_painter.Redraw();
				app.GetAtlas().GetListUpdateCurrent(iSelected, &m_list);
			}
			return;
		case dmcCenter:
			if (iSelected >= 0)
			{
				app.m_painter.SetView(app.GetAtlas().ById(iSelected).GetCenter(), true);
				EndDialog(hDlg, 0);
			}
			return;
		case dmcInfo:
			if (iSelected >= 0)
			{
				const CIMGFile & img = app.GetAtlas().ById(iSelected);
				std::wstring info;
				info += L("Filename: ");
				info += img.GetFilename();
				info += L"\n";
				info += L("Loaded: ");
				info += img.IsLoaded() ? L("Yes") : L("No");
				info += L"\n";
				MessageBox(0, info.c_str(), L("Map information"), MB_ICONINFORMATION);
			}
			return;
		case dmcUnloadAll:
			if (MessageBox(hDlg, L("Are you sure"), L("Unload all"), MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			{
				app.GetAtlas().CloseAll();
				app.m_painter.Redraw();
				FillList();
			}
			return;
		case dmcDump:
			if (iSelected >= 0)
			{
				const CIMGFile & img = app.GetAtlas().ById(iSelected);
				img.Dump();
			}
			return;
		case IDC_PAGEUP:
		case IDC_PAGEDOWN:
			ProcessUpDown(m_list.HWnd(), iCommand);
			return;
		}
	}
};

class CSearchResultsDlg : public CMADialog
{
	enum
	{
		dmcToMap = 1001,
		dmcClose = 1002,
	};
	CListView m_list;
	void FillList()
	{
		m_list.Clear();
		app.m_Found.GetList(&m_list);
	}
	virtual void InitDialog(HWND hDlg)
	{
		SetWindowText(hDlg, L("Search results"));
		m_list.Create(hDlg, true);
		m_list.AddColumn(L("Place"), 500, 0);
		FillList();
		SetSoftkeybar(hDlg, IDR_TEMPLATE_MENUBAR_2);
		m_MenuBar.SetItemLabelAndCommand(IDC_LEFT, L("To map"), dmcToMap);
		m_MenuBar.SetItemLabelAndCommand(IDC_RIGHT, L("Close"), dmcClose);
		SetSubWindowProc(m_list.HWnd());
	}
	virtual void WindowPosChanged(HWND hDlg)
	{
		ReinitItemY();
		AddList(m_list.HWnd());
	}
	virtual void Command(HWND hDlg, int iCommand)
	{
		switch (iCommand)
		{
		case dmcClose:
			{
				EndDialog(hDlg, 0);
				return;
			}
		case IDOK:
		case dmcToMap:
			{
				int idx = m_list.GetCurSelParam();
				if (idx >= 0)
				{
					app.m_painter.SetView(app.m_Found.GetPointByIDApprox(idx), true);
					EndDialog(hDlg, 0);
				}
				return;
			}
		case IDC_PAGEUP:
		case IDC_PAGEDOWN:
			ProcessUpDown(m_list.HWnd(), iCommand);
			return;
		}
	}
};

class CSearchOSMDlg : public CMADialog
{
	enum
	{
		dmcSearch = 1000,
		dmcClose = 1002,
	};
	CEditText m_query;
	virtual void InitDialog(HWND hDlg)
	{
		SetWindowText(hDlg, L("Search OpenStreetMap.org"));
		m_query.Create(hDlg);
		m_query.SetText(L"");
		SetSoftkeybar(hDlg, IDR_TEMPLATE_MENUBAR_2);
		m_MenuBar.SetItemLabelAndCommand(IDC_LEFT, L("Search"), dmcSearch);
		m_MenuBar.SetItemLabelAndCommand(IDC_RIGHT, L("Close"), dmcClose);
	}
	virtual void WindowPosChanged(HWND hDlg)
	{
		ReinitItemY();
		AddItem(hDlg, m_query);
	}
	virtual void Command(HWND hDlg, int iCommand)
	{
		switch (iCommand)
		{
		case dmcClose:
			{
				EndDialog(hDlg, 0);
				return;
			}
		case IDOK:
		case dmcSearch:
			{
				wchar_t wquery[1000];
				m_query.GetText(wquery, 1000);
				if (wcslen(wquery) == 0)
					return;
				char query[3000];
				int res = WideCharToMultiByte(CP_UTF8, 0, wquery, wcslen(wquery), query, 3000, 0, 0);
				query[res] = 0;
				char quoted[9000];
				char * to = quoted;
				for (char * from = query; *from; ++from)
				{
					if ((*from >= '0' && *from <= '9') || (*from >='A' && *from <= 'Z') || (*from >= 'a' && *from <= 'b'))
						*(to++) = *from;
					else
						to += sprintf(to, "%%%02X", (unsigned char)*from);
				}
				(*to) = 0;
				std::string url = "http://gazetteer.openstreetmap.org/namefinder/search.xml?find=";
				url += quoted;
				app.SetSearchURL(url.c_str());
				EndDialog(hDlg, 0);
				return;
			}
		}
	}
};

class CTracksDlg : public CMADialog
{
	enum
	{
		dmcUnload = 1000,
		dmcInfoTrack,
		dmcFollowTrack
	};
	CListView m_list;
	void FillList()
	{
		m_list.Clear();
		app.GetTrackList(&m_list);
	}
	virtual void InitDialog(HWND hDlg)
	{
		SetWindowText(hDlg, L("Tracks"));
		m_list.Create(hDlg, true);
		m_list.AddColumn(L("File name"), 500, 0);
		FillList();
		SetSoftkeybar(hDlg, IDR_TEMPLATE_MENUBAR_MENU);
		CreateMenu();
		SetSubWindowProc(m_list.HWnd());
	}
	void CreateMenu()
	{
		m_MenuBar.SetItemLabelAndCommand(IDC_LEFT, L("Done"), IDOK);
		m_MenuBar.SetItemLabelAndCommand(IDC_RIGHT, L("Menu"), 0);
		CMenu & menu = m_MenuBar.GetMenu();
		menu.CreateItem(L("Unload"), dmcUnload);
		menu.CreateItem(L("Information"), dmcInfoTrack);
		menu.CreateItem(L("Follow"), dmcFollowTrack);
	}
	virtual void WindowPosChanged(HWND hDlg)
	{
		ReinitItemY();
		AddList(m_list.HWnd());
	}
	virtual void Command(HWND hDlg, int iCommand)
	{
		switch (iCommand)
		{
		case IDOK:
			{
				EndDialog(hDlg, 0);
				return;
			}
		case dmcUnload:
			{
				int idx = m_list.GetCurSelParam();
				if (idx >= 0)
				{
					app.CloseTrack(idx);
				}
				m_list.RemoveSelected();
				return;
			}
		case dmcInfoTrack:
			{
				int idx = m_list.GetCurSelParam();
				if (idx >= 0)
				{
					app.InfoTrack(hDlg, idx);
				}
				return;
			}
		case dmcFollowTrack:
			{
				int idx = m_list.GetCurSelParam();
				if (idx >= 0)
				{
					app.FollowTrack(idx);
				}
				return;
			}
		case IDC_PAGEUP:
		case IDC_PAGEDOWN:
			ProcessUpDown(m_list.HWnd(), iCommand);
			return;
		}
	}
};


class CInfoDlg : public CMADialog
{
	CListView m_list;
public:
	std::wstring InfoCaption;
	std::wstring InfoText;
	virtual void InitDialog(HWND hDlg)
	{
		SetWindowText(hDlg, InfoCaption.c_str());
		m_list.Create(hDlg, false);
		m_list.AddColumn(L("Info"), 500, 0);
		m_list.Clear();
		// Split the string in "InfoText" in lines for the list:
		int iIndex = 0;
		wchar_t *cstr = new wchar_t [InfoText.size()+1];
		wcscpy (cstr, InfoText.c_str());
		wchar_t *p = wcstok(cstr, L"\n");
		while (p!=NULL)
		{
			m_list.AddItem(p, iIndex);
			p = wcstok(NULL, L"\n");
			iIndex++;
		}
		delete[] cstr;  
		// Buttons:
		SetSoftkeybar(hDlg, IDR_TEMPLATE_MENUBAR_2);
		m_MenuBar.SetItemLabelAndCommand(IDC_LEFT, L("Ok"), IDOK);
		//m_MenuBar.SetItemLabelAndCommand(IDC_RIGHT, L("Ok"), IDOK);
	}
	virtual void WindowPosChanged(HWND hDlg)
	{
		ReinitItemY();
		AddList(m_list.HWnd());
	}
	virtual void Command(HWND hDlg, int iCommand)
	{
		switch (iCommand)
		{
		case IDOK:
			{
				EndDialog(hDlg, TRUE);
				return;
			}
		}
	}
};


class CSettingsDlg : public CMADialog
{
	enum
	{
		dmcCancel = 1000
	};
	CCombo m_port;
	CCombo m_portSpeed;
	CCombo m_trackstep;
	CEditText  m_proxy;
	CCombo m_coordformat;
	CCombo m_utmZone;
	CCombo m_metrics;
	CEditCombo m_GeoidMode;
	CCombo m_OldTileDays;

	std::map<std::wstring, int> m_comboItems;
	void AddComboItem(int iCombo, int iItem, wchar_t * wcString, int iToSelect)
	{
		int index = SendDlgItemMessage(m_hDialog, iCombo, CB_ADDSTRING, 0, LPARAM(wcString));
		if (iItem == iToSelect)
			SendDlgItemMessage(m_hDialog, iCombo, CB_SETCURSEL, index, 0);
		m_comboItems[wcString] = iItem + 100;
	}
	void AddComboItem(int iCombo, int iItem, int iToSelect)
	{
		wchar_t wstrBuf[100];
		wsprintf(wstrBuf, L"%d", iItem);
		int index = SendDlgItemMessage(m_hDialog, iCombo, CB_ADDSTRING, 0, LPARAM(wstrBuf));
		if (iItem == iToSelect)
			SendDlgItemMessage(m_hDialog, iCombo, CB_SETCURSEL, index, 0);
	}
	virtual void InitDialog(HWND hDlg)
	{
		SetWindowText(hDlg, L("Settings"));
		m_port.Create(hDlg, false);
//#ifdef UNDER_CE
//		wchar_t buff[1000];
//		unsigned long l = 1000;
//		EnumDevices(buff, &l);
//		wchar_t * pos = buff;
//		while (*pos)
//		{
//			m_port.AddItem(pos, app.m_rsPort().c_str());
//			pos += wcslen(pos) + 1;
//		}
//#else
		wchar_t buff[1000];
		for (int i = 0; i <= 20; ++i)
		{
			swprintf(buff, 1000, L"COM%d:", i);
			m_port.AddItem(buff, app.m_rsPort().c_str());			
		}
//#endif
		m_port.SetText(app.m_rsPort().c_str());

		m_portSpeed.Create(hDlg, false);
		m_portSpeed.AddItem(L"Default", app.m_rsPortSpeed().c_str());
		m_portSpeed.AddItem(L"2400", app.m_rsPortSpeed().c_str());
		m_portSpeed.AddItem(L"4800", app.m_rsPortSpeed().c_str());
		m_portSpeed.AddItem(L"9600", app.m_rsPortSpeed().c_str());
		m_portSpeed.AddItem(L"14400", app.m_rsPortSpeed().c_str());
		m_portSpeed.AddItem(L"19200", app.m_rsPortSpeed().c_str());
		m_portSpeed.AddItem(L"38400", app.m_rsPortSpeed().c_str());
		m_portSpeed.AddItem(L"57600", app.m_rsPortSpeed().c_str());
		m_portSpeed.SetText(app.m_rsPortSpeed().c_str());

		m_trackstep.Create(hDlg, false);
		m_trackstep.AddItem(1, app.m_riTrackStep());
		m_trackstep.AddItem(3, app.m_riTrackStep());
		m_trackstep.AddItem(5, app.m_riTrackStep());
		m_trackstep.AddItem(10, app.m_riTrackStep());
		m_trackstep.AddItem(20, app.m_riTrackStep());
		m_trackstep.AddItem(50, app.m_riTrackStep());
		m_trackstep.AddItem(100, app.m_riTrackStep());

		m_proxy.Create(hDlg);
		m_proxy.SetText(app.m_rsProxy().c_str());

		m_coordformat.Create(hDlg, false);
		m_coordformat.AddItem(L"N37°27'35\"");
		m_coordformat.AddItem(L"N37°27.8743'");
		m_coordformat.AddItem(L"N37.278742°");
		m_coordformat.AddItem(L"+37.278742");
		m_coordformat.AddItem(L"N37°27'35.64\"");
		m_coordformat.AddItem(L"UTM (experimental)");
		m_coordformat.Select(app.m_riCoordFormat());

		m_utmZone.Create(hDlg, false);
		m_utmZone.AddItem(L"Automatic");
		for(int i=1;i<=60;i++)
			m_utmZone.AddItem((wchar_t *)UTMZoneToLongText(i).c_str());
		for(int i=-1;i>=-60;i--)
			m_utmZone.AddItem((wchar_t *)UTMZoneToLongText(i).c_str());
		int utmZoneIndex = app.m_riUTMZone();
		if (utmZoneIndex < 0) utmZoneIndex = 60-utmZoneIndex;
		m_utmZone.Select(utmZoneIndex);

		m_metrics.Create(hDlg, false);
		m_metrics.AddItem(L("Metric"));
		m_metrics.AddItem(L("Nautical"));
		m_metrics.AddItem(L("Imperial"));
		m_metrics.Select(app.m_riMetrics());

		m_GeoidMode.Create(hDlg, false);
		m_GeoidMode.AddItem(L"Auto", app.m_rsGeoidMode().c_str());
		m_GeoidMode.AddItem(L"Always", app.m_rsGeoidMode().c_str());
		m_GeoidMode.AddItem(L"Never", app.m_rsGeoidMode().c_str());
		m_GeoidMode.AddItem((wchar_t *)app.m_rsGeoidMode().c_str() , app.m_rsGeoidMode().c_str());
		m_GeoidMode.SetText(app.m_rsGeoidMode().c_str());

		m_OldTileDays.Create(hDlg, false);
		m_OldTileDays.AddItem(1, app.m_riOldTileDays());
		m_OldTileDays.AddItem(3, app.m_riOldTileDays());
		m_OldTileDays.AddItem(7, app.m_riOldTileDays());
		m_OldTileDays.AddItem(30, app.m_riOldTileDays());
		m_OldTileDays.AddItem(90, app.m_riOldTileDays());
		m_OldTileDays.AddItem(365, app.m_riOldTileDays());

		AddItem(hDlg, CText(hDlg, L("Port:")));
		AddItem(hDlg, m_port);
		AddItem(hDlg, CText(hDlg, L("Port speed:")));
		AddItem(hDlg, m_portSpeed);
		AddItem(hDlg, CText(hDlg, L("Track step:")));
		AddItem(hDlg, m_trackstep);
		AddItem(hDlg, CText(hDlg, L("Proxy server (user:pass@host:port):")));
		AddItem(hDlg, m_proxy);
		AddItem(hDlg, CText(hDlg, L("Coordinates format:")));
		AddItem(hDlg, m_coordformat);
		AddItem(hDlg, CText(hDlg, L("UTM Zone:")));
		AddItem(hDlg, m_utmZone);
		AddItem(hDlg, CText(hDlg, L("Metric system:")));
		AddItem(hDlg, m_metrics);
		AddItem(hDlg, CText(hDlg, L("Alt correction (meters):")));
		AddItem(hDlg, m_GeoidMode);
		AddItem(hDlg, CText(hDlg, L("Tile is old after ... days:")));
		AddItem(hDlg, m_OldTileDays);
		
		SetSoftkeybar(hDlg, IDR_TEMPLATE_MENUBAR_2);
		m_MenuBar.SetItemLabelAndCommand(IDC_LEFT, L("Ok"), IDOK);
		m_MenuBar.SetItemLabelAndCommand(IDC_RIGHT, L("Cancel"), dmcCancel);
	}
	void ApplySettings(HWND hDlg)
	{
		const Int cnMaxStr = 100;
		wchar_t buff[cnMaxStr] = {0};
		bool fStartListening = false;

		m_port.GetText(buff, cnMaxStr);
		if (app.m_rsPort() != buff)
		{
			app.m_rsPort = buff;
			fStartListening = true;
		}

		m_portSpeed.GetText(buff, cnMaxStr);
		if (app.m_rsPortSpeed() != buff)
		{
			app.m_rsPortSpeed = buff;
			fStartListening = true;
		}

		if (fStartListening)
			app.StartListening();

		m_trackstep.GetText(buff, cnMaxStr);
		app.m_riTrackStep.Set(wcstol(buff, 0, 10));

		{
		m_OldTileDays.GetText(buff, cnMaxStr);
		long n = wcstol(buff, 0, 10);
		app.m_riOldTileDays.Set(n);
		app.m_pRasterMapPainter->SetOldTileDays(n);
		}

		int index = m_coordformat.GetCurSel();
		if (app.m_riCoordFormat() != index)
		{
			app.m_riCoordFormat.Set(index);
			app.m_monLongitude.SetIdL(L"Longitude", CoordLabelLon());
			app.m_monLatitude.SetIdL(L"Latitude", CoordLabelLat());
		}

		index = m_utmZone.GetCurSel();
		if (index>60) index = 60-index;
		app.m_riUTMZone.Set(index);

		index = m_metrics.GetCurSel();
		app.m_riMetrics.Set(index);

		m_proxy.GetText(buff,cnMaxStr);
		if (app.m_rsProxy() != buff)
		{
			app.m_rsProxy = buff;
		}
		CHttpRequest::SetProxy(app.m_rsProxy());

		m_GeoidMode.GetText(buff, cnMaxStr);
		if (app.m_rsGeoidMode() != buff)
		{
			app.m_rsGeoidMode = buff;
			EGM96init(buff);

		}

		
	}
	virtual void Command(HWND hDlg, int iCommand)
	{
		switch (iCommand)
		{
		case IDOK:
			ApplySettings(hDlg);
		case dmcCancel:
			EndDialog(hDlg, 0);
		}
	}
};

class CWaypointsDlg : public CMADialog
{
	CListView m_list;
	void FillList()
	{
		m_list.Clear();
		app.GetWaypoints().GetList(&m_list, app.m_painter.GetCenterCross(), app.m_riWaypointsRadius());
	}
	virtual void InitDialog(HWND hDlg)
	{
		SetWindowText(hDlg, L("Waypoints"));
		m_list.Create(hDlg, true);
		m_list.AddColumn(L("Name"), 400, 0);
		FillList();
		SetSoftkeybar(hDlg, IDR_TEMPLATE_MENUBAR_MENU);
		CreateMenu();
		SetSubWindowProc(m_list.HWnd());
	}
	enum
	{
		dmcWaypointsProperies = 1000,
		dmcWaypointsToMap, 
		dmcWaypointsDelete, 
		dmcWaypointsNavigate, 
		dmcWaypointsRadius10, 
		dmcWaypointsRadius50, 
		dmcWaypointsRadius100, 
		dmcWaypointsRadius500, 
		dmcWaypointsRadiusInf,
		dmcExportWaypoint
	};
	void CreateMenu()
	{
		m_MenuBar.SetItemLabelAndCommand(IDC_LEFT, L("Done"), IDOK);
		m_MenuBar.SetItemLabelAndCommand(IDC_RIGHT, L("Menu"), 0);
		CMenu & menu = m_MenuBar.GetMenu();
		menu.CreateItem(L("Properties"), dmcWaypointsProperies);
		menu.CreateItem(L("To map"), dmcWaypointsToMap);
		menu.CreateItem(L("Delete"), dmcWaypointsDelete);
		menu.CreateItem(L("Navigate"), dmcWaypointsNavigate);
		CMenu & radius = menu.CreateSubMenu(L("Radius"));
		radius.CreateItem(L("10 km"), dmcWaypointsRadius10);
		radius.CreateItem(L("50 km"), dmcWaypointsRadius50);
		radius.CreateItem(L("100 km"), dmcWaypointsRadius100);
		radius.CreateItem(L("500 km"), dmcWaypointsRadius500);
		radius.CreateItem(L("Infinite"), dmcWaypointsRadiusInf);
		menu.CreateItem(L("Export waypoint"), dmcExportWaypoint);
		CheckMenu();
	}
	void CheckMenu()
	{
		m_MenuBar.GetMenu().CheckMenuItem(dmcWaypointsRadius10, (app.m_riWaypointsRadius() == 10000));
		m_MenuBar.GetMenu().CheckMenuItem(dmcWaypointsRadius50, (app.m_riWaypointsRadius() == 50000));
		m_MenuBar.GetMenu().CheckMenuItem(dmcWaypointsRadius100, (app.m_riWaypointsRadius() == 100000));
		m_MenuBar.GetMenu().CheckMenuItem(dmcWaypointsRadius500, (app.m_riWaypointsRadius() == 500000));
		m_MenuBar.GetMenu().CheckMenuItem(dmcWaypointsRadiusInf, (app.m_riWaypointsRadius() == 40000000));
	}
	virtual void WindowPosChanged(HWND hDlg)
	{
		ReinitItemY();
		AddList(m_list.HWnd());
	}
	virtual int Translate(int iChar)
	{
		switch (iChar)
		{
		case '1':
			return dmcWaypointsProperies;
		case '2':
			return dmcWaypointsToMap;
		case '3':
			return dmcWaypointsDelete;
		case '4':
			return dmcWaypointsNavigate;
		case '5':
			return dmcWaypointsRadiusInf;
		case '6':
			return dmcExportWaypoint;
		}
		return CMADialog::Translate(iChar);
	}
	virtual void Command(HWND hDlg, int iCommand)
	{
		LONG idx = m_list.GetCurSelParam();
		switch (iCommand)
		{
		case IDOK:
			{
				EndDialog(hDlg, 0);
				return;
			}
		case dmcWaypointsProperies:
			{
				if (idx >= 0)
				{
					static CWaypointPropertiesDlg dlg;
					dlg.m_iWaypoint = idx;
					g_pNextDialog = &dlg;
					DialogBox(g_hInst, (LPCTSTR)IDD_TEMPLATE, hDlg, (DLGPROC)MADlgProc);
					m_list.UpdateSelected(app.GetWaypoints().ById(idx).GetLabel().c_str());
				}
				return;
			}
		case dmcWaypointsToMap:
			{
				if (idx >= 0)
				{
					app.m_painter.SetView(app.GetWaypoints().GetPointByIDApprox(idx), true);
					EndDialog(hDlg, 0);
				}
				return;
			}
		case dmcWaypointsDelete:
			{
				if (idx >= 0)
				{
					if (MessageBox(hDlg, L("Are you sure"), L("Delete waypoint"), MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
					{
						app.GetWaypoints().DeleteByID(idx);
						m_list.RemoveSelected();
					}
				}
				return;
			}
		case dmcWaypointsNavigate:
			{
				if (idx >= 0)
				{
					app.Navigate(app.GetWaypoints().GetPointByIDApprox(idx), app.GetWaypoints().GetLabelByID(idx).c_str());
					EndDialog(hDlg, 0);
				}
				return;
			}
		case dmcWaypointsRadius10:
			app.m_riWaypointsRadius.Set(10000);
			FillList();
			CheckMenu();
			return;
		case dmcWaypointsRadius50:
			app.m_riWaypointsRadius.Set(50000);
			FillList();
			CheckMenu();
			return;
		case dmcWaypointsRadius100:
			app.m_riWaypointsRadius.Set(100000);
			FillList();
			CheckMenu();
			return;
		case dmcWaypointsRadius500:
			app.m_riWaypointsRadius.Set(500000);
			FillList();
			CheckMenu();
			return;
		case dmcWaypointsRadiusInf:
			app.m_riWaypointsRadius.Set(40000000);
			FillList();
			CheckMenu();
			return;
		case dmcExportWaypoint:
			if (idx >= 0)
			{
				app.ExportWaypoint(idx, m_hDialog);
			}
			return;
		case IDC_PAGEUP:
		case IDC_PAGEDOWN:
			ProcessUpDown(m_list.HWnd(), iCommand);
			return;
		}
	}
};

class CKeymapDlg : public CMADialog
{
	enum
	{
		dmcOk = 1000,
		dmcSetKey
	};
	bool m_fSetKey;
	CListView m_list;
	void FillList()
	{
		m_list.Clear();
		app.GetKeymap().GetList(&m_list);
		m_fSetKey = false;
	}
	virtual void InitDialog(HWND hDlg)
	{
		SetWindowText(hDlg, L("Key map"));
		m_list.Create(hDlg, false);
		m_list.AddColumn(L("Key"), 60, 0);
		m_list.AddColumn(L("Action"), 300, 1);
		FillList();
		SetSubWindowProc(m_list.HWnd());
		SetSoftkeybar(hDlg, IDR_TEMPLATE_MENUBAR_2);
		m_MenuBar.SetItemLabelAndCommand(IDC_LEFT, L("Done"), dmcOk);
		m_MenuBar.SetItemLabelAndCommand(IDC_RIGHT, L("Set key"), dmcSetKey);
	}
	virtual void WindowPosChanged(HWND hDlg)
	{
		ReinitItemY();
		AddList(m_list.HWnd());
	}
	virtual int Translate(int iChar)
	{
		return 0;
	}
	virtual void Command(HWND hDlg, int iCommand)
	{
		switch (iCommand)
		{
		case dmcSetKey:
			m_fSetKey = true;
			return;
		case IDOK:
			ProcessSubWindowMessage(hDlg, WM_KEYDOWN, VK_RETURN, 0);
			return;
		case dmcOk:
			EndDialog(hDlg, 0);
			return;
		case IDC_PAGEUP:
		case IDC_PAGEDOWN:
			ProcessUpDown(m_list.HWnd(), iCommand);
			return;
		}
	}
	virtual LRESULT ProcessSubWindowMessage(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (message == WM_KEYDOWN)
		{
			if (!m_fSetKey)
				return CMADialog::ProcessSubWindowMessage(hDlg, message, wParam, lParam);
			m_fSetKey = false;

			int nScancode = MakeScancode(wParam, lParam);
			int iSelected = m_list.GetCurSelParam();
			app.GetKeymap().SetActionKey(iSelected, nScancode);

			m_list.Update(&app.GetKeymap());
			// FillList(); // REPLACE
			return TRUE;
		}
		return CMADialog::ProcessSubWindowMessage(hDlg, message, wParam, lParam);
	}
};

LRESULT CALLBACK MADlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (g_Dialogs.find(hDlg) == g_Dialogs.end())
		g_Dialogs[hDlg] = g_pNextDialog;
	return g_Dialogs[hDlg]->Process(hDlg, message, wParam, lParam);
}

LRESULT CALLBACK MASubWindowProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (g_Dialogs.find(hDlg) == g_Dialogs.end())
		return TRUE;
	return g_Dialogs[hDlg]->ProcessSubWindowMessage(hDlg, message, wParam, lParam);
}

DWORD WINAPI ThreadRoutine(LPVOID pArg)
{
	((CMapApp*)pArg)->ThreadRoutine();
	return 0;
}

DWORD WINAPI HttpThreadRoutine(LPVOID pArg)
{
	((CMapApp*)pArg)->HttpThreadRoutine();
	return 0;
}

#define LG(x) \
	if (fWriteLog){wchar_t wcFilename[1000];wsprintf(wcFilename, L"%s\\Connection.log", m_rsTrackFolder().c_str());\
	FILE * log = wfopen(wcFilename, L"at");\
	if (log) {SYSTEMTIME st;GetLocalTime(&st);\
	fprintf(log, "%02d:%02d:%02d.%03d: ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);\
	x;fclose(log);}}

struct TrackPlayer : public IPainter
{
	IGPSClient * m_pClient;
	TrackPlayer(IGPSClient * pClient) : m_pClient(pClient){}
	virtual void StartPolygon(UInt uiType, const wchar_t * wcName)
	{
	}
	virtual void StartPolyline(UInt uiType, const wchar_t * wcName)
	{
	}
	virtual void FinishObject()
	{
		m_pClient->NoFix();
		app.m_TrafficNodes.RefreshTrafficData();
		Sleep(100);
	}
	virtual void AddPoint(const GeoPoint & gp)
	{
		m_pClient->Fix(gp, 0, 1);
		Sleep(50);
		if (app.m_fExiting)
			throw m_pClient;
	}
	virtual bool WillPaint(const GeoRect & rect)
	{
		return true;
	}
	virtual void SetView(const GeoPoint & gp, bool fManual)
	{
	}
	virtual void PaintPoint(UInt uiType, const GeoPoint & gp, const wchar_t * wcName)
	{
	}
	virtual void SetLabelMandatory()
	{
	}
	virtual GeoRect GetRect()
	{
		return GeoRect();
	}
};

void CMapApp::ThreadRoutine()
{
	bool fWriteLog;
	bool fWait = false;
	Byte buff[4096];
	DWORD i;
	m_hPortFile = 0;
	{
		fWriteLog = m_Options[mcoWriteConnectionLog];
		m_NMEAParser.NewStream();
		m_monSleepCounter.Reset(); // NOSYNC
	}
	LG(fprintf(log, "Thread started\n"))
	while (!m_fExiting)
	{
		try
		{
			bool fPresent = false;
			{
				AutoLock l;
				fPresent = m_ReplayTrack.IsPresent();
			}
			if (fPresent)
			{
				TrackPlayer player(this);
				m_ReplayTrack.PaintUnlocked(&player, 0);
				m_ReplayTrack = CTrack();
			}
			FILE * fileReplayNMEA = 0;
			{
				AutoLock l;
				if (!m_wstrReplayNMEA.empty())
				{
					fileReplayNMEA = wfopen(m_wstrReplayNMEA.c_str(), L"rb");
				}
			}
			Byte buffer[10];
			int iRead;
			while (!m_fExiting && fileReplayNMEA && (iRead = fread(buffer, 1, sizeof(buffer), fileReplayNMEA)))
			{
				m_NMEAParser.AddData(buffer, iRead);
				Sleep(10);
			}
		}
		catch (IGPSClient *)
		{
		}
		int iTime;
		if (!m_Options[mcoConnect])
			m_NMEAParser.ConnectionDisabled();
		for (iTime = 5; --iTime && !m_fExiting && !m_Options[mcoConnect];)
			Sleep(1000);
#ifdef SMARTPHONE
		if (m_Options[mcoBluetoothOn] && m_Options[mcoConnect])
		{
			DWORD mode;
			int res = BthGetMode(&mode);
			if (res == ERROR_SUCCESS && mode == BTH_POWER_OFF)
			{
				BthSetMode(BTH_DISCOVERABLE);
				m_fBluetoothWasTurnedOn = true;
			}
		}
#endif
		while (!m_fExiting && m_Options[mcoConnect])
		{
			try
			{
				Sleep(1000);
				{
					LG(fprintf(log, "Port is %S\n", m_rsPort().c_str()))
					if (m_rsPort() != L"")
					{
						m_hPortFile = CreateFile(m_rsPort().c_str(), 
							GENERIC_READ, 0, 0, OPEN_EXISTING,
							FILE_ATTRIBUTE_NORMAL, 0);
					}
					LG(fprintf(log, "m_hPortFile is %p\n", m_hPortFile))
					if (m_hPortFile != INVALID_HANDLE_VALUE)
					{
						long lSpeed = wcstol(m_rsPortSpeed().c_str(), 0, 10);
						if (lSpeed > 0)
						{
							DCB dcb;
							if (GetCommState(m_hPortFile, &dcb))
							{
								dcb.BaudRate = lSpeed;
								SetCommState(m_hPortFile, &dcb);
							}
						}
						break;
						// Note this loop is build so it keeps trying to open the]
						// port. You have to break if opening works.
					}
				}
				for (iTime = 9; --iTime && !m_fExiting;)
					Sleep(1000);
			}
			catch(...)
			{
			}
		}
		LG(fprintf(log, "Configuring port\n"))
		m_dwConnected = GetTickCount();
		fWait = false;
		int iTimeOut = 0;
		COMMTIMEOUTS ct;
		ZeroMemory(&ct, sizeof(ct));
		ct.ReadIntervalTimeout = 50;		// MAXDWORD;
		ct.ReadTotalTimeoutConstant = 1000;
		ct.ReadTotalTimeoutMultiplier = 0;	// MAXDWORD;
		SetCommTimeouts(m_hPortFile, &ct);

		while(m_hPortFile && !m_fExiting)
		{
			LG(fprintf(log, "Reading port\n"));
			i = 0;
			Sleep(500);
			iTimeOut += 500;
			if (!ReadFile(m_hPortFile, buff, sizeof(buff), &i, 0)) {
				break;
			}
			LG(fprintf(log, "Read %d bytes: ", i); for (unsigned long j = 0; j < i; ++j) fprintf(log, "%c", buff[j]); fprintf(log, "\n"))
			if (i < 1) {
				if ((iTimeOut > 9999)) {
					LG(fprintf(log, "Reading underrun\n"));
					break;
				}
				LG(fprintf(log, "Re-reading\n"));
				continue;
			}
			if ((i > sizeof(buff))) {
				LG(fprintf(log, "Reading overflow\n"));
 				break;
			}
			iTimeOut = 0;
			m_NMEAParser.AddData(buff, i);
			if (m_riConnectPeriodMin() > 0 && GetTickCount() > m_dwConnected + 5000)
			{
				fWait = true;
				m_NMEAParser.Pause();
				LG(fprintf(log, "Max GPS connect time\n"));
				break;
			}
			if (!m_Options[mcoConnect]) {
				LG(fprintf(log, "GPS disconnected\n"));
				break;
			}
#ifdef SMARTPHONE
			if ((!m_Options[mcoConnect] || !m_Options[mcoBluetoothOn]) && m_fBluetoothWasTurnedOn)
			{
				BthSetMode(BTH_POWER_OFF);
				m_fBluetoothWasTurnedOn = false;
			}
#endif // SMARTPHONE
		}
		if (m_hPortFile)
		{
			try
			{
				LG(fprintf(log, "Closing port\n"))
				CloseHandle(m_hPortFile);
			}
			catch(...)
			{
			}
			m_hPortFile = 0;
		}
		while (fWait && !m_fExiting && GetTickCount()  < m_dwConnected + m_riConnectPeriodMin() * 60000 && m_Options[mcoConnect])
		{
			LG(fprintf(log, "Sleeping\n"))
			{
				m_monSleepCounter = double(m_dwConnected + m_riConnectPeriodMin() * 60000 - GetTickCount()) / 1000;
			}
			Sleep(1000);
		}
		m_monSleepCounter.Reset();
		if (!fWait || !m_Options[mcoConnect])
		{
			LG(fprintf(log, "Starting new stream\n"));
			m_NMEAParser.NewStream();
		}
#ifdef SMARTPHONE
		if ((!m_Options[mcoConnect] || !m_Options[mcoBluetoothOn]) && m_fBluetoothWasTurnedOn)
		{
			BthSetMode(BTH_POWER_OFF);
			m_fBluetoothWasTurnedOn = false;
		}
#endif // SMARTPHONE
	}
};

void CMapApp::StartListening()
{
	if (m_hPortThread)
	{
		m_fExiting = true;
		WaitForSingleObject(m_hPortThread, INFINITE);
		CloseHandle(m_hPortThread);
		m_hPortThread = 0;
	}
	m_fExiting = false;
	m_hPortThread = CreateThread(0, 10000, &::ThreadRoutine, this, 0, 0);
}

void CMapApp::StartHttpThread()
{
	m_fStopHttpThread = false;
	m_hHttpThread = CreateThread(0, 10000, &::HttpThreadRoutine, this, 0, 0);
}

CMapApp::CMapApp() : m_fMoving(false), m_hWnd(0), m_hPortThread(0), m_hHttpThread(0),
	m_hPortFile(0), m_wstrCmdLine(0), m_hPwrReq(0), m_fMemoryLow(false), m_fMemoryVeryLow(false),
	m_NMEAParser(), m_Options(), m_iMonitorUnder(INVALID_MONITOR_ID)
{
	m_NMEAParser.SetClient(this);
	m_pRasterMapPainter = new CGMPainter();
	InitCoreDll();
}

CMapApp::~CMapApp() 
{
	CHttpRequest::CleanupSocketsIfNecessary();
#ifdef UNDER_CE
	if (m_hCoreDll) {
		m_pfnGetIdleTime = NULL;
		FreeLibrary(m_hCoreDll);
	}
#endif // UNDER_CE
	// Hope, no one uses m_pRasterMapPainter from another thread now...
	delete m_pRasterMapPainter;
	Exit();
}

void CMapApp::OnLButtonDown(ScreenPoint pt)
{
	m_spFrom = pt;
	if (m_Options[mcoMonitorsMode]) {
		m_iMonitorUnder = m_MonitorSet.GetMonitorUnder(pt);
		m_fMoving = true;
	} else {
		m_iPressedButton = m_painter.CheckButton(pt);
		if (m_iPressedButton != -1)
		{
			GetButtons().SelectButton(m_iPressedButton);
			m_painter.BeginPaintLite(VP::DC(m_hWnd));
			GetButtons().Paint(&m_painter);
		} else {
			m_fMoving = true;
			#ifndef UNDER_CE
				// "Preview on click" only on desktop (no use on SP and not good on PPC, because long click = contextmenu)
				m_ClickPointForRoutePreview = m_painter.ScreenToGeo(pt);
				m_painter.Redraw();
			#endif
		}
	}
}

void CMapApp::OnMouseMove(ScreenPoint pt)
{
	if (m_fMoving) {
		if (m_Options[mcoMonitorsMode]) {
			int curMonitorUnder = m_MonitorSet.GetMonitorUnder(pt);
			if (curMonitorUnder != INVALID_MONITOR_ID) {
				m_MonitorSet.SetActiveMonitor(curMonitorUnder);
			}
			m_MonitorSet.SetMovingMonitor(m_iMonitorUnder, pt - m_spFrom);
			m_painter.Redraw();
		} else {
		}
	}
	if (!m_ClickPointForRoutePreview.IsNull())
	{
		m_ClickPointForRoutePreview = GeoPoint();
		m_painter.Redraw();
	}
}

void CMapApp::OnLButtonUp(ScreenPoint pt)
{
	if (m_Options[mcoMonitorsMode]) {
		int newMonitorUnder = m_MonitorSet.GetMonitorUnder(pt);
		if (newMonitorUnder != INVALID_MONITOR_ID) {
			if ((m_iMonitorUnder != INVALID_MONITOR_ID) && (newMonitorUnder != m_iMonitorUnder)) {
				m_MonitorSet.SwapMonitors(m_iMonitorUnder, newMonitorUnder);
				m_MonitorSet.SetActiveMonitor(newMonitorUnder);
				m_painter.Redraw();
			} else if (m_iMonitorUnder == newMonitorUnder) {
				m_MonitorSet.SetActiveMonitor(newMonitorUnder);
				m_painter.Redraw();
			}
		}
		m_iMonitorUnder = INVALID_MONITOR_ID;
		m_MonitorSet.SetMovingMonitor(INVALID_MONITOR_ID, ScreenDiff());
	} else {
		if (m_iPressedButton != -1) {
			if (m_painter.CheckButton(pt) == m_iPressedButton)
				ProcessCommand(GetButtons().GetCommand(m_iPressedButton));
			GetButtons().DeselectButton();
			m_painter.BeginPaintLite(VP::DC(m_hWnd));
			GetButtons().Paint(&m_painter);
		} else if (pt == m_spFrom) {
			LeftClickOrContextMenu(pt, false);
		} else {
			if (!m_painter.WillPaint(pt) && !m_painter.WillPaint(m_spFrom)) {
				// Click inside a monitor bar
				int xDiff = m_spFrom.x - pt.x;
				int minDiff = 16;
				if (xDiff + minDiff < 0) {
					NextMonitorsRow();
				} else if (xDiff - minDiff > 0) {
					PrevMonitorsRow();
				}
			} else {
				// Normal click inside a map area
				if (m_fMoving) {
					// This check is for PPC devices.
					// Long press brings context menu and then short press on non-menu 
					// screen area will NOT generate LButtonDown but WILL generate LButtonUp.
					m_painter.Move(pt - m_spFrom);
				}
			}
		}
	}
	m_fMoving = false;
	if (!m_ClickPointForRoutePreview.IsNull())
	{
		m_ClickPointForRoutePreview = GeoPoint();
		m_painter.Redraw();
	}
}
void CMapApp::ViewZoomIn()
{
	m_painter.ZoomInAtCursor();
}
void CMapApp::ViewZoomOut()
{
	m_painter.ZoomOutAtCursor();
}
void CMapApp::ViewUp()
{
	if (m_Options[mcoMonitorsMode])
	{
		m_MonitorSet.Up();
		m_painter.Redraw();
	}
	else
		m_painter.Up();
}
void CMapApp::ViewDown()
{
	if (m_Options[mcoMonitorsMode])
	{
		m_MonitorSet.Down();
		m_painter.Redraw();
	}
	else
		m_painter.Down();
}
void CMapApp::ViewLeft()
{
	if (m_Options[mcoMonitorsMode])
	{
		m_MonitorSet.Left();
		m_painter.Redraw();
	}
	else
		m_painter.Left();
}
void CMapApp::ViewRight()
{
	if (m_Options[mcoMonitorsMode])
	{
		m_MonitorSet.Right();
		m_painter.Redraw();
	}
	else
		m_painter.Right();
}
void CMapApp::FileExit()
{
	Exit();
	DestroyWindow(m_hWnd);
}

class CIndex
{
	struct File
	{
		std::wstring m_wstrFilename;
		GeoRect m_grArea;
	};
	typedef std::list<File> ListFiles;
	ListFiles m_listFiles;
	std::wstring m_wstrFolder;
	int m_iNextNotification;
	int m_iCount;
public:
	CIndex()
	{
		m_iNextNotification = 10;
		m_iCount = 0;
	}
	void SetFolder(const wchar_t * wstrFolder)
	{
		m_wstrFolder = wstrFolder;
	}
	void AddFile(const wchar_t * wstrFile)
	{
		CIMGFile file;
		file.Parse((m_wstrFolder + wstrFile).c_str());
		m_listFiles.push_back(File());
		m_listFiles.back().m_wstrFilename = m_wstrFolder + wstrFile;
		m_listFiles.back().m_grArea = file.GetRect();
		++ m_iCount;
		if (m_iCount >= m_iNextNotification)
		{
			wchar_t text[100];
			swprintf(text, 100, L("Parsed %d files"), m_iCount);
			MessageBox(0, text, L("Info"), MB_ICONINFORMATION);
			m_iNextNotification += m_iNextNotification / 2; 
		}
	}
};

void CMapApp::FillOpenFileName(OPENFILENAME * of, HWND hwndOwner, wchar_t * wstrFilter, 
							   wchar_t * strFile, bool fDirectory, bool fMustExist, bool fOverwritePrompt)
{
	memset(of, 0, sizeof(*of));
	of->hwndOwner = hwndOwner;
	of->lStructSize = sizeof(*of);
	of->lpstrFilter = wstrFilter;
#ifndef UNDER_CE
	of->lpstrInitialDir = L".";
#endif
	of->lpstrFile = strFile;
	of->nMaxFile = MAX_PATH + 1;
	of->hInstance = g_hInst;
#if defined(UNDER_CE) && !defined(BARECE)
	of->Flags = OFN_EXPLORER | (fMustExist ? OFN_FILEMUSTEXIST : 0) | (fOverwritePrompt ? OFN_OVERWRITEPROMPT : 0) |
		(fDirectory ? OFN_PROJECT : 0);
#else
	of->Flags = OFN_EXPLORER | (fMustExist ? OFN_FILEMUSTEXIST : 0) | (fOverwritePrompt ? OFN_OVERWRITEPROMPT : 0);
#endif
}

void CMapApp::FileIndexDirectory()
{
	wchar_t strFile[MAX_PATH + 1] = {0};
	OPENFILENAME of;
	FillOpenFileName(&of, m_hWnd, L("All files\0*.*\0"), strFile, false, false);
	if (GetOpenFileName(&of))
	{
		std::wstring dir = strFile;
		dir.resize(dir.find_last_of(L"/\\") + 1);

		CIndex idx;

		idx.SetFolder(dir.c_str());

		WIN32_FIND_DATA ffd;
		HANDLE h = FindFirstFile((dir + L"*.img").c_str(), &ffd);
		while (h)
		{
			idx.AddFile(ffd.cFileName);
			if (!FindNextFile(h, &ffd))
				break;
		}
	}
}

void CMapApp::FileOpenMap()
{
	wchar_t strFile[MAX_PATH + 1] = {0};
	OPENFILENAME of;
	FillOpenFileName(&of, m_hWnd, L("Map files\0*.img\0\0"), strFile, false, true); 
	if (GetOpenFileName(&of))
	{
		m_atlas.Add(strFile, &m_painter);
		m_painter.Redraw();
	}
	else
	{
		int err = GetLastError();
	}
}

void CMapApp::ReplayNMEA()
{
	wchar_t strFile[MAX_PATH + 1] = {0};
	OPENFILENAME of;
	FillOpenFileName(&of, m_hWnd, L("All files\0*.*\0\0"), strFile, false, true); 
	if (GetOpenFileName(&of))
	{
		m_wstrReplayNMEA = strFile;
	}
}

void CMapApp::FileCloseAllMaps()
{
	if (MessageBox(m_hWnd, L("Are you sure"), L("Unload all"), MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
	{
		GetAtlas().CloseAll();
		m_painter.Redraw();
	}
}

void CMapApp::FileOpenMapFolder()
{
	wchar_t strFile[MAX_PATH + 1] = {0};
#ifdef UNDER_CE
	OPENFILENAME of;
	FillOpenFileName(&of, m_hWnd, L"\0", strFile, true, true);
	if (GetOpenFileName(&of))
	{
#else
	BROWSEINFO bi;
	memset(&bi, 0, sizeof(bi));
	bi.hwndOwner = m_hWnd;
	bi.lpszTitle = L("Open map folder");
	bi.pszDisplayName = strFile;
	LPITEMIDLIST pl = SHBrowseForFolder(&bi);
	if (pl && SHGetPathFromIDList(pl, strFile))
	{
#endif
		WIN32_FIND_DATA ffd;
		HANDLE h = FindFirstFile((std::wstring(strFile) + L"\\*.img").c_str(), &ffd);
		while (h)
		{
			m_atlas.Add((std::wstring(strFile) + L"\\" + ffd.cFileName).c_str(), &m_painter);
			if (!FindNextFile(h, &ffd))
				break;
		}
		m_painter.Redraw();
	}
	else
	{
		int err = GetLastError();
	}
}

void CMapApp::ExportWaypoint(int id, HWND hWnd)
{
	wchar_t strFile[MAX_PATH + 1] = {0};
	OPENFILENAME of;
	FillOpenFileName(&of, hWnd, L("Waypoint files\0*.wpt\0"), strFile, false, false);
	if (GetOpenFileName(&of))
	{
		int iLen = wcslen(strFile);
		if (iLen >= 4 && !!_wcsicmp(strFile + iLen - 4, L".wpt"))
			wcscpy(strFile + iLen, L".wpt");
		CWaypoints points;
		points.Read(strFile);
		points.AddPoint(m_Waypoints.ById(id));
	}
}

void CMapApp::FileOpenWaypoints()
{
	wchar_t strFile[MAX_PATH + 1] = {0};
	wsprintf(strFile, L"%s", m_Waypoints.GetFilename().c_str());
	OPENFILENAME of;
	FillOpenFileName(&of, m_hWnd, L("Waypoint files\0*.wpt;*.gpx\0"), strFile, false, true);
	if (GetOpenFileName(&of))
	{
		m_Waypoints.Read(strFile);
		if (!m_Waypoints.CanWrite())
		{
			MessageBox(m_hWnd, L("This isn't a gpsvp file. Please use the import function."), L("Not a gpsVP file"), MB_OK | MB_ICONEXCLAMATION);
			// Back to the previous file
			m_Waypoints.Read(m_rsWaypointsFile().c_str());
		}
		else
		{
			m_rsWaypointsFile = strFile;
			m_painter.Redraw();
		}
	}
}

void CMapApp::FileNewWaypointsWPT()
{
	wchar_t strFile[MAX_PATH + 1] = {0};
	SYSTEMTIME st;
	GetLocalTime(&st);
	wsprintf(strFile, L"Wpt-%04d-%02d-%02d-%02d-%02d.wpt", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
	OPENFILENAME of;
	FillOpenFileName(&of, m_hWnd, L("Waypoint files\0*.wpt\0"), strFile, false, false, true);
	if (GetSaveFileName(&of))
	{
		int iLen = wcslen(strFile);
		if (iLen >= 4 && !!_wcsicmp(strFile + iLen - 4, L".wpt"))
			wcscpy(strFile + iLen, L".wpt");
		CWaypoints newPoints;
		newPoints.WriteWPT(strFile);
		m_Waypoints.Read(strFile);
		m_rsWaypointsFile = strFile;
		m_painter.Redraw();
	}
}
void CMapApp::FileNewWaypointsGPX()
{
	wchar_t strFile[MAX_PATH + 1] = {0};
	SYSTEMTIME st;
	GetLocalTime(&st);
	wsprintf(strFile, L"Wpt-%04d-%02d-%02d-%02d-%02d.gpx", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
	OPENFILENAME of;
	FillOpenFileName(&of, m_hWnd, L("Waypoint files\0*.gpx\0"), strFile, false, false, true);
	if (GetSaveFileName(&of))
	{
		int iLen = wcslen(strFile);
		if (iLen >= 4 && !!_wcsicmp(strFile + iLen - 4, L".gpx"))
			wcscpy(strFile + iLen, L".gpx");
		CWaypoints newPoints;
#ifndef UNDER_WINE
		newPoints.WriteGPX(strFile);
#endif // UNDER_WINE
		m_Waypoints.Read(strFile);
		m_rsWaypointsFile = strFile;
		m_painter.Redraw();
	}
}

void CMapApp::FileImportWaypoints()
{
	wchar_t strFile[MAX_PATH + 1] = {0};
	OPENFILENAME of;
	FillOpenFileName(&of, m_hWnd, L("Waypoint files\0*.wpt;*.gpx\0"), strFile, false, true);
	if (GetOpenFileName(&of))
	{
		CWaypoints t;
		t.Read(strFile);
		m_Waypoints.Import(t);
		m_painter.Redraw();
	}
}

void CMapApp::FileExportWaypointsWPT()
{
	wchar_t strFile[MAX_PATH + 1] = {0};
	SYSTEMTIME st;
	GetLocalTime(&st);
	wsprintf(strFile, L"Exported_wpt-%04d-%02d-%02d-%02d-%02d.wpt", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
	OPENFILENAME of;
	FillOpenFileName(&of, m_hWnd, L("Waypoint files\0*.wpt\0"), strFile, false, false, true);
	if (GetSaveFileName(&of))
	{
		int iLen = wcslen(strFile);
		if (iLen >= 4 && !!_wcsicmp(strFile + iLen - 4, L".wpt"))
			wcscpy(strFile + iLen, L".wpt");
		CWaypoints newPoints;
		newPoints.SetNameWPT(strFile);
		newPoints.Import(m_Waypoints); // Import also write the file
	}
}
void CMapApp::FileExportWaypointsGPX()
{
	wchar_t strFile[MAX_PATH + 1] = {0};
	SYSTEMTIME st;
	GetLocalTime(&st);
	wsprintf(strFile, L"Exported_wpt-%04d-%02d-%02d-%02d-%02d.gpx", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
	OPENFILENAME of;
	FillOpenFileName(&of, m_hWnd, L("Waypoint files\0*.gpx\0"), strFile, false, false, true);
	if (GetSaveFileName(&of))
	{
		int iLen = wcslen(strFile);
		if (iLen >= 4 && !!_wcsicmp(strFile + iLen - 4, L".gpx"))
			wcscpy(strFile + iLen, L".gpx");
		CWaypoints newPoints;
		newPoints.SetNameGPX(strFile);
		newPoints.Import(m_Waypoints); // Import also write the file
	}
}
void CMapApp::FileExportWaypointsOSM()
{
	wchar_t strFile[MAX_PATH + 1] = {0};
	SYSTEMTIME st;
	GetLocalTime(&st);
	wsprintf(strFile, L"gpsvp-wpt-%04d-%02d-%02d-%02d-%02d.osm", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
	OPENFILENAME of;
	FillOpenFileName(&of, m_hWnd, L("OpenStreetMap files\0*.osm\0"), strFile, false, false, true);
	if (GetSaveFileName(&of))
	{
		int iLen = wcslen(strFile);
		if (iLen >= 4 && !!_wcsicmp(strFile + iLen - 4, L".osm"))
			wcscpy(strFile + iLen, L".osm");
#ifndef UNDER_WINE
		m_Waypoints.WriteOSM(strFile);
#endif // UNDER_WINE
	}
}

void CMapApp::FileSaveWaypoints()
{
	m_Waypoints.Write();
}

void CMapApp::FileNextColors()
{
	if (!m_rsToolsFile().empty())
		m_rsToolsFile = L"";
	else
		m_riScheme.Set((m_riScheme() + 1) % 2);
	m_painter.InitTools(m_riScheme());
	m_painter.Redraw();
}

void CMapApp::FileOpenColors()
{
	wchar_t strFile[MAX_PATH + 1] = {0};
	OPENFILENAME of;
	FillOpenFileName(&of, m_hWnd, L("Color scheme files\0*.vpc\0"), strFile, false, true);
	if (GetOpenFileName(&of))
	{
		m_painter.InitTools(m_riScheme());
		m_painter.InitTools(strFile);
		m_rsToolsFile = strFile;
		m_painter.Redraw();
	}
	CheckMenu();
}
void CMapApp::FileCloseColors()
{
	m_painter.InitTools(m_riScheme());
	m_rsToolsFile = L"";
	m_painter.Redraw();
	CheckMenu();
}
void CMapApp::FileOpenTranslation()
{
	wchar_t strFile[MAX_PATH + 1] = {0};
	OPENFILENAME of;
	FillOpenFileName(&of, m_hWnd, L("Translation files\0*.vpl\0"), strFile, false, true);
	if (GetOpenFileName(&of))
	{
		m_rsTranslationFile = strFile;
		MessageBox(0, L("Language settings will be applied after restart"), L("Language settings"), MB_ICONINFORMATION);
		CheckMenu();
	}
}
void CMapApp::FileCloseTranslation()
{
	m_rsTranslationFile = L"";
	MessageBox(0, L("Language settings will be applied after restart"), L("Language settings"), MB_ICONINFORMATION);
	CheckMenu();
}
void CMapApp::FileOpenTrack()
{
	wchar_t strFile[MAX_PATH + 1] = {0};
	OPENFILENAME of;
	FillOpenFileName(&of, m_hWnd, L("Track files\0*.plt;*.gpx\0"), strFile, false, true);
	if (GetOpenFileName(&of))
	{
		if (-1 != m_Tracks.OpenTracks(strFile))
		{
			m_painter.SetView(m_Tracks.Last().GetLastPoint(), true);
			m_painter.Redraw();
		}
	}
}

void CMapApp::CloseTrack(Int iIndex)
{
	m_Tracks.CloseTrack(iIndex);
	m_painter.Redraw();
}

void CMapApp::FollowTrack(Int iIndex)
{
	if (m_Tracks.NewRouteFromTrackIndex(iIndex))
	{
		SaveCurrentRoute(); // Save it as route.plt
		ToolsNavigateRoute();
	}
}

void CMapApp::InfoTrack(HWND hDlgParent, const CTrack& track)
{
	std::wstring strInfo = L"";
	for(int i=0; i<track.GetInfoCount(); i++)
	{
		strInfo += track.GetInfo(i) + L" \n"; // Whitespace important for the list in CInfoDlg
	}

	// MessageBox is good on SP, OK on desktop, bad on PPC.
	//	MessageBox(hDlgParent, strInfo.c_str(), L("Track info"), MB_ICONINFORMATION);

	// Own dialog to replace MessageBox:
	static CInfoDlg dlg;
	dlg.InfoCaption = L("Track info");		
	dlg.InfoText = strInfo;		
	g_pNextDialog = &dlg;
	DialogBox(g_hInst, (LPCTSTR)IDD_TEMPLATE, hDlgParent, (DLGPROC)MADlgProc);
}

void CMapApp::OptionsSetTrackFolder()
{
	wchar_t strFile[MAX_PATH + 1] = {0};
#ifdef UNDER_CE
	OPENFILENAME of;
	FillOpenFileName(&of, m_hWnd, L"\0", strFile, true, true);
	if (GetOpenFileName(&of))
	{
#else
	BROWSEINFO bi;
	memset(&bi, 0, sizeof(bi));
	bi.hwndOwner = m_hWnd;
	bi.lpszTitle = L("Set track folder");
	bi.pszDisplayName = strFile;
	LPITEMIDLIST pl = SHBrowseForFolder(&bi);
	if (pl && SHGetPathFromIDList(pl, strFile))
	{
#endif
		m_rsTrackFolder = strFile;
	}
}

void CMapApp::SetRasterMapFolder()
{
	wchar_t strFile[MAX_PATH + 1] = {0};
#ifdef UNDER_CE
	OPENFILENAME of;
	FillOpenFileName(&of, m_hWnd, L"\0", strFile, true, true);
	if (GetOpenFileName(&of))
	{
#else
	BROWSEINFO bi;
	memset(&bi, 0, sizeof(bi));
	bi.hwndOwner = m_hWnd;
	bi.lpszTitle = L("Open map folder");
	bi.pszDisplayName = strFile;
	LPITEMIDLIST pl = SHBrowseForFolder(&bi);
	if (pl && SHGetPathFromIDList(pl, strFile))
	{
#endif
		m_rsRasterMapFolder = strFile;
		m_pRasterMapPainter->SetMapFolder(m_rsRasterMapFolder().c_str(), g_gpsVPVersion);
	}
}

void CMapApp::ToolsTracks()
{
	static CTracksDlg dlg;
	g_pNextDialog = &dlg;
	DialogBox(g_hInst, (LPCTSTR)IDD_TEMPLATE, m_hWnd, (DLGPROC)MADlgProc);
}

void CMapApp::SearchOSM()
{
	static CSearchOSMDlg dlg;
	g_pNextDialog = &dlg;
	DialogBox(g_hInst, (LPCTSTR)IDD_TEMPLATE, m_hWnd, (DLGPROC)MADlgProc);
	m_painter.Redraw();
}

void CMapApp::SearchResults()
{
	static CSearchResultsDlg dlg;
	g_pNextDialog = &dlg;
	DialogBox(g_hInst, (LPCTSTR)IDD_TEMPLATE, m_hWnd, (DLGPROC)MADlgProc);
	m_painter.Redraw();
}

void CMapApp::Create(HWND hWnd, wchar_t * wcHome)
{
	RECT rc;
	::GetWindowRect(hWnd, &rc);
	bool hiRes = rc.bottom > 400 || rc.left > 400;
#ifdef SMARTPHONE
	m_fBluetoothWasTurnedOn = false;
#endif // SMARTPHONE
	m_TypeInfo.Parse(g_hInst);
	HKEY hRegKey = 0;
#ifdef RegCreateKey
	RegCreateKey(HKEY_CURRENT_USER, L"Software\\Vsevolod Shorin\\VSMapViewer", &hRegKey);
#else
	RegCreateKeyEx(HKEY_CURRENT_USER, L"Software\\Vsevolod Shorin\\VSMapViewer", 0, L"", 0, 0, 0, &hRegKey, 0);
#endif
	m_rsTranslationFile.Init(hRegKey, L"TranslationFile");
	if (m_rsTranslationFile() != L"")
		m_dict.Read(m_rsTranslationFile().c_str());

	m_MenuBar.SetItemLabelAndCommand(IDC_LEFT, L("Context menu"), mcContextMenu);
	m_MenuBar.SetItemLabelAndCommand(IDC_RIGHT, L("Menu"), 0);

	m_MonitorSet.Init(hRegKey);

	m_atlas.Init(hRegKey);
	m_rsWaypointsFile.Init(hRegKey, L"WaypointsFile");
	m_rsToolsFile.Init(hRegKey, L"ToolsFile");
	m_riScheme.Init(hRegKey, L"ToolsResource", 0);
	m_rsTrackFolder.Init(hRegKey, L"TrackFolder");

	m_Options.Init(hRegKey, &m_MonitorSet);
	m_Options.AddOption(L("Follow cursor"), L"FollowCursor", true, mcoFollowCursor);
	m_Options.AddOption(L("Show monitor bar"), L"ShowMonitors", true, mcoShowMonitorBar);
	m_Options.AddOption(L("Show POIs"), L"ShowPOIs", true, mcoShowPOIs);
	m_Options.AddOption(L("Show waypoints"), L"ShowWaypoints", true, mcoShowWaypoints);
	m_Options.AddOption(L("Show area labels"), L"ShowPolygonLabels", true, mcoShowPolygonLabels);
	m_Options.AddOption(L("Show outline only"), L"ShowAreaAsOutline", false, mcoShowAreaAsOutline);
	m_Options.AddOption(L("Show unknown types"), L"ShowUnknownTypes", true, mcoShowUnknownTypes);
	m_Options.AddOption(L("Rotate map by course"), L"RotateMap", false, mcoRotateMap);
	m_Options.AddOption(L("Show road name"), L"ShowRoadName", true, mcoShowRoadName);
	m_Options.AddOption(L("Show area name"), L"ShowAreaName", true, mcoShowAreaName);
	m_Options.AddOption(L("Monitors mode"), L"MonitorsMode", false, mcoMonitorsMode);
	m_Options.AddOption(L("Buffer output"), L"Buffer", true, mcoBuffered);
	m_Options.AddOption(L("Sound"), L"Sound", true, mcoSound);
	m_Options.AddOption(L("Warn on GPS loss"), L"WarnNoGPS", false, mcoWarnNoGPS);
	m_Options.AddOption(L("Keep backlight"), L"KeepBacklight", false, mcoKeepBacklight);
	m_Options.AddOption(L("Keep device on"), L"KeepDeviceOn", true, mcoKeepDeviceOn);
//	m_Options.AddOption(L("Allow internet connection always"), L"AllowInternetAlways", true, mcoAllowInternetAlways);
//	m_Options.AddOption(L("Allow internet connection home only"), L"AllowInternetHomeOnly", true, mcoAllowInternetHomeOnly);
	m_Options.AddOption(L("Use proxy server"), L"EnableProxy", false, mcoUseProxy);
	m_Options.AddOption(L("Connect"), L"Connect", true, mcoConnect);
	m_Options.AddOption(L("Show center"), L"ShowCenter", true, mcoShowCenter);
	m_Options.AddOption(L("Refresh traffic on startup"), L"TrafficOnStartup", false, mcoRefreshTrafficOnStartup);
	// m_Options.AddOption(L("Show traffic nodes"), L"ShowTrafficNodes", false, mcoShowTrafficNodes);
	m_Options.AddOption(L("Download Google maps"), L"DownloadGoogleMaps", false, mcoDownloadGoogleMaps);
	m_Options.AddOption(L("Download all lower levels"), L"DownloadLowerLevels", false, mcoDownloadLowerLevels);
	m_Options.AddOption(L("Cache auto delete"), L"CacheAutoDelete", false, mcoRasterCacheAutoDelete);
	m_Options.AddOption(L("Show Garmin maps"), L"ShowGarminMaps", true, mcoShowGarminMaps);
	m_Options.AddOption(L("Prefer Google zoom levels"), L"GoogleZoomLevels", false, mcoGoogleZoomLevels);
	m_Options.AddOption(L("Invert satellite images"), L"InvertSatelliteImages", false, mcoInvertSatelliteImages);
	m_Options.AddOption(L("Show fastest way"), L"ShowFastestWay", true, mcoShowFastestWay);
	m_Options.AddOption(L("Show traffic information"), L"ShowTrafficInformation", true, mcoShowTrafficInformation);
	m_Options.AddOption(L("Use test server"), L"TestServer", false, mcoTestServer);
	m_Options.AddOption(L("Show traffic flags"), L"TrafficFlags", false, mcoTrafficFlags);
	m_Options.AddOption(L("Large monitors"), L"LargeMonitors", false, mcoLargeMonitors);
	m_Options.AddOption(L("Automatic light"), L"AutomaticLight", false, mcoAutoLight);
	m_Options.AddOption(L("Large fonts"), L"LargeFonts", hiRes, mcoLargeFonts);
	m_Options.AddOption(L("Show Sun azimuth"), L"ShowSunAz", false, mcoShowSunAz);
	m_Options.AddOption(L("Hide cache tiles"), L"Hide cache tiles", false, mcoHideCacheTiles);
	
	m_fTrafficAgreement.Init(hRegKey, L"TrafficAgreement", false);
	
	if (m_Options[mcoRefreshTrafficOnStartup])
		m_TrafficNodes.RefreshTrafficData();
#ifndef SMARTPHONE
	m_Options.AddOption(L("Show screen buttons"), L"ShowScreenButtons", true, mcoScreenButtons);
#endif // !SMARTPHONE
#ifdef AC_SRC_OVER
	m_Options.AddOption(L("Low light"), L"LowLight", false, mcoLowLight);
#endif // AC_SRC_OVER
#ifdef SMARTPHONE
	m_Options.AddOption(L("Turn bluetooth on"), L"BluetoothOn", false, mcoBluetoothOn);
#endif // SMARTPHONE
	m_Options.AddOption(L("Full screen"), L"FullScreen", false, mcoFullScreen);
	m_Options.AddOption(L("Debug mode"), L"DebugMode", false, mcoDebugMode);
	m_Options.AddOption(L("Write connection log"), L"WriteConnectionLog", false, mcoWriteConnectionLog);
	m_Options.AddOption(L("Keep memory low"), L"LowMemory", true, mcoLowMemory);
	m_Options.AddOption(L("Write track"), L"WriteTrack", true, mcoWriteTrack);
	m_Options.AddOption(L("Show current track"), L"ShowCurrentTrack", true, mcoShowCurrentTrack);
	m_Options.AddOption(L("Quick read gpx (no time, no elevation)"), L"QuickReadGPX", false, mcoQuickReadGPXTrack);	
	m_Options.AddOption(L("Read multitrack file as single track"), L"MultitrackAsSingle", true, mcoMultitrackAsSingleTrack);
	m_Options.AddOption(L("Highlight maps"), L"ShowDetailMaps", true, mcoShowDetailMaps);
	m_Options.AddOption(L("Direct paint"), L"DirectPaint", false, mcoDirectPaint);

	if (!m_Options[mcoDebugMode])
	{
		
	}

	m_riTrackStep.Init(hRegKey, L"TrackStep", 1);
	m_riCoordFormat.Init(hRegKey, L"CoordinateFormat", 0);
	m_riUTMZone.Init(hRegKey, L"UTMZone", 0);
	m_riMetrics.Init(hRegKey, L"MetricSystem", 0);
	m_riOldTileDays.Init(hRegKey, L"OldTileDays", 90);
	m_riWaypointsRadius.Init(hRegKey, L"WaypointsRadius", 40000000);
	m_riDetail.Init(hRegKey, L"Detail", 0);
	m_riConnectPeriodMin.Init(hRegKey, L"ConnectPeriod", 0);
	m_riTrackFormat.Init(hRegKey, L"TrackFormat", 0);
	m_gpNavigate.Init(hRegKey, L"NavigatePoint", GeoPoint(0,0));
	m_riNavigate.Init(hRegKey, L"Navigate", nmOff);
	m_MRUPoints.Init(hRegKey);
	m_rsPort.Init(hRegKey, L"Port");
	m_rsPortSpeed.Init(hRegKey, L"PortSpeed");
	if (m_rsPortSpeed().empty())
		m_rsPortSpeed = L"Default";
	m_rsCurrentFolder.Init(hRegKey, L"");
	m_rsProxy.Init(hRegKey,L"Proxy");
	m_rsGeoidMode.Init(hRegKey, L"GeoidMode");
	if (m_rsGeoidMode().empty())
		m_rsGeoidMode = L"Auto";
	EGM96init((wchar_t *)m_rsGeoidMode().c_str());
	GetKeymap().Init(hRegKey);
	GetButtons().Init(hRegKey);


	m_painter.Init(hWnd, hRegKey);

	m_monDistance.SetIdL(L"Distance");
	m_MonitorSet.AddMonitor(&m_monDistance);

	m_monAzimuth.SetIdL(L"Azimuth");
	m_MonitorSet.AddMonitor(&m_monAzimuth);
	
	m_monCourse.SetIdL(L"Course");
	m_MonitorSet.AddMonitor(&m_monCourse);
	
	m_monCourseRel.SetIdL(L"Relative course");
	m_MonitorSet.AddMonitor(&m_monCourseRel);
	

	//m_monTrackDistance.SetIdL(L"Track distance"));
	//m_MonitorSet.AddMonitor(&m_monTrackDistance);

	m_monForwardRouteDistance.SetIdL(L"Route distance");
	m_MonitorSet.AddMonitor(&m_monForwardRouteDistance);

	m_NMEAParser.InitMonitors(m_MonitorSet, hRegKey, m_Options[mcoDebugMode]);

	m_monSatellites.SetIdL(L"Satellites");
	m_monSatellites.SetParser(&m_NMEAParser);
	m_MonitorSet.AddMonitor(&m_monSatellites);

	m_monInternet.SetIdL(L"Internet");
	m_monInternet.SetTextCopySrc(CTextMonitor::TEXTCOPY_SRCURL);
	m_MonitorSet.AddMonitor(&m_monInternet);

	m_monHDOP.SetIdL(L"HDOP");
	m_MonitorSet.AddMonitor(&m_monHDOP);

	m_monAltitude.SetIdL(L"Altitude");
	m_MonitorSet.AddMonitor(&m_monAltitude);
	m_monSeparation.SetIdL(L"Alt. correction");
	//m_monSeparation.SetRegistry(hRegKey, L"GeoidSeparation");
	m_MonitorSet.AddMonitor(&m_monSeparation);
	
	m_monLongitude.SetIdL(L"Longitude", CoordLabelLon());
	m_monLongitude.SetLinkedLatitude(&m_monLatitude);
	m_MonitorSet.AddMonitor(&m_monLongitude);
	m_monLatitude.SetIdL(L"Latitude", CoordLabelLat());
	m_monLatitude.SetLinkedLongitude(&m_monLongitude);
	m_MonitorSet.AddMonitor(&m_monLatitude);

	m_monMemory.SetIdL(L"Memory");
	m_MonitorSet.AddMonitor(&m_monMemory);

	m_monDLRemaining.SetIdL(L"Download queue");
	m_MonitorSet.AddMonitor(&m_monDLRemaining);

	m_monDataIn.SetIdL(L"Data in");
	m_monDataIn = 0;
	m_MonitorSet.AddMonitor(&m_monDataIn);

	m_monDataOut.SetIdL(L"Data out");
	m_monDataOut = 0;
	m_MonitorSet.AddMonitor(&m_monDataOut);

	m_monDataTotal.SetIdL(L"Data total");
	m_monDataTotal = 0;
	m_MonitorSet.AddMonitor(&m_monDataTotal);

	m_monBattery.SetIdL(L"Battery");
	m_MonitorSet.AddMonitor(&m_monBattery);

	if (m_Options[mcoDebugMode]) {
		m_monCPU.SetIdL(L"CPU");
		m_MonitorSet.AddMonitor(&m_monCPU);
	}

	m_monOdometerTotal.SetIdL(L"Odo total");
	m_monOdometerTotal.SetRegistry(hRegKey, L"OdometerTotal");
	m_MonitorSet.AddMonitor(&m_monOdometerTotal);

	m_monOdometer1.SetIdL(L"Odo 1");
	m_monOdometer1.SetResetable();
	m_monOdometer1.SetRegistry(hRegKey, L"Odometer1");
	m_MonitorSet.AddMonitor(&m_monOdometer1);

	m_monOdometer2.SetIdL(L"Odo 2");
	m_monOdometer2.SetResetable();
	m_monOdometer2.SetRegistry(hRegKey, L"Odometer2");
	m_MonitorSet.AddMonitor(&m_monOdometer2);

	if (m_Options[mcoDebugMode])
	{
		for (int i = 0; i < sizeof(m_monProfile) / sizeof(m_monProfile[0]); ++i)
		{
			wchar_t buff[100];
			wsprintf(buff, L"Profile %d", i);
			m_monProfile[i].SetIdL(buff);
			m_MonitorSet.AddMonitor(&m_monProfile[i]);
		}
	}

	m_monSleepCounter.SetIdL(L"Pause timer");
	m_MonitorSet.AddMonitor(&m_monSleepCounter);

	m_Sun.m_monSunrise.SetIdL(L"Sunrise");
	m_MonitorSet.AddMonitor(&m_Sun.m_monSunrise);

	m_Sun.m_monDaytime.SetIdL(L"Daytime");
	m_MonitorSet.AddMonitor(&m_Sun.m_monDaytime);

	m_Sun.m_monSunset.SetIdL(L"Sunset");
	m_MonitorSet.AddMonitor(&m_Sun.m_monSunset);

	if (m_rsWaypointsFile().empty())
		m_rsWaypointsFile = L"\\My Documents\\Waypoints.wpt";
	m_Waypoints.Read(m_rsWaypointsFile().c_str());
	m_painter.InitTools(m_riScheme());
	if (!m_rsToolsFile().empty())
		m_painter.InitTools(m_rsToolsFile().c_str());
	
	m_iConnectionStatus = csNotConnected;
	m_wstrHome = wcHome;
	m_hWnd = hWnd;
			
	m_fCursorVisible = false;
	m_fFix = false;
	m_fCoursePointPresent = false;

	m_fActive = true;
	m_fNeedPaintOnActivate = false;

	if (m_wstrCmdLine && m_wstrCmdLine[0])
		ProcessCmdLine(m_wstrCmdLine);

	m_fCheckLatestVersion = false;

	m_rsRasterMapFolder.Init(hRegKey, L"RasterMapFolder");
	if (!m_rsRasterMapFolder().empty())
	{
		m_pRasterMapPainter->SetMapFolder(m_rsRasterMapFolder().c_str(), g_gpsVPVersion);
		LoadCurrentRoute();
	}
	// Only after loading the list of WMS-Maps
	m_riGMapType.Init(hRegKey, L"GMapType", gtMap);
	if (m_riGMapType() < 0 || m_riGMapType() >= m_pRasterMapPainter->GetGMapCount())
		m_riGMapType.Set(0);

	m_pRasterMapPainter->SetOldTileDays(m_riOldTileDays());

	m_riAllowInternet.Init(hRegKey, L"AllowInternet", 0);
#ifdef UNDER_CE
	OnPhoneRoaming(PHONE_ROAM_NOTIFY_DISABLED);
#endif

	CheckOptions();

	m_Tracks.GetCurTrack().Init();

	StartListening();

	InitMenu();

	if (m_pRasterMapPainter->NeedRelocateFiles())
	{
		if (IDYES == MessageBox(m_hWnd, L("Google maps cache structure has changed. Do you want to move old files to new structure? It may take some time."), L("Google maps files"), MB_ICONWARNING | MB_YESNO))
		{
			m_pRasterMapPainter->RelocateFiles();
		}
	}

	StartHttpThread();
}
void CMapApp::OnTimer()
{
	if (!m_wstrVersionMessage.empty())
	{
		std::wstring wstrVersionMessage = m_wstrVersionMessage;
		m_wstrVersionMessage = L"";
		MessageBox(m_hWnd, wstrVersionMessage.c_str(), L"gpsVP.com", MB_ICONINFORMATION);
	}
	m_painter.OnTimer();
#ifdef UNDER_CE
	if (m_Options[mcoKeepDeviceOn])
		SystemIdleTimerReset();
#endif // UNDER_CE
	if (m_fActive)
	{
		if (m_Options[mcoMonitorsMode])
		{
			m_painter.Redraw();
		}
		else
			m_painter.RedrawMonitors();
#ifdef UNDER_CE
		if (m_Options[mcoKeepBacklight])
		{
#ifdef SMARTPHONE
			SHIdleTimerReset();
#endif
		}
#endif
	}
}

bool operator == (RECT & a, RECT b)
{
	return a.bottom == b.bottom && a.left == b.left && a.right == b.right && a.top == b.top;
}

void CMapApp::CheckOptions()
{
	m_painter.SetShowMonitors(m_Options[mcoShowMonitorBar] && !m_Options[mcoMonitorsMode]);
	bool fFullScreen = (m_Options[mcoFullScreen] && m_fActive);
	if (fFullScreen != m_painter.IsFullScreen())
		m_painter.SetFullScreen(fFullScreen);
	if (m_Tracks.GetCurTrack().IsWriting() != m_Options[mcoWriteTrack])
		m_Tracks.GetCurTrack().SetWriting(m_Options[mcoWriteTrack]);
	if (!m_Options[mcoFollowCursor])
		m_painter.ResetManualMode();

#ifdef UNDER_CE
	bool fKeep = m_Options[mcoKeepBacklight] && m_fActive;
	if (fKeep != (m_hPwrReq != 0))
	{
		if (m_hPwrReq)
		{
			ReleasePowerRequirement(m_hPwrReq);
			m_hPwrReq = 0;
		}
		else
		{
			m_hPwrReq = SetPowerRequirement(TEXT("BKL1:"), D0, POWER_NAME, NULL, 0);	
		}
	}
#endif
	m_painter.GetFontCache().SetLargeFonts(m_Options[mcoLargeFonts]);
}

#ifdef UNDER_CE
#	if UNDER_CE >= 0x0500

void CMapApp::RegisterRoamNotify()
{
	// Create a notification to detect whether phone is in roaming network
	NOTIFICATIONCONDITION nc;
	ZeroMemory(&nc, sizeof(NOTIFICATIONCONDITION));
	nc.ctComparisonType = REG_CT_ANYCHANGE;
	nc.dwMask = SN_PHONEROAMING_BITMASK;
	nc.TargetValue.dw = SN_PHONEROAMING_BITMASK;
	HRESULT hRes = RegistryNotifyWindow(SN_PHONEROAMING_ROOT, SN_PHONEROAMING_PATH, SN_PHONEROAMING_VALUE, m_hWnd, UM_REGNOTIFY, 0, &nc, &GetRegNotify());
	if (SUCCEEDED(hRes)) {
		OnPhoneRoaming(PHONE_ROAM_NOTIFY_ENABLED);
	}
	OnRoamNotify(); // Here it gets roaming status using regular registry read
}

void CMapApp::OnRoamNotify()
{
	if (IsPhoneRoaming()) {
		OnPhoneRoaming(PHONE_ROAM_ROAMING_NETWORK);
	} else {
		OnPhoneRoaming(PHONE_ROAM_HOME_NETWORK);
	}
}
#	endif

bool CMapApp::IsPhoneRoaming()
{
#	if UNDER_CE >= 0x0500
	HKEY hRegKey = 0;
	DWORD nRes;
#ifdef RegOpenKey
	nRes = RegOpenKey(SN_PHONEROAMING_ROOT, SN_PHONEROAMING_PATH, &hRegKey);
#else
	nRes = RegOpenKeyEx(SN_PHONEROAMING_ROOT, SN_PHONEROAMING_PATH, 0, KEY_READ, &hRegKey);
#endif
	if (ERROR_SUCCESS != nRes)
		return false;
	DWORD dwData, dwType;
	DWORD dwLen = sizeof(dwData);
	nRes = RegQueryValueEx(hRegKey, SN_PHONEROAMING_VALUE, NULL, &dwType, (LPBYTE) &dwData, &dwLen);
	if (ERROR_SUCCESS != nRes)
		return false;
	bool bIsRoaming = (dwData & SN_PHONEROAMING_BITMASK) != 0;
	RegCloseKey(hRegKey);
	return bIsRoaming;
#	else //UNDER_CE >= 0x0500
	return false;
#	endif
}

void CMapApp::OnPhoneRoaming(DWORD roaming)
{
	switch (roaming) {
		case PHONE_ROAM_NOTIFY_ENABLED:
			m_bPhoneRoamingNotifyEnabled = true;
			m_bIsPhoneRoaming = false;
			break;
		case PHONE_ROAM_NOTIFY_DISABLED:
			m_bPhoneRoamingNotifyEnabled = false;
			m_bIsPhoneRoaming = false;
			break;
		case PHONE_ROAM_HOME_NETWORK:
			m_bIsPhoneRoaming = false;
			break;
		case PHONE_ROAM_ROAMING_NETWORK:
			m_bIsPhoneRoaming = true;
			break;
	}
}
#endif //UNDER_CE

bool CMapApp::IsInternetAllowed()
{
	if (m_riAllowInternet() == 0)
		return false;

	if (m_riAllowInternet() & ALLOW_INTERNET_ALWAYS)
		return true;

#ifdef UNDER_CE
	if (m_riAllowInternet() & ALLOW_INTERNET_HOME_ONLY) {
		bool bIsRoam = false;
		if (m_bPhoneRoamingNotifyEnabled) {
			bIsRoam = m_bIsPhoneRoaming;
		} else {
			bIsRoam = IsPhoneRoaming();
		}
		if (bIsRoam) {
			AutoLock l;
			m_wstrHttpStatus = L("Roaming");
		}
		return !bIsRoam;
	}
#endif // UNDER_CE
	return false;
}

class CStatusPainter : public IStatusPainter
{
	int m_iProgress;
public:
	CStatusPainter(HWND hwnd, CFontCache & fc) : m_dc(hwnd), m_fc(fc), m_hwnd(hwnd), m_iProgress(0) {}
	virtual void PaintText(const wchar_t * wcText)
	{
		PaintText(wcText, false);
	}
	void PaintText(const wchar_t * wcText, bool fSecond)
	{
		RECT rect;
		GetWindowRect(m_hwnd, &rect);
		HFONT f = m_fc.GetFont((rect.right - rect.left) / 6, true, 0); 
		m_dc.SelectObject(f);
		SIZE s;
		m_dc.getTextExtentPoint(wcText, &s);
		m_dc.ExtTextOut((rect.right - rect.left) / 2 - s.cx / 2, (rect.bottom - rect.top) / 2 - (fSecond ? 0 : s.cy), 0, 0, wcText, 0);
	}
	virtual void SetProgressItems(int iLevel, int iCount) 
	{
		for (std::map<int, int>::iterator it = m_Progress.begin(); it != m_Progress.end(); ++it)
		{
			if (it->first > iLevel)
				it->second = 0;
		}
		m_Progress[iLevel] = 0;
		m_Levels[iLevel] = iCount;
		PaintProgress();
	}
	virtual void SetProgress(int iLevel, int iProgress)
	{
		for (std::map<int, int>::iterator it = m_Progress.begin(); it != m_Progress.end(); ++it)
		{
			if (it->first > iLevel)
				it->second = 0;
		}
		m_Progress[iLevel] = iProgress;
		PaintProgress();
	}
	virtual void Advance(int iLevel)
	{
		for (std::map<int, int>::iterator it = m_Progress.begin(); it != m_Progress.end(); ++it)
		{
			if (it->first > iLevel)
				it->second = 0;
		}
		++m_Progress[iLevel];
		PaintProgress();
	}
	void PaintProgress()
	{
		double progress = 0;
		double scale = 1.0;
		for (std::map<int, int>::iterator it = m_Levels.begin(); it != m_Levels.end(); ++it)
		{
			scale /= it->second;
			progress += scale * m_Progress[it->first];
		}
		int iProgress = int(100 * progress);
		if (iProgress != m_iProgress)
		{
			m_iProgress = iProgress;
			PaintText((IntToText(m_iProgress) + L"%").c_str(), true);
		}
	}
private:
	VP::DC m_dc;
	CFontCache & m_fc;
	HWND m_hwnd;
	std::map<int, int> m_Levels;
	std::map<int, int> m_Progress;
};

void CMapApp::AdjustZoom()
{
	if (m_Options[mcoGoogleZoomLevels]) // && (m_painter.GetScale256() > 3*SCALEFACTOR))
	{
		m_painter.PrepareScales();
		double xscale = m_painter.GetXScale();
		double new_scale = m_pRasterMapPainter->GetPreferredScale(xscale);
		if (abs(xscale - new_scale) / xscale > 0.004)
			m_painter.SetXScale(new_scale);
	}
}

void CMapApp::Paint()
{
	if (!m_fActive)
	{
		m_fNeedPaintOnActivate = true;
		return;
	}
	try
	{
		AdjustZoom();
		bool fMonitorsMode;
		bool fBuffered;
		bool fLowLight;
		bool fShowRoadName;
		bool fShowAreaName;
		bool fLowMemory;
		bool fShowDetailMaps;
		bool fShowPOIs;
		bool fShowWaypoints;
		bool fDirectPaint;
		bool fRotateMap;
		bool fShowUnknownTypes;
		bool fShowPolygonLabels;
		bool fShowAreaAsOutline;
		bool fShowCurrentTrack;
		bool fShowGarminMaps;
		bool fLargeMonitors;
		bool fLargeFonts;
		std::string request;
		{
			AutoLock l;
			fShowGarminMaps = m_Options[mcoShowGarminMaps];
			fMonitorsMode = m_Options[mcoMonitorsMode];
			GetKeymap().SetMonitorsMode(fMonitorsMode);
			fBuffered = m_Options[mcoBuffered];
			fLowLight = m_Options[mcoLowLight];
			fShowRoadName = fShowGarminMaps && m_Options[mcoShowRoadName];
			fShowAreaName = fShowGarminMaps && m_Options[mcoShowAreaName];
			fLowMemory = m_Options[mcoLowMemory];
			fShowDetailMaps = m_Options[mcoShowDetailMaps];
			fShowPOIs = m_Options[mcoShowPOIs];
			fShowWaypoints = m_Options[mcoShowWaypoints];
			fShowUnknownTypes = m_Options[mcoShowUnknownTypes];
			fDirectPaint = m_Options[mcoDirectPaint];
			fRotateMap = m_Options[mcoRotateMap] && m_riGMapType() == gtNone;
			fShowPolygonLabels = m_Options[mcoShowPolygonLabels];
			fShowAreaAsOutline = m_Options[mcoShowAreaAsOutline];
			fShowCurrentTrack = m_Options[mcoShowCurrentTrack];
			fLargeMonitors = m_Options[mcoLargeMonitors];
			fLargeFonts = m_Options[mcoLargeFonts];
			request = m_request;
		}
		PAINTSTRUCT ps;
		VP::DC hdc;
		VP::DC hdcScreen(m_hWnd, &ps);
		if (fMonitorsMode)
		{
			ScreenRect srWindow;
			GetClientRect(m_hWnd, &srWindow);
			if (fBuffered)
				hdc = m_screenBuffer.GetContext(hdcScreen, srWindow.right - srWindow.left, srWindow.bottom - srWindow.top);
			else
				hdc = hdcScreen;

			m_painter.BeginPaint(m_hWnd, hdc, ps.rcPaint, 0, false);
			UpdateMonitors();
			m_MonitorSet.PaintMonitors(&m_painter, srWindow, true, m_painter.IsVertical(), fLargeMonitors);
			
			if (fBuffered)
			{
				hdcScreen.BitBlt(srWindow.left, srWindow.top,
					srWindow.right - srWindow.left,
					srWindow.bottom - srWindow.top,
					hdc, 0, 0, SRCCOPY);
			}
		}
		else
		{
			bool fOnlyMonitors = !((ps.rcPaint.top < m_painter.GetMonitorsBar().top)
				|| (ps.rcPaint.left < m_painter.GetMonitorsBar().left));

			ScreenRect srWindow;
			GetClientRect(m_hWnd, &srWindow);

			if (fBuffered)
				hdc = m_screenBuffer.GetContext(hdcScreen, srWindow.right - srWindow.left, srWindow.bottom - srWindow.top);
			else
				hdc = hdcScreen;
		
			DWORD dwTimer = GetTickCount();
			DWORD dwTmp;
			GeoPoint gpCursor;
			bool fCursorVisible;
			{
				AutoLock l;
				gpCursor = m_gpCursor;
				fCursorVisible = m_fCursorVisible;
				int degree = 0;
				bool fLowCenter = false;
				if (fRotateMap && m_monCourse.IsSet() && fCursorVisible)
				{
					degree = int(m_monCourse.Get());
					if (!m_painter.ManualMode())
						fLowCenter = true;
				}
				m_painter.BeginPaint(m_hWnd, hdc, m_painter.GetScreenRect(), degree, fLowCenter);
				m_painter.SetShowUnknownTypes(fShowUnknownTypes);
				m_painter.SetShowPolygonLabels(fShowPolygonLabels);
				m_painter.SetShowAreaAsOutline(fShowAreaAsOutline);
			}

			if (!fOnlyMonitors)
			{
				if (!m_fMemoryLow)
				{
					UInt uiScale = PrepareScale(m_painter.GetScale256());
					if (fShowGarminMaps)
					{
						CStatusPainter statuspainter(m_hWnd, m_painter.GetFontCache());
						m_atlas.BeginPaint(uiScale, &m_painter, &statuspainter);
						if (fShowDetailMaps)
							m_atlas.PaintMapPlaceholders(&m_painter);
					}

					m_pRasterMapPainter->SetKeepMemoryLow(fLowMemory);
					m_pRasterMapPainter->Paint(hdc.Get(), m_painter.GetScreenRect(), m_painter.GetCenter(), m_painter.GetXScale(), enumGMapType(m_riGMapType()), fLargeFonts);

					if (fShowGarminMaps)
					{
						dwTmp = GetTickCount(); m_monProfile[0] = dwTmp - dwTimer; dwTimer = dwTmp;
						m_atlas.Paint(maskPolygons, fDirectPaint);
						dwTmp = GetTickCount(); m_monProfile[1] = dwTmp - dwTimer; dwTimer = dwTmp;
						m_atlas.Paint(maskPolylines, fDirectPaint);
						dwTmp = GetTickCount(); m_monProfile[2] = dwTmp - dwTimer; dwTimer = dwTmp;
					}
				}
				else
				{
					m_painter.PaintLowMemory(L("Memory low!"), m_fMemoryVeryLow ? L("Track disabled") : L("Map disabled."));
				}
				{
					AutoLock l;
					if (!m_ClickPointForRoutePreview.IsNull())
						m_Tracks.GetCurRoute().SetPreviewInsertionPoint(m_ClickPointForRoutePreview);
					else
						m_Tracks.GetCurRoute().SetPreviewInsertionPoint(m_painter.GetCenterCross());
					if (m_TrackCompetition.IsDetected()) 
						m_Tracks.PaintOldTracksWithCompetition(&m_painter, m_TrackCompetition.GetPoint(), m_TrackCompetition.GetTime());
					else
						m_Tracks.PaintOldTracks(&m_painter);
				}
				dwTmp = GetTickCount(); m_monProfile[3] = dwTmp - dwTimer; dwTimer = dwTmp;
				m_TrackCompetition.Paint(&m_painter);
				if (fShowWaypoints)
					m_Waypoints.Paint(&m_painter, (m_fFix ? &gpCursor : 0));
				m_TrafficNodes.Paint(&m_painter, m_Options[mcoTrafficFlags]);
				dwTmp = GetTickCount(); m_monProfile[4] = dwTmp - dwTimer; dwTimer = dwTmp;
				if (!m_fMemoryLow && fShowPOIs && fShowGarminMaps)
				{
					m_atlas.Paint(maskPoints, fDirectPaint);
					dwTmp = GetTickCount(); m_monProfile[5] = dwTmp - dwTimer; dwTimer = dwTmp;
				}
				if (fShowCurrentTrack)
				{
					AutoLock l;
					if (m_TrackCompetition.IsDetected()) 
					{
						m_Tracks.GetCurTrack().SetCompetition(m_TrackCompetition.GetPoint(), m_TrackCompetition.GetTime());
					}
					else
						m_Tracks.GetCurTrack().SetCompetition(GeoPoint(), 0);
					m_Tracks.GetCurTrack().PaintUnlocked(&m_painter, CTrack::typeCurrentTrack);
				}
				dwTmp = GetTickCount(); m_monProfile[6] = dwTmp - dwTimer; dwTimer = dwTmp;
				m_painter.PaintScale();
				if (!app.m_searchurl.empty())
					m_painter.PaintStatusLine(L("Searching ..."));
				if (!m_fMemoryLow)
				{
					if (fShowRoadName && m_fFix)
					{
						ObjectInfo info = FindNearestPolyline(gpCursor, 100);
						if (info.fPresent)
							m_painter.PaintStatusLine(info.GetDescription().c_str());
					}
					if (fShowRoadName && m_fFix)
					{
						ObjectInfo info = FindPolygon(gpCursor);
						if (info.fPresent)
							m_painter.PaintStatusLine(info.GetDescription().c_str());
					}
				}
				if (m_Options[mcoDebugMode])
				{
					wchar_t buffer[1000];
					swprintf(buffer, 1000, L"%S", request.c_str());
					m_painter.PaintStatusLine(buffer);
				}
				PaintCursor(gpCursor, fCursorVisible);
				m_painter.PaintStatusIcon(m_iConnectionStatus);
				m_painter.PaintCompass();
			}
			UpdateMonitors();
			m_MonitorSet.PaintMonitors(&m_painter, m_painter.GetMonitorsBar(), false, m_painter.IsVertical(), fLargeMonitors);
#ifndef SMARTPHONE
			m_painter.ClearButtons();
			if (m_Options[mcoScreenButtons])
			{
				GetButtons().Paint(&m_painter);
			}
#endif

			dwTmp = GetTickCount(); m_monProfile[7] = dwTmp - dwTimer; dwTimer = dwTmp;
			if (fBuffered)
			{
#if defined(AC_SRC_OVER)
				if (fLowLight)
				{
					GetClientRect(m_hWnd, &srWindow);
					VP::DC hdcPreScreen = m_screenBuffer.GetContext(hdcScreen, 
						ps.rcPaint.right - ps.rcPaint.left, 
						ps.rcPaint.bottom - ps.rcPaint.top);

					hdcPreScreen.BitBlt(ps.rcPaint.left, ps.rcPaint.top,
						ps.rcPaint.right - ps.rcPaint.left,
						ps.rcPaint.bottom - ps.rcPaint.top,
						hdc, ps.rcPaint.left, ps.rcPaint.top, BLACKNESS);
					hdcPreScreen.BitBlend(ps.rcPaint.left, ps.rcPaint.top,
						ps.rcPaint.right - ps.rcPaint.left,
						ps.rcPaint.bottom - ps.rcPaint.top,
						hdc, ps.rcPaint.left, ps.rcPaint.top,
						128);
					hdcScreen.BitBlt(ps.rcPaint.left, ps.rcPaint.top,
						ps.rcPaint.right - ps.rcPaint.left,
						ps.rcPaint.bottom - ps.rcPaint.top,
						hdcPreScreen, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
				}
				else
#endif // AC_SRC_OVER
				{
					hdcScreen.BitBlt(ps.rcPaint.left, ps.rcPaint.top,
						ps.rcPaint.right - ps.rcPaint.left,
						ps.rcPaint.bottom - ps.rcPaint.top,
						hdc, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
				}
			}
			if (fLowMemory && fShowGarminMaps)
				m_atlas.Trim(m_painter.GetRect());
			m_painter.EndPaint();
		}
	}
	catch(...)
	{
		// MessageBox(0, L("Error"), L("Error"), 0);
	}
}

void CMapApp::NoFix()
{
	if (m_fCursorVisible)
		m_painter.Redraw();
	m_Tracks.GetCurTrack().Break();
	m_fCursorVisible = false;
	m_monHDOP.Reset();
	m_monLatitude.Reset();
	m_monLongitude.Reset();
	m_fFix = false;
}

void CMapApp::NoVFix()
{
	m_monAltitude.Reset();
	m_Tracks.GetCurTrack().ResetAltitude();
}

void CMapApp::Fix(GeoPoint gp, double dTimeUTC, double dHDOP)
{
	if (m_fFix)
		AddOdometer(DoubleDistance(gp, m_gpCursor));
	else
		m_fFix = true;
	// Add point to track
	if (m_fMemoryVeryLow)
		m_Tracks.GetCurTrack().Break();
	else
		m_Tracks.GetCurTrack().AddPoint(gp, dTimeUTC, dHDOP); // TODO: GPS time
	// Move view to point if flag present
	if (m_Options[mcoFollowCursor])
		m_painter.SetView(gp, false); // SYNC
	// Show cursor @ the point
	if (!m_fCursorVisible || gp != m_gpCursor)
	{
		m_gpCursor = gp;
		m_fCursorVisible = true;
		m_painter.Redraw();
	}
	m_monHDOP = dHDOP;
	(CDoubleMonitor&)m_monLatitude = Degree(gp.lat);
	(CDoubleMonitor&)m_monLongitude = Degree(gp.lon);
	if (m_fCoursePointPresent)
	{
		if (IntDistance(m_gpCoursePoint, gp) > 50)
		{
			m_monCourse.Set(IntAzimuth(m_gpCoursePoint, gp));
			m_gpCoursePoint = gp;
		}
	}
	else
	{
		m_fCoursePointPresent = true;
		m_gpCoursePoint = gp;
	}
#ifdef UNDER_CE
	// We check the option first not to check for proximity in vain
	if (m_Options[mcoSound] && m_Waypoints.CheckProximity(gp))
		PlaySound(L"ProximitySound", g_hInst, SND_RESOURCE | SND_ASYNC);
#endif // UNDER_CE
	m_TrafficNodes.Fix(gp, GetTickCount());
	m_TrackCompetition.Fix(gp, GetTickCount());
	m_Sun.Fix(m_NMEAParser.GetTimeMonitor(), gp);
	m_team.Fix(gp, GetTickCount());
}

void CMapApp::VFix(double dAltitude, double dSeparation)
{
	m_Tracks.GetCurTrack().SetAltitude(dAltitude);
	(CDoubleMonitor &)m_monAltitude = dAltitude;
	(CDoubleMonitor &)m_monSeparation = dSeparation;
}	

void CMapApp::UpdateMonitors()
{
	AutoLock l;
	{
		if ((nmOff != m_riNavigate()) && m_fCursorVisible)
		{
			(CDoubleMonitor &)m_monDistance = DoubleDistance(m_gpNavigate(), m_gpCursor);
			m_monAzimuth.Set(IntAzimuth(m_gpCursor, m_gpNavigate()));
			if (m_monCourse.IsSet() && m_fCursorVisible) {
				int course = int(m_monCourse.Get());
				int azimuth = int(m_monAzimuth.Get());
				int diff = (course - azimuth + 720) % 360;
				if (diff > 180)
					diff -= 360;
				m_monCourseRel.Set(diff);
			}
			/*
			double dTrackDistance = GetDistanceByTrack(m_gpNavigate(), gpCursor);
			if (dTrackDistance < 40000000)
				(CDoubleMonitor&)m_monTrackDistance = dTrackDistance;
			else
				m_monTrackDistance.Reset();
			*/
			CRoute& route = m_Tracks.GetCurRoute();
			if ((nmRoute == m_riNavigate()) && (!route.IsEmpty()))
			{
				double dForward;
				GeoPoint gpNextOnTrack;
				route.UpdatePosition(m_gpCursor, dForward, gpNextOnTrack);
				(CDoubleMonitor &)m_monForwardRouteDistance = dForward;
				m_gpNavigate.Set(gpNextOnTrack);
			}
			else
			{
				route.UpdateAsNotFollowed();
				m_monForwardRouteDistance.Reset();
			}
		}
		else
		{
			m_monDistance.Reset();
			m_monAzimuth.Reset();
			// m_monTrackDistance.Reset();
			if (nmRoute == m_riNavigate())
				m_Tracks.GetCurRoute().UpdateNoPosition();
			else
				m_Tracks.GetCurRoute().UpdateAsNotFollowed();
			m_monForwardRouteDistance.Reset();
		}
	}
	{
		MEMORYSTATUS ms;
		ms.dwLength = sizeof(ms);
		GlobalMemoryStatus(&ms);
		m_monMemory = ms.dwAvailPhys;
#ifdef SMARTPHONE
		const int cnMinMemory = 1024 * 1024;
		const int cnCriticalMemory = 512 * 1024;
#else
		const int cnMinMemory = 1024 * 1024;
		const int cnCriticalMemory = 512 * 1024 ;
#endif
		m_fMemoryLow = (ms.dwAvailPhys < cnMinMemory);
		m_fMemoryVeryLow = (ms.dwAvailPhys < cnCriticalMemory);
	}
	{
#if !defined(BARECE) && !defined(UNDER_WINE)
#if defined(UNDER_CE)
		SYSTEM_POWER_STATUS_EX status;
		GetSystemPowerStatusEx(&status, FALSE);
#else
		SYSTEM_POWER_STATUS status;
		GetSystemPowerStatus(&status);
#endif
		if (status.BatteryFlag == BATTERY_FLAG_NO_BATTERY)
			m_monBattery.Reset();
		else
			m_monBattery = status.BatteryLifePercent;
#endif
	}
	{
#ifdef UNDER_CE
		if (m_pfnGetIdleTime) {
			DWORD dwTicksElapsed = m_dwLastTickTimer;
			m_dwLastTickTimer = GetTickCount();
			dwTicksElapsed = m_dwLastTickTimer - dwTicksElapsed;
			if (dwTicksElapsed > MIN_CPUMON_STEP_MS) {
				DWORD dwTicksIdled = m_dwLastIdleTick;
				m_dwLastIdleTick = (*m_pfnGetIdleTime)();
				dwTicksIdled = m_dwLastIdleTick - dwTicksIdled;

				// TODO: use only integer maths
				float f = (float) (dwTicksElapsed - dwTicksIdled);
				f *= 100;
				f /= dwTicksElapsed;
				m_monCPU = (byte) (f+.5);
			}
#else
		if (m_nProcessorUsage > 0) {
			DWORD nTicksElapsed = GetTickCount() - m_dwLastTickTimer;
			if (nTicksElapsed > MIN_CPUMON_STEP_MS) {
				m_dwLastTickTimer = GetTickCount();
				FILETIME tmCreatTime, tmExitTime, tmKernel, tmUser;
				if (GetProcessTimes(GetCurrentProcess(), &tmCreatTime, &tmExitTime, &tmKernel, &tmUser)) {
					__int64 *ptmUser = (__int64 *) &tmUser;
					__int64 *ptmKernel = (__int64 *) &tmKernel;
					__int64 nUsage = (*ptmKernel + *ptmUser) - m_nProcessorUsage;
					m_nProcessorUsage = *ptmKernel + *ptmUser;
					m_monCPU = (byte) (((double) nUsage / 100) / nTicksElapsed + 0.5);
				}
			} else {
				// m_monCPU is showing actual info
			}
#endif
		} else {
			m_monCPU.Reset();
		}
	}

	m_monInternet = m_wstrHttpStatus.c_str();
	m_monDLRemaining = IntToText(m_pRasterMapPainter->GetDownloadQueueSize()).c_str();
}

int CMapApp::AddPointScreen(ScreenPoint pt, wchar_t * wcName)
{
	return m_Waypoints.AddPoint(m_painter.ScreenToGeo(pt), -777, wcName, 0);
}

void CMapApp::Navigate(ScreenPoint pt, const wchar_t * wcName)
{
	Navigate(m_painter.ScreenToGeo(pt), wcName);
}

int CMapApp::AddPointCenter(wchar_t * wcName)
{
	return m_Waypoints.AddPoint(m_painter.GetCenterCross(), (m_monAltitude.IsSet() ? int(m_monAltitude.Get()) : -777), wcName, 0);
}

void CMapApp::SetMenu(HWND hWndMenu)
{
	m_MenuBar.SetMenuBar(hWndMenu);
}

void CMapApp::SetMenu(HMENU hMenu)
{
	m_MenuBar.SetMenu(hMenu);
}

void CMapApp::InitMenu()
{
	CMenu & mMenu = m_MenuBar.GetMenu();
	{
		CMenu & mmMaps = mMenu.CreateSubMenu(L("Maps"));
		{
			CMenu & mmGarminMaps = mmMaps.CreateSubMenu(L("Garmin maps"));
			{
				CMenu & mmDetail = mmGarminMaps.CreateSubMenu(L("Detail"));
				mmDetail.CreateItem(L("Decrease detail"), mcDecreaseDetail);
				mmDetail.CreateItem(L("Increase detail"), mcIncreaseDetail);
				mmDetail.CreateBreak();
				mmDetail.CreateItem(L("Set lowest detail"), mcSetDetail1);
				mmDetail.CreateItem(L("Set low detail"), mcSetDetail2);
				mmDetail.CreateItem(L("Set normal detail"), mcSetDetail3);
				mmDetail.CreateItem(L("Set high detail"), mcSetDetail4);
				mmDetail.CreateItem(L("Set highest detail"), mcSetDetail5);
			}
			mmGarminMaps.CreateItem(L("Map list"), mcMapList);
			mmGarminMaps.CreateItem(L("Open map"), mcOpenMap);
			mmGarminMaps.CreateItem(L("Open map folder"), mcOpenMapFolder);
			mmGarminMaps.CreateItem(L("Unload all maps"), mcCloseAllMaps);
			mmGarminMaps.CreateBreak();
			mmGarminMaps.CreateItem(L("Show Garmin maps"), mcoShowGarminMaps);
			mmGarminMaps.CreateItem(L("Show POIs"), mcoShowPOIs);
			mmGarminMaps.CreateItem(L("Highlight detailed maps"), mcoShowDetailMaps);
			mmGarminMaps.CreateItem(L("Show unknown types"), mcoShowUnknownTypes);
			mmGarminMaps.CreateItem(L("Show area labels"), mcoShowPolygonLabels);
			mmGarminMaps.CreateItem(L("Show road name"), mcoShowRoadName);
			mmGarminMaps.CreateItem(L("Show area name"), mcoShowAreaName);
			mmGarminMaps.CreateItem(L("Show only outline"), mcoShowAreaAsOutline);
		}
		{
			CMenu & mmGoogleMaps = mmMaps.CreateSubMenu(L("Raster maps"));
			{
				CMenu & mmMapType = mmGoogleMaps.CreateSubMenu(L("Raster map type"));
				mmMapType.CreateItem(L("Previous map type"), mcPrevGMapType);
				mmMapType.CreateItem(L("Next map type"), mcNextGMapType);
				mmMapType.CreateBreak();
				mmMapType.CreateItem(L("None"), mcGMapType + gtNone);
				mmMapType.CreateItem(L("OpenStreetMap.org map"), mcGMapType + gtOsm);
				mmMapType.CreateItem(L("Google.com map"), mcGMapType + gtMap);
				mmMapType.CreateItem(L("Google.com satellite"), mcGMapType + gtSatellite);
				// mmMapType.CreateItem(L("Google.com hybrid"), mcGMapType + gtHybrid);
				mmMapType.CreateItem(L("Google.com topo"), mcGMapType + gtTopo);
				// mmMapType.EnableMenuItem(mcGMapType + gtHybrid, false);
				mmMapType.CreateItem(L("Live.com map"), mcGMapType + gtMSMap);
				mmMapType.CreateItem(L("Live.com satellite"), mcGMapType + gtMSSat);
				mmMapType.CreateItem(L("Live.com hybrid"), mcGMapType + gtMSHyb);
				mmMapType.CreateItem(L("n.maps.yandex.ru"), mcGMapType + gtNYandex);
				InitMenuAllWMSMaps(mmMapType);
			}
			{
				CMenu & mmDownloadMaps = mmGoogleMaps.CreateSubMenu(L("Download maps"));
				mmDownloadMaps.CreateItem(L("Add current view"), mcDownlRasterAddCurrentView);
				mmDownloadMaps.CreateItem(L("Start with current zoom"), mcDownlRasterStartWithCurZoom);
				mmDownloadMaps.CreateItem(L("By track"), mcDownlRasterByTrack);
				mmDownloadMaps.EnableMenuItem(mcDownlRasterByTrack, false);
				mmDownloadMaps.CreateBreak();
				mmDownloadMaps.CreateItem(L("Download all lower levels"), mcoDownloadLowerLevels);
				// mmDownloadMaps.EnableMenuItem(mcoDownloadLowerLevels, false);
			}
			{
				CMenu & mmMaintMaps = mmGoogleMaps.CreateSubMenu(L("Cache maintenance"));
				mmMaintMaps.CreateItem(L("Refresh all"), mcRastMapsRefreshAll);
				mmMaintMaps.CreateItem(L("Refresh inside region"), mcRastMapsRefreshInsideRegion);
				mmMaintMaps.CreateItem(L("Delete from cache"), mcRastMapsDeleteFromCache);
				mmMaintMaps.EnableMenuItem(mcRastMapsDeleteFromCache, false);
				mmMaintMaps.CreateBreak();
				mmMaintMaps.CreateItem(L("Cache auto delete"), mcoRasterCacheAutoDelete);
				mmMaintMaps.EnableMenuItem(mcoRasterCacheAutoDelete, false);
				mmMaintMaps.CreateItem(L("Set raster cache folder"), mcSetGoogleMapsFolder);
				mmMaintMaps.CreateItem(L("Hide cache tiles"), mcoHideCacheTiles);
			}
			mmGoogleMaps.CreateItem(L("menu placeholder"), mcoPlaceholder1);
			mmGoogleMaps.EnableMenuItem(mcoPlaceholder1, false);
			mmGoogleMaps.CreateBreak();
			mmGoogleMaps.CreateItem(L("Prefer Google zoom levels"), mcoGoogleZoomLevels);
			mmGoogleMaps.CreateItem(L("Enable raster downloading"), mcoDownloadGoogleMaps);
			mmGoogleMaps.CreateItem(L("Invert satellite images"), mcoInvertSatelliteImages);		
			mmGoogleMaps.CreateItem(L("Go to demo point"), mcGoToDemoPoint);
		}
		{
			CMenu & mmSearch = mmMaps.CreateSubMenu(L("Search"));
			mmSearch.CreateItem(L("Search OpenStreetMap.org"), mcSearchOSM);
			mmSearch.CreateItem(L("Search results"), mcSearchResults);
		}
		mmMaps.CreateItem(L("About maps"), mcAboutMaps);
	}
	{
		CMenu & mmTracks = mMenu.CreateSubMenu(L("Tracks"));
		mmTracks.CreateItem(L("Open track"), mcOpenTrack);
		mmTracks.CreateItem(L("Track list"), mcTrackList);
		mmTracks.CreateItem(L("Start new track"), mcNewTrack);
		mmTracks.CreateItem(L("Set track folder"), mcSetTrackFolder);
		mmTracks.CreateItem(L("Info current track"), mcInfoCurTrack);
		mmTracks.CreateBreak();
		mmTracks.CreateItem(L("Write track"), mcoWriteTrack);
		mmTracks.CreateItem(L("Show current track"), mcoShowCurrentTrack);
		{
			CMenu & mmTrackFormat = mmTracks.CreateSubMenu(L("Track format"));
			mmTrackFormat.CreateItem(L("plt"), mcTrackFormatPLT);
			mmTrackFormat.CreateItem(L("gpx"), mcTrackFormatGPX);
		}
		{
			CMenu & mmTrackOptions = mmTracks.CreateSubMenu(L("Track options"));
			mmTrackOptions.CreateItem(L("Quick read gpx (no time, no elevation)"), mcoQuickReadGPXTrack);
			mmTrackOptions.CreateItem(L("Read multitrack file as single track"), mcoMultitrackAsSingleTrack);
		}
	}
	{
		CMenu & mmWaypoints = mMenu.CreateSubMenu(L("Waypoints"));
		{
			CMenu & mmSelectWaypointsFile = mmWaypoints.CreateSubMenu(L("Select waypoints file"));
			mmSelectWaypointsFile.CreateItem(L("Existing file"), mcOpenWaypoints);
			mmSelectWaypointsFile.CreateItem(L("New WPT file"), mcNewWaypointsWPT);
			mmSelectWaypointsFile.CreateItem(L("New GPX file"), mcNewWaypointsGPX);
		}
		mmWaypoints.CreateItem(L("Add waypoint"), mcAddWaypoint);
		mmWaypoints.CreateItem(L("Waypoints list"), mcWaypointsList);
		{
			CMenu & mmImportExportWpt = mmWaypoints.CreateSubMenu(L("Import/Export"));
			mmImportExportWpt.CreateItem(L("Import"), mcImportWaypoints);
			mmImportExportWpt.CreateItem(L("Export as WPT"), mcExportWaypointsWPT);
			mmImportExportWpt.CreateItem(L("Export as GPX"), mcExportWaypointsGPX);
			mmImportExportWpt.CreateItem(L("Export as OSM"), mcExportWaypointsOSM);
		}
		mmWaypoints.CreateBreak();
		mmWaypoints.CreateItem(L("Show waypoints"), mcoShowWaypoints);
	}
	{
		CMenu & mmNavigation = mMenu.CreateSubMenu(L("Navigation"));
		mmNavigation.CreateItem(L("Off"), mcStopNavigating);
		mmNavigation.CreateItem(L("Direct to point"), mcNavigatingToPoint);
		mmNavigation.CreateItem(L("Along route"), mcNavigatingAlongRoute);
		mmNavigation.CreateBreak();
		{
			CMenu & mmRoute = mmNavigation.CreateSubMenu(L("Route"));
			mmRoute.CreateItem(L("Edit route"), mcEditRoute);
			mmRoute.CreateItem(L("Center route target"), mcCenterRouteTarget);
			mmRoute.CreateItem(L("Import from track"), mcLoadRoute);
			mmRoute.CreateItem(L("Export as track"), mcSaveRoute);
			mmRoute.CreateItem(L("Close route"), mcClearRoute);
			mmRoute.CreateItem(L("Info route"), mcInfoCurRoute);
		}
		{
			CMenu & mmTraffic = mmNavigation.CreateSubMenu(L("Traffic online (beta)"));
			mmTraffic.CreateItem(L("Refresh traffic information"), mcRefreshTraffic);
			mmTraffic.CreateItem(L("About traffic"), mcAboutTraffic);
			mmTraffic.CreateBreak();
			mmTraffic.CreateItem(L("Refresh traffic on startup"), mcoRefreshTrafficOnStartup);
			// mmTraffic.CreateItem(L("Show traffic nodes"), mcoShowTrafficNodes);
			mmTraffic.CreateItem(L("Show fastest way"), mcoShowFastestWay);
			mmTraffic.CreateItem(L("Show traffic information"), mcoShowTrafficInformation);
			if (m_Options[mcoDebugMode]) {
				mmTraffic.CreateItem(L("Use test server"), mcoTestServer);
				mmTraffic.CreateItem(L("Show traffic flags"), mcoTrafficFlags);
			}
		}
		mmNavigation.CreateItem(L("Navigate recent"), mcNavigateRecent);
		mmNavigation.CreateItem(L("Show Sun azimuth"), mcoShowSunAz);
	}
	{
		CMenu & mmDisplay = mMenu.CreateSubMenu(L("Display"));
		{
			CMenu & mmView = mmDisplay.CreateSubMenu(L("View"));
			mmView.CreateItem(L("Zoom in"), mcZoomIn);
			mmView.CreateItem(L("Zoom out"), mcZoomOut);
			mmView.CreateItem(L("Left"), mcLeft);
			mmView.CreateItem(L("Right"), mcRight);
			mmView.CreateItem(L("Up"), mcUp);
			mmView.CreateItem(L("Down"), mcDown);
			mmView.CreateBreak();
			mmView.CreateItem(L("Follow cursor"), mcoFollowCursor);
		}
		{
			CMenu & mmMonitorBar = mmDisplay.CreateSubMenu(L("Monitor bar"));
			mmMonitorBar.CreateItem(L("Previous monitors row"), mcPrevMonitorsRow);
			mmMonitorBar.CreateItem(L("Next monitors row"), mcNextMonitorsRow);
			mmMonitorBar.CreateBreak();
			mmMonitorBar.CreateItem(L("Show monitor bar"), mcoShowMonitorBar);
			mmMonitorBar.CreateItem(L("Large monitors"), mcoLargeMonitors);
		}
		mmDisplay.CreateItem(L("Open color scheme"), mcOpenColors);
		mmDisplay.CreateItem(L("Close color scheme"), mcCloseColors);

		mmDisplay.CreateBreak();
		mmDisplay.CreateItem(L("Monitors mode"), mcoMonitorsMode);
		mmDisplay.CreateItem(L("Full screen"), mcoFullScreen);
		mmDisplay.CreateItem(L("Show center"), mcoShowCenter);
		mmDisplay.CreateItem(L("Rotate map by course"), mcoRotateMap); 
#ifndef SMARTPHONE
		mmDisplay.CreateItem(L("Show screen buttons"), mcoScreenButtons);
#endif // !SMARTPHONE
#ifdef SMARTPHONE
		mmDisplay.CreateItem(L("Low light"), mcoLowLight);
#endif // SMARTPHONE
		mmDisplay.CreateItem(L("Large fonts"), mcoLargeFonts);
	}
	{
		CMenu & mmGPS = mMenu.CreateSubMenu(L("GPS"));
		{
			CMenu & mmTeam = mmGPS.CreateSubMenu(L("Team GPS"));
			mmTeam.CreateItem(L("Settings"), mcTeamSettings);
			mmTeam.CreateItem(L("Update now"), mcTeamUpdateNow);
			mmTeam.CreateBreak();
			mmTeam.CreateItem(L("Update periodically"), mcTeamUpdatePeriodically);
		}
		{
			CMenu & mmPeriod = mmGPS.CreateSubMenu(L("Connect period"));
			mmPeriod.CreateItem(L("Always"), mcConnectPeriod0);
			mmPeriod.CreateItem(L("1 minute"), mcConnectPeriod1);
			mmPeriod.CreateItem(L("2 minutes"), mcConnectPeriod2);
			mmPeriod.CreateItem(L("4 minutes"), mcConnectPeriod4);
			mmPeriod.CreateItem(L("9 minutes"), mcConnectPeriod9);
		}
		mmGPS.CreateItem(L("Synchronize time"), mcSetTime);
		mmGPS.CreateItem(L("Dump to file"), mcDumpNMEA);
		mmGPS.CreateBreak();
		mmGPS.CreateItem(L("Connect GPS"), mcoConnect);
		mmGPS.CreateItem(L("Warn on GPS loss"), mcoWarnNoGPS); 
#ifdef SMARTPHONE
		mmGPS.CreateItem(L("Turn bluetooth on"), mcoBluetoothOn);
		mmGPS.CreateItem(L("Automatic light"), mcoAutoLight);
#endif // SMARTPHONE
	}
	{
		CMenu & mmSetup = mMenu.CreateSubMenu(L("Setup"));
		mmSetup.CreateItem(L("Settings"), mcSettings);
		mmSetup.CreateItem(L("Keymap"), mcKeymap);
		mmSetup.CreateItem(L("Register file types"), mcRegisterFileTypes);
		mmSetup.CreateItem(L("Open translation"), mcOpenTranslation);
		mmSetup.CreateItem(L("Close translation"), mcCloseTranslation);
		mmSetup.CreateBreak();
		mmSetup.CreateItem(L("Sound"), mcoSound);
		mmSetup.CreateItem(L("Keep backlight"), mcoKeepBacklight);
		mmSetup.CreateItem(L("Keep memory low"), mcoLowMemory);
#ifndef SMARTPHONE
		mmSetup.CreateItem(L("Keep device on"), mcoKeepDeviceOn);
#endif // SMARTPHONE
		{
#if UNDER_CE >= 0x500
			CMenu & mmInet = mmSetup.CreateSubMenu(L("Allow internet connection"));
			mmInet.CreateItem(L("Never"), mcoAllowInternetNever);
			mmInet.CreateItem(L("Always"), mcoAllowInternetAlways);
			mmInet.CreateItem(L("Home network only"), mcoAllowInternetHomeOnly);
#else // UNDER_CE >= 0x500
		mmSetup.CreateItem(L("Allow internet connection"), mcoAllowInternetAlways);
#endif // UNDER_CE >= 0x500
		}
		mmSetup.CreateItem(L("Use proxy server"), mcoUseProxy);
	}
	{
		CMenu & mmHelp = mMenu.CreateSubMenu(L("Help"));
		mmHelp.CreateItem(L("About"), mcAbout);
		mmHelp.CreateItem(L("Check newer versions online"), mcCheckLatestVersion);
	}
	mMenu.CreateItem(L("Exit"), mcExit);
	if (m_Options[mcoDebugMode])
	{
		CMenu & mSubMenu = mMenu.CreateSubMenu(L("Debug"));
		mSubMenu.CreateItem(L("NMEA commands"), mcDebugNmeaCommands);
		mSubMenu.CreateItem(L("Cursor here"), mcDebugCursorHere);
		mSubMenu.CreateItem(L("No fix"), mcDebugNoFix);
		mSubMenu.CreateItem(L("Show times"), mcDebugShowTimes);
		mSubMenu.CreateItem(L("Unknown point types"), mcUnknownPointTypes);
		mSubMenu.CreateItem(L("Replay track"), mcReplayTrack);
		mSubMenu.CreateItem(L("Replay NMEA sequence"), mcReplayNMEA);
		mSubMenu.CreateBreak();
		mSubMenu.CreateItem(L("Buffer output"), mcoBuffered);
		mSubMenu.CreateItem(L("Debug mode"), mcoDebugMode);
		mSubMenu.CreateItem(L("Write connection log"), mcoWriteConnectionLog);
		mSubMenu.CreateItem(L("Direct paint"), mcoDirectPaint);
	}
	GetKeymap().AddAction(mcContextMenu, L("Context menu"));
	GetKeymap().AddAction(mcLeftClickOrContextMenu, L("Click or else context menu"));

	GetKeymap().Load();
	GetButtons().Load();
	CheckMenu();
}

void CMapApp::InitMenuAllWMSMaps(CMenu& baseMenu)
{
	for(int i=0, iEnd=m_pRasterMapPainter->GetWMSMapCount();
		i<iEnd; i++)
	{
		baseMenu.CreateItem(m_pRasterMapPainter->GetWMSMapName(i).c_str(), mcFirstWMSMapType+i);
	}
}

void CMapApp::CheckMenu()
{
	CMenu & menu = m_MenuBar.GetMenu();
	m_Options.CheckMenu(m_MenuBar);
	for (int i = mcSetDetail1; i <= mcSetDetail5; ++i)
		menu.CheckMenuItem(i, m_riDetail() == (i - mcSetDetail3));

	menu.CheckMenuItem(mcConnectPeriod0, m_riConnectPeriodMin() == 0);
	menu.CheckMenuItem(mcConnectPeriod1, m_riConnectPeriodMin() == 1);
	menu.CheckMenuItem(mcConnectPeriod2, m_riConnectPeriodMin() == 2);
	menu.CheckMenuItem(mcConnectPeriod4, m_riConnectPeriodMin() == 4);
	menu.CheckMenuItem(mcConnectPeriod9, m_riConnectPeriodMin() == 9);

	menu.CheckMenuItem(mcTrackFormatPLT, m_riTrackFormat() == 0);
	menu.CheckMenuItem(mcTrackFormatGPX, m_riTrackFormat() == 1);

	for (int i = 0, iEnd = m_pRasterMapPainter->GetGMapCount();
		 i < iEnd; ++i)
		menu.CheckMenuItem(mcGMapType + i, i == m_riGMapType());

	DWORD allowInet = m_riAllowInternet();
#if UNDER_CE >= 0x500
	menu.CheckMenuItem(mcoAllowInternetNever, allowInet == 0);
	menu.CheckMenuItem(mcoAllowInternetAlways, (allowInet & ALLOW_INTERNET_ALWAYS) != 0);
	menu.CheckMenuItem(mcoAllowInternetHomeOnly, (allowInet & ALLOW_INTERNET_HOME_ONLY) != 0);
#else
	menu.CheckMenuItem(mcoAllowInternetAlways, allowInet != 0);
#endif // UNDER_CE
	menu.EnableMenuItem(mcCloseColors, !m_rsToolsFile().empty());
	bool fGarminMaps = !m_atlas.IsEmpty();
	menu.EnableMenuItem(mcoShowGarminMaps, fGarminMaps);
	menu.EnableMenuItem(mcoShowPOIs, fGarminMaps);
	menu.EnableMenuItem(mcoShowDetailMaps, fGarminMaps);
	menu.EnableMenuItem(mcoShowUnknownTypes, fGarminMaps);
	menu.EnableMenuItem(mcoShowPolygonLabels, fGarminMaps);
	menu.EnableMenuItem(mcoShowAreaAsOutline, fGarminMaps);
	menu.EnableMenuItem(mcoShowRoadName, fGarminMaps);
	menu.EnableMenuItem(mcoShowAreaName, fGarminMaps);
	menu.EnableMenuItem(mcCloseAllMaps, fGarminMaps);
	menu.EnableMenuItem(mcMapList, fGarminMaps);
	menu.EnableMenuItem(mcDownlRasterStartWithCurZoom, 
		m_Options[mcoDownloadGoogleMaps] && m_pRasterMapPainter->IsSelectingZoomToDownload());
	menu.EnableMenuItem(mcDownlRasterAddCurrentView, m_Options[mcoDownloadGoogleMaps]);
	
	menu.EnableMenuItem(mcCloseTranslation, !m_rsTranslationFile().empty());

	menu.CheckMenuItem(mcDumpNMEA, !m_NMEAParser.GetFilename().empty());
	menu.EnableMenuItem(mcGoToDemoPoint, m_pRasterMapPainter->HasDemoPoint(enumGMapType(m_riGMapType())) );

	menu.CheckMenuItem(mcEditRoute, m_Tracks.IsEditingRoute());
	menu.CheckMenuItem(mcStopNavigating, (nmOff == m_riNavigate()));
	menu.CheckMenuItem(mcNavigatingToPoint, (nmPoint == m_riNavigate()));
	menu.CheckMenuItem(mcNavigatingAlongRoute, (nmRoute == m_riNavigate()));
}

void CMapApp::PaintCursor(const GeoPoint & gp, bool fCursorVisible)
{
	if (fCursorVisible)
	{
		if (m_Options[mcoShowSunAz])
		{
			double az = (m_Sun.m_dSunAzimuth - m_painter.GetScreenRotationAngle()) * (pi/180.);
			double sunradius = 16./60/180*pi;
			ScreenPoint sp = m_painter.GeoToScreen(gp);
			int iScrSize = (std::max)(m_painter.GetScreenRect().Width(), m_painter.GetScreenRect().Height());
			iScrSize *= 10; // !!! fix this later
			m_painter.StartPolyline(0x204, 0);
			m_painter.AddPoint(sp);
			m_painter.AddPoint(ScreenPoint(sp.x + int(iScrSize*sin(az+sunradius)), sp.y - int(iScrSize*cos(az+sunradius))));
			m_painter.FinishObject();
			m_painter.StartPolyline(0x204, 0);
			m_painter.AddPoint(sp);
			m_painter.AddPoint(ScreenPoint(sp.x + int(iScrSize*sin(az-sunradius)), sp.y - int(iScrSize*cos(az-sunradius))));
			m_painter.FinishObject();
		}
		if (nmOff != m_riNavigate())
		{
			if (!m_Options[mcoShowFastestWay] || !m_TrafficNodes.PaintFastestWay(gp, m_gpNavigate(), &m_painter))
			{
				m_painter.StartPolyline(0x200, 0);
				m_painter.AddPoint(gp);
				m_painter.AddPoint(m_gpNavigate());
				m_painter.FinishObject();
			}
		}
		m_painter.PaintPoint(0x10000, gp, 0);
	}
	if (m_Options[mcoShowCenter] && (!fCursorVisible || m_painter.GetCenterCross() != m_painter.GetCenter() || gp != m_painter.GetCenter()))
	{
		m_painter.PaintPoint(0x10001, m_painter.GetCenterCross(), NULL);
	}
}

void CMapApp::ToolsWaypoints()
{
	static CWaypointsDlg dlg;
	g_pNextDialog = &dlg;
	DialogBox(g_hInst, (LPCTSTR)IDD_TEMPLATE, m_hWnd, (DLGPROC)MADlgProc);
}

void CMapApp::OptionsSettings()
{
	static CSettingsDlg dlg;
	g_pNextDialog = &dlg;
	DialogBox(g_hInst, (LPCTSTR)IDD_TEMPLATE, m_hWnd, (DLGPROC)MADlgProc);
}

void CMapApp::TeamSettings()
{
	static CTeamSettingsDlg dlg;
	g_pNextDialog = &dlg;
	DialogBox(g_hInst, (LPCTSTR)IDD_TEMPLATE, m_hWnd, (DLGPROC)MADlgProc);
}

void CMapApp::OptionsKeymap()
{
	static CKeymapDlg dlg;
	g_pNextDialog = &dlg;
	DialogBox(g_hInst, (LPCTSTR)IDD_TEMPLATE, m_hWnd, (DLGPROC)MADlgProc);
}

void CMapApp::DebugNmeaCommands()
{
	static CNmeaCommandsDlg dlg;
	g_pNextDialog = &dlg;
	DialogBox(g_hInst, (LPCTSTR)IDD_TEMPLATE, m_hWnd, (DLGPROC)MADlgProc);
}

void CMapApp::DebugUnknownPointTypes()
{
	static CUnknownPointTypesDlg dlg;
	g_pNextDialog = &dlg;
	DialogBox(g_hInst, (LPCTSTR)IDD_TEMPLATE, m_hWnd, (DLGPROC)MADlgProc);
}

void CMapApp::ToolsMaps()
{
	static CMapsDlg dlg;
	g_pNextDialog = &dlg;
	DialogBox(g_hInst, (LPCTSTR)IDD_TEMPLATE, m_hWnd, (DLGPROC)MADlgProc);
}

bool CMapApp::ToolsWaypointProperties(int iPoint)
{
	static CWaypointPropertiesDlg dlg;
	dlg.m_iWaypoint = iPoint;
	g_pNextDialog = &dlg;
	return DialogBox(g_hInst, (LPCTSTR)IDD_TEMPLATE, m_hWnd, (DLGPROC)MADlgProc) != 0;
}

void CMapApp::AddOdometer(double dDist)
{
	m_monOdometer1 += dDist;
	m_monOdometer2 += dDist;
	m_monOdometerTotal += dDist;
}

bool CMapApp::ProcessCommand(WPARAM wp)
{
	wp = m_MenuBar.CheckCommand(wp);
    switch (wp)
    {
		case mcExit:
			app.FileExit();
			break;
		case mcZoomIn:
			ViewZoomIn();
			break;
		case mcZoomOut:
			ViewZoomOut();
			break;
		case mcLeft:
			ViewLeft();
			break;
		case mcRight:
			ViewRight();
			break;
		case mcUp:
			ViewUp();
			break;
		case mcDown:
			ViewDown();
			break;
		case mcOpenMap:
			FileOpenMap();
			break;
		case mcOpenMapFolder:
			FileOpenMapFolder();
			break;
		case mcCloseAllMaps:
			FileCloseAllMaps();
			break;
		case mcOpenTrack:
			FileOpenTrack();
			break;
		case mcOpenWaypoints:
			FileOpenWaypoints();
			break;
		case mcNewWaypointsWPT:
			FileNewWaypointsWPT();
			break;
		case mcNewWaypointsGPX:
			FileNewWaypointsGPX();
			break;
		case mcImportWaypoints:
			FileImportWaypoints();
			break;
		case mcExportWaypointsWPT:
			FileExportWaypointsWPT();
			break;
		case mcExportWaypointsGPX:
			FileExportWaypointsGPX();
			break;
		case mcExportWaypointsOSM:
			FileExportWaypointsOSM();
			break;
		case mcOpenColors:
			FileOpenColors();
			break;
		case mcCloseColors:
			FileCloseColors();
			break;
		case mcOpenTranslation:
			FileOpenTranslation();
			break;
		case mcCloseTranslation:
			FileCloseTranslation();
			break;
		case mcCheckLatestVersion:
			m_fCheckLatestVersion = true;
			break;
		case mcNextColors:
			FileNextColors();
			break;
		case mcTrackList:
			ToolsTracks();
			break;
		case mcSearchResults:
			if (m_Found.Empty())
				MessageBox(m_hWnd, L("Nothing found"), L("Search results"), MB_ICONINFORMATION | MB_OK);
			else
				SearchResults();
			break;
		case mcSearchOSM:
			SearchOSM();
			break;
		case mcStopNavigating:
			ToolsStopNavigating();
			break;
		case mcNavigatingToPoint:
			m_riNavigate.Set(nmPoint);
			m_painter.Redraw();
			CheckMenu();
			break;
		case mcNavigatingAlongRoute:
			ToolsNavigateRoute();
			break;
		case  mcSetTrackFolder:
			OptionsSetTrackFolder();
			break;
		case mcAddWaypoint:
			{
				wchar_t wcPoint[1000];
				SYSTEMTIME st;
				GetLocalTime(&st);
				wsprintf(wcPoint, L"%04d.%02d.%02d %02d:%02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
				int id = AddPointCenter(wcPoint);
				if (!ToolsWaypointProperties(id))
					m_Waypoints.DeleteByID(id);
			}
			break;
		case mcRefreshTraffic:
			m_TrafficNodes.RefreshTrafficData();
			break;
		case mcAboutTraffic:
			MessageBox(m_hWnd, L("To read about road traffic please visit http://gpsvp.com/?cat=10"), L("About traffic"), MB_ICONINFORMATION);
			break;
		case mcAboutMaps:
			MessageBox(m_hWnd, L("To read about maps for gpsVP please visit http://gpsvp.com/?cat=12"), L("About maps"), MB_ICONINFORMATION);
			break;
		case mcWaypointsList:
			ToolsWaypoints();
			break;
		case mcMapList:
			ToolsMaps();
			break;
		case mcNextMonitorsRow:
			NextMonitorsRow();
			break;
		case mcPrevMonitorsRow:
			PrevMonitorsRow();
			break;
		case mcContextMenu:
			ContextMenu();
			break;
		case mcLeftClickOrContextMenu:
			LeftClickOrContextMenu();
			break;
		case mcNewTrack:
			NewTrack();
			break;
		case mcNavigateRecent:
			ToolsNavigateRecent();
			break;
		case mcSettings:
			OptionsSettings();
			break;
		case mcTeamSettings:
			TeamSettings();
			break;
		case mcTeamUpdateNow:
			m_team.Update();
			break;
		case mcKeymap:
			OptionsKeymap();
			break;
		case mcDebugNmeaCommands:
			DebugNmeaCommands();
			break;
		case mcDebugCursorHere:
			DebugCursorHere();
			break;
		case mcDebugNoFix:
			DebugNoFix();
			break;
		case mcDebugShowTimes:
			DebugShowTime();
			break;
		case mcUnknownPointTypes:
			DebugUnknownPointTypes();
			break;
		case mcSetDetail1:
		case mcSetDetail2:
		case mcSetDetail3:
		case mcSetDetail4:
		case mcSetDetail5:
			SetDetail(int(wp) - mcSetDetail3);
			break;
		case mcConnectPeriod0:
			SetConnectPeriod(0);
			break;
		case mcConnectPeriod1:
			SetConnectPeriod(1);
			break;
		case mcConnectPeriod2:
			SetConnectPeriod(2);
			break;
		case mcConnectPeriod4:
			SetConnectPeriod(4);
			break;
		case mcConnectPeriod9:
			SetConnectPeriod(9);
			break;
		case mcIncreaseDetail:
			SetDetail(m_riDetail() + 1);
			break;
		case mcPrevGMapType:
			{
				long mapCount = m_pRasterMapPainter->GetGMapCount();
				SetGMapType((m_riGMapType() - 1 + mapCount) % mapCount);
			}
			break;
		case mcNextGMapType:
			{
				long mapCount = m_pRasterMapPainter->GetGMapCount();
				SetGMapType((m_riGMapType() + 1 + mapCount) % mapCount);
			}
			break;
		case mcDownlRasterAddCurrentView:
			DRMAddCurrentView();
			break;
		case mcDownlRasterStartWithCurZoom:
			DRMStartWithCurrentZoom();
			break;
		case mcDownlRasterByTrack:
			DRMByTrack();
			break;
		case mcRastMapsRefreshAll:
			DRMRefreshAll();
			break;
		case mcRastMapsRefreshInsideRegion:
			DRMRefreshInsideRegion();
			break;
		case mcDecreaseDetail:
			SetDetail(m_riDetail() - 1);
			break;
		case mcRegisterFileTypes:
			RegisterFileTypes();
			break;
		case mcAbout:
			About();
			break;
		case mcSetTime:
			m_NMEAParser.SetTime();
			break;
		case mcDumpNMEA:
			DumpNMEA();
			break;
		case mcReplayTrack:
			ReplayTrack();
			break;
		case mcReplayNMEA:
			ReplayNMEA();
			break;
		case mcSetGoogleMapsFolder:
			SetRasterMapFolder();
			break;
		case mcTrackFormatPLT:
			SetTrackFormat(0);
			break;
		case mcTrackFormatGPX:
			SetTrackFormat(1);
			break;
		case mcGoToDemoPoint:
			GoToDemoPoint();
			break;
		case mcInfoCurTrack:
			InfoTrack(m_hWnd, -1);
			break;
		case mcLoadRoute:
			FileOpenRoute();
			break;
		case mcEditRoute:
			if (m_Tracks.IsEditingRoute())
				StopEditingRoute();
			else
				StartEditingRoute();
			break;
		case mcSaveRoute:
			FileSaveRoute();
			break;
		case mcClearRoute:
			StopEditingRoute();
			m_Tracks.NewRoute();
			ToolsStopNavigateRoute();
			SaveCurrentRoute();
			m_painter.Redraw();
			break;
		case mcInfoCurRoute:
			InfoTrack(m_hWnd, m_Tracks.GetCurRoute().AsTrack());
			break;
		case mcCenterRouteTarget:
			CenterRouteTarget();
			break;
		case mcoAllowInternetNever:
			m_riAllowInternet.Set(0);
			CheckMenu();
			break;
		case mcoAllowInternetAlways:
#if UNDER_CE >= 0x500
			m_riAllowInternet.Set(ALLOW_INTERNET_ALWAYS);
#else
			m_riAllowInternet.Set((m_riAllowInternet() != 0) ? 0 : 1);
#endif
			CheckMenu();
			break;
		case mcoAllowInternetHomeOnly:
			m_riAllowInternet.Set(ALLOW_INTERNET_HOME_ONLY);
			CheckMenu();
			break;
		default:
			{
			    if ((mcGMapType <= wp) && (wp <= mcLastGMapType))
				{
					int iNewType = wp - mcGMapType;
					if (iNewType == m_riGMapType())
						iNewType = 0;
					SetGMapType(iNewType);
					break;
				}
				if (m_Options.ProcessCommand(wp, m_MenuBar))
				{
					switch (wp) {
						case mcoDownloadGoogleMaps:
							CheckMenu();
							break;
					}
					CheckOptions();
					m_painter.Redraw();
				}
			}
    }
	return true;
}

void CMapApp::SetDetail(int iDetail)
{
	if (iDetail < -2 || iDetail > 2)
		return;
	m_riDetail.Set(iDetail);
	CheckMenu();	
	m_painter.Redraw();
}

void CMapApp::SetConnectPeriod(int nPeriod)
{
	m_riConnectPeriodMin.Set(nPeriod);
	CheckMenu();
}

void CMapApp::SetTrackFormat(int nTrackFormat)
{
	m_riTrackFormat.Set(nTrackFormat);
	CheckMenu();
	NewTrack(); // New track to apply change
}

void CMapApp::NextMonitorsRow()
{
	m_MonitorSet.NextRow();
	m_painter.RedrawMonitors();
}

void CMapApp::PrevMonitorsRow()
{
	m_MonitorSet.PrevRow();
	m_painter.RedrawMonitors();
}

void CMapApp::ContextMenu()
{
	ScreenPoint sp;
	{
		if (m_Options[mcoMonitorsMode])
			sp = m_painter.GetActiveMonitorCenter();
		else
			sp = m_painter.GetScreenCenterCross();
	}
	ContextMenu(sp);
}

void CMapApp::LeftClickOrContextMenu()
{
	ScreenPoint sp;
	{
		if (m_Options[mcoMonitorsMode])
			sp = m_painter.GetActiveMonitorCenter();
		else
			sp = m_painter.GetScreenCenterCross();
	}
	LeftClickOrContextMenu(sp, true);
}

void CMapApp::LeftClickOrContextMenu(ScreenPoint sp, bool bAllowContextMenu)
{
	if (!m_Options[mcoMonitorsMode] && m_Tracks.IsEditingRoute())
	{
		m_Tracks.GetCurRoute().InsertPoint(m_painter.ScreenToGeo(sp));
		m_painter.Redraw();
		return;
	}
	else if (bAllowContextMenu)
		ContextMenu(sp);
}

void CMapApp::ContextMenu(ScreenPoint sp)
{
	// Context menu for the monitors?
	if (m_Options[mcoMonitorsMode])
	{
		m_MonitorSet.ContextMenu(m_hWnd, sp);
		m_painter.Redraw();
		return;
	}
    // Context menu for the on screen buttons?
	int iButton = m_painter.CheckButton(sp);
	if (iButton != -1)
	{
		if (GetButtons().ContextMenu(iButton, sp, m_hWnd))
			m_painter.Redraw();
		return;
	}
	// In the map area?
	if (m_painter.WillPaint(sp))
	{
		// Context menu for editing a route?
		if (m_Tracks.IsEditingRoute())
		{
			ContextMenuEditRoute(sp);
			return;
		}
		// Normal context menu for the map?
		else
		{
			ContextMenuMapNormal(sp);
			return;
		}
	}
	else
	{
		// In the monitor bar
		m_MonitorSet.ContextMenu(m_hWnd, sp);
		m_painter.Redraw();
		return;
	}
}

void CMapApp::ContextMenuMapNormal(ScreenPoint sp)
{
	if (!m_painter.WillPaint(sp)) return;

	CMenu mmMenu;
	mmMenu.Init();

	ObjectInfo pinfo, linfo, pginfo;
	GeoPoint gp = m_painter.ScreenToGeo(sp);

	if (m_Options[mcoShowGarminMaps])
	{
		CMenu & mmGarmin = mmMenu.CreateSubMenu(L("Garmin map"));

		pinfo = FindNearestPoint(sp, m_painter.GetScale256() * 50 / 10 / SCALEFACTOR);
		if (pinfo.fPresent)
		{
			CMenu & mmPointMenu = mmGarmin.CreateSubMenu((L("POI: ") + pinfo.GetDescription()).c_str());
			mmPointMenu.CreateItem(L("Center"), 21);
			mmPointMenu.CreateItem(L("Navigate"), 23);
			mmPointMenu.CreateItem(L("Info"), 24);
		}
		else
		{
			mmGarmin.CreateItem(L("No POI near"), -1);
		}

		linfo = FindNearestPolyline(gp, m_painter.GetScale256() * 50 / 10 / SCALEFACTOR);
		if (linfo.fPresent)
			mmGarmin.CreateItem((L("Line: ") + linfo.GetDescription()).c_str(), 30);
		else
			mmGarmin.CreateItem(L("No polyline near"), -1);

		pginfo = FindPolygon(gp);
		if (pginfo.fPresent)
			mmGarmin.CreateItem((L("Area: ") + pginfo.GetDescription()).c_str(), 40);
		else
			mmGarmin.CreateItem(L("No area here"), -1);
	}
	else
	{
		mmMenu.CreateItem(L("Garmin maps disabled"), -1);
	}

	int nPointID = m_Waypoints.GetNearestPoint(gp, m_painter.GetScale256() * 50 / 10 / SCALEFACTOR);
	if (nPointID >= 0)
	{
		CMenu & mmNearestMenu = mmMenu.CreateSubMenu(m_Waypoints.GetLabelByID(nPointID).c_str());
		mmNearestMenu.CreateItem(L("Properties"), 10);
		mmNearestMenu.CreateItem(L("Center"), 11);
		mmNearestMenu.CreateItem(L("Delete"), 12);
		mmNearestMenu.CreateItem(L("Navigate"), 13);
		mmNearestMenu.CreateItem(L("Info"), 14);
		mmNearestMenu.CreateItem(L("Move here"), 15);
		mmNearestMenu.CreateItem(L("Export"), 16);
	}
	else
	{
		mmMenu.CreateItem(L("No waypoints near"), -1);
	}

	mmMenu.CreateItem(L("Add point here"), 1);
	//mmMenu.CreateItem(L("Navigate here"), 2);
	CMenu & mmNavigate = mmMenu.CreateSubMenu(L("Navigate here"));
	{
		mmNavigate.CreateItem(L("Navigate to point"), 2);
		mmNavigate.CreateItem(L("New route to here"), 61);
		mmNavigate.CreateItem(L("Center route target"), 62);
		mmNavigate.CreateItem(L("Edit current route"), 63);
		mmNavigate.CreateItem(L("Stop navigating"), 64);
	}

	if (m_Options[mcoFullScreen])
		mmMenu.CreateItem(L("Full screen off"), 4);
	else
		mmMenu.CreateItem(L("Full screen"), 4);
	if (m_Options[mcoFollowCursor] && m_painter.ManualMode() && m_fFix)
		mmMenu.CreateItem(L("Restore follow cursor"), 6);
	else
		mmMenu.CreateItem(L("Restore follow cursor"), -1);

	if (m_TrackCompetition.IsStarted())
		mmMenu.CreateItem(L("Stop track competition"), 8);
	else
		mmMenu.CreateItem(L("Start track competition here"), 7);

	//if (m_Options[mcoDebugMode])
	//	mmMenu.CreateItem(L("Navigate azimuth"), 5);
	if (m_Options[mcoDebugMode])
		mmMenu.CreateItem(L("Debug: cursor here"), 3);

	int iPointIndexOnNearestTrack;
	CTrack & nearestTrack = m_Tracks.GetNearestTrack(gp, iPointIndexOnNearestTrack);
	CTrack & currentTrack = m_Tracks.GetCurTrack();
	CMenu & mmTrack = (&nearestTrack == &currentTrack)
	                  ? mmMenu.CreateSubMenu(L("Nearest track: current"))
					  : mmMenu.CreateSubMenu(nearestTrack.GetExtFilename().c_str());
	{
		mmTrack.CreateItem(L("Information"), 51);
		mmTrack.CreateItem(L("Set start cursor"), 52);
		mmTrack.CreateItem(L("Set end cursor"), 53);
		mmTrack.CreateItem(L("Reset cursors"), 54);
		if (&nearestTrack == &currentTrack)
			mmTrack.CreateItem(L("Follow as route"), -1);
		else
			mmTrack.CreateItem(L("Follow as route"), 55);
	}


//		RECT rect = m_painter.GetScreenRect();
	DWORD res = mmMenu.Popup(sp.x, sp.y, m_hWnd);
	std::wstring wstrLon, wstrLat;

	switch(res)
	{
	case 1:
		{
			wchar_t wcPoint[1000];
			SYSTEMTIME st;
			GetLocalTime(&st);
			wsprintf(wcPoint, L"%04d.%02d.%02d %02d:%02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
			int id = AddPointScreen(sp, wcPoint);
			if (!ToolsWaypointProperties(id))
				m_Waypoints.DeleteByID(id);
			break;
		}
	case 2:
		Navigate(sp, L("Screen"));
		break;
	case 3:
		{
			Fix(gp, 0, 100);
			break;
		}
	case 4:
		m_Options.ProcessCommand(mcoFullScreen, m_MenuBar);
		CheckOptions();
		m_painter.Redraw();
		break;
	case 5:
		{
			//static CAzimuthDlg dlg;
			//dlg.gp = gp;
			//g_pNextDialog = &dlg;
			//if (DialogBox(g_hInst, (LPCTSTR)IDD_TEMPLATE, m_hWnd, (DLGPROC)MADlgProc) != IDC_CANCEL)
			//	Navigate(dlg.gp, "Azimuth");
		}
		break;
	case 6:
		m_painter.ResetManualMode();
		break;
	case 7:
		{
			m_TrackCompetition.Start(gp);
			m_painter.Redraw();
			break;
		}
	case 8:
		m_TrackCompetition.Stop();
		m_painter.Redraw();
		break;
	case 10:
		ToolsWaypointProperties(nPointID);
		m_painter.Redraw();
		break;
	case 11:
		m_painter.SetView(m_Waypoints.GetPointByIDApprox(nPointID), true);
		m_painter.Redraw();
		break;
	case 12:
		if (MessageBox(m_hWnd, L("Are you sure"), L("Delete waypoint"), MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
		{
			m_Waypoints.DeleteByID(nPointID);
			m_painter.Redraw();
		}
		break;
	case 13:
		Navigate(m_Waypoints.GetPointByIDApprox(nPointID), m_Waypoints.GetLabelByID(nPointID).c_str());
		m_painter.Redraw();
		break;
	case 14:
		{
			CWaypoints::CPoint & p = m_Waypoints.ById(nPointID);
			CoordToText(p.Longitude(), p.Latitude(), wstrLon, wstrLat);
			MessageBox(0, (std::wstring(L"")
				+ L("Name: ") + p.GetLabel()
				+ L"\n" + L("Latitude: ") + wstrLat
				+ L"\n" + L("Longitude: ") + wstrLon
				+ L"\n" + L("Altitude: ") + DoubleToText(p.GetAltitude())
				).c_str(),
				L("Waypoint info"),MB_ICONINFORMATION);
		}
		break;
	case 15:
		m_Waypoints.MovePoint(nPointID, gp);
		m_painter.Redraw();
		break;
	case 16:
		ExportWaypoint(nPointID, m_hWnd);
		break;
	case 21:
		m_painter.SetView(pinfo.gp, true);
		m_painter.Redraw();
		break;
	case 23:
		Navigate(pinfo.gp, pinfo.GetDescription().c_str());
		m_painter.Redraw();
		break;
	case 24:
		CoordToText(Degree(pinfo.gp.lon), Degree(pinfo.gp.lat), wstrLon, wstrLat);
		MessageBox(0, (std::wstring(L"")
			+ L("Name: ") + pinfo.wstrName
			+ L"\n" + L("Type: ") + m_TypeInfo.PointType(pinfo.uiType)
			+ L"\n" + L("Code: ") + IntToHex(pinfo.uiType)
			+ L"\n" + L("Latitude: ") + wstrLat
			+ L"\n" + L("Longitude: ") + wstrLon
			).c_str(),
			L("Point info"),MB_ICONINFORMATION);
		break;
	case 30:
		MessageBox(0, (std::wstring(L"")
			+ L("Name: ") + linfo.wstrName
			+ L"\n" + L("Type: ") + m_TypeInfo.PolylineType(linfo.uiType)
			).c_str(),
			L("Line info"),MB_ICONINFORMATION);
		break;
	case 40:
		MessageBox(0, (std::wstring(L"")
			+ L("Name: ") + pginfo.wstrName
			+ L"\n" + L("Type: ") + m_TypeInfo.PolygonType(pginfo.uiType)
			).c_str(),
			L("Area info"),MB_ICONINFORMATION);
		break;
	case 51:
		InfoTrack(m_hWnd, nearestTrack);
		break;
	case 52:
		nearestTrack.SetStartCursor(iPointIndexOnNearestTrack);
		m_painter.Redraw();
		break;
	case 53:
		nearestTrack.SetEndCursor(iPointIndexOnNearestTrack);
		m_painter.Redraw();
		break;
	case 54:
		nearestTrack.SetStartCursor(-1);
		nearestTrack.SetEndCursor(-1);
		m_painter.Redraw();
		break;
	case 55:
		if (m_Tracks.NewRouteFromTrack(nearestTrack))
		{
			SaveCurrentRoute(); // Save it as route.plt
			ToolsNavigateRoute();
		}
		break;
	case 61:
		if (m_Tracks.NewRoute())
		{
			if (m_fFix)
				m_Tracks.GetCurRoute().AppendPoint(m_gpCursor);
			m_Tracks.GetCurRoute().AppendPoint(gp);
			StartEditingRoute();
			ToolsNavigateRoute();
		}
		break;
	case 62:
		CenterRouteTarget();
		break;
	case 63:
		StartEditingRoute();
		break;
	case 64:
		ToolsStopNavigating();
		break;
	}
}

void CMapApp::ContextMenuEditRoute(ScreenPoint sp)
{
	if (!m_painter.WillPaint(sp)) return;

	CMenu mmMenu;
	mmMenu.Init();

	GeoPoint gp = m_painter.ScreenToGeo(sp);

	mmMenu.CreateItem(L("Insert point here"),		1);
	if (m_Tracks.GetCurRoute().CanUndo())
		mmMenu.CreateItem(L("Undo"),				2);
	else
		mmMenu.CreateItem(L("Undo"),				-1);
	CMenu & mmInsertMode = mmMenu.CreateSubMenu(L("Insert mode"));
	{
		enumRouteInsertMode insertMode = m_Tracks.GetCurRoute().GetInsertMode();
		mmInsertMode.CreateItem(L("Nearest segment (always)"), 21);
		mmInsertMode.CheckMenuItem(21, rimNearestSegment == insertMode);
		mmInsertMode.CreateItem(L("Backwards (nearest segment once)"), 22);
		mmInsertMode.CheckMenuItem(22, rimBackwards == insertMode);
		mmInsertMode.CreateItem(L("Forwards (nearest segment once)"), 23);
		mmInsertMode.CheckMenuItem(23, rimForwards == insertMode);
	}
	mmMenu.CreateItem(L("Erase nearest point"),		3);
	mmMenu.CreateItem(L("Reverse route"),			4);
	mmMenu.CreateItem(L("Stop editing"),			5);
	mmMenu.CreateItem(L("Export as track"),			6);
	mmMenu.CreateItem(L("Main context menu"),		7);
	// Extra menu which shows the current length:
	std::wstring wstrLength = std::wstring(L("Length")) + L": " + DistanceToText(m_Tracks.GetCurRoute().AsTrack().GetFullLength());
	mmMenu.CreateItem(wstrLength.c_str(),			8);

	DWORD res = mmMenu.Popup(sp.x, sp.y, m_hWnd);

	switch(res)
	{
	case 1:
		m_Tracks.GetCurRoute().InsertPoint(gp);
		m_painter.Redraw();
		break;
	case 2:
		m_Tracks.GetCurRoute().Undo();
		m_painter.Redraw();
		break;
	case 21:
		m_Tracks.GetCurRoute().SetInsertMode(rimNearestSegment);
		m_painter.Redraw();
		break;
	case 22:
		m_Tracks.GetCurRoute().SetInsertMode(rimBackwards, gp);
		m_painter.Redraw();
		break;
	case 23:
		m_Tracks.GetCurRoute().SetInsertMode(rimForwards, gp);
		m_painter.Redraw();
		break;
	case 3:
		{
			int iDist;
			int iPointIndexOnRoute = m_Tracks.GetCurRoute().AsTrack().FindNearestPointIndex(gp, iDist);
			if (-1 == iPointIndexOnRoute) iPointIndexOnRoute = m_Tracks.GetCurRoute().GetPointCount()-1;
			m_Tracks.GetCurRoute().ErasePoint(iPointIndexOnRoute);
		}
		m_painter.Redraw();
		break;
	case 4:
		m_Tracks.GetCurRoute().Reverse();
		CenterRouteTarget();
		break;
	case 5:
		StopEditingRoute();
		break;
	case 6:
		FileSaveRoute();
		break;
	case 7:
		ContextMenuMapNormal(sp);
		break;
	case 8:
		InfoTrack(m_hWnd, m_Tracks.GetCurRoute().AsTrack());
		break;
	}
}

class CObjectFinder : public IPainter
{
private:
	IPainter * m_pRealPainter;
	GeoPoint m_gp;
	double m_dRadius;
	ObjectInfo m_pinfo;
	bool m_fPolyline;
	bool m_fStarted;
	double m_dDistance;
	GeoPoint m_gpLast;
	std::wstring m_wcName;
	UInt m_uiType;
	bool m_fPolygon;
	bool m_fInside;
	bool m_fOnBorder;
	int m_iLastSign;
	GeoPoint m_gpFirst;
	int m_iPlus, m_iMinus, m_iZero, m_iTotal;
public:
	double PointFromLine(const GeoPoint & pt, const GeoPoint & lpt1, const GeoPoint & lpt2)
	{
		double d1 = IntDistance(lpt1, pt);
		double d2 = IntDistance(lpt2, pt);
		double d = IntDistance(lpt1, lpt2);
		if (d == 0)
			return d1;
		if (sqr(d2) > sqr(d) + sqr(d1))
			return d1;
		if (sqr(d1) > sqr(d) + sqr(d2))
			return d2;
		double p = (d + d1 + d2) / 2;
		double t = p * (p - d) * (p - d1) * (p - d2);
		if (t < 0)
			return 0;
		else
			return sqrt(t) * 2 / d;
	}
	CObjectFinder(IPainter * pRealPainter, const GeoPoint & gp, double dRadius)
	{
		m_pRealPainter = pRealPainter;
		m_gp = gp;
		m_dRadius = dRadius;
		m_fPolyline = false;
		m_fPolygon = false;
	}
	ObjectInfo GetObjectInfo()
	{
		return m_pinfo;
	}
	// IPainter
	virtual void StartPolygon(UInt uiType, const wchar_t * wcName)
	{
		if (uiType == 75)
			return;
		m_uiType = uiType;
		m_wcName = wcName ? wcName : L"";
		m_fPolygon = true;
		m_fStarted = false;
		m_fInside = false;
		m_fOnBorder = false;
		m_iLastSign = 0;
		m_iTotal = m_iMinus = m_iPlus = m_iZero = 0;
	};
	virtual void StartPolyline(UInt uiType, const wchar_t * wcName)
	{
		m_uiType = uiType;
		m_wcName = wcName ? wcName : L"";
		m_fPolyline = true;
		m_fStarted = false;
		m_dDistance = 40000000;
	}
	virtual void FinishObject()
	{
		if (m_fPolyline)
		{
			if (m_dDistance < m_dRadius)
			{
				m_pinfo.wstrName = m_wcName;
				m_pinfo.uiType = m_uiType;
				m_pinfo.fPresent = true;
				m_dRadius = m_dDistance;
			}
			m_fPolyline = false;
		}
		if (m_fPolygon)
		{
			AddPoint(m_gpFirst);
			if (m_fInside || m_fOnBorder)
			{
				m_pinfo.wstrName = m_wcName;
				m_pinfo.uiType = m_uiType;
				m_pinfo.fPresent = true;
				m_dRadius = m_dDistance;
			}
			m_fPolygon = false;
		}
	}
	virtual void AddPoint(const GeoPoint & gp)
	{
		if (m_fPolyline)
		{
			if (m_fStarted)
				m_dDistance = (std::min)(m_dDistance, PointFromLine(m_gp, m_gpLast, gp));
			else
				m_fStarted = true;
			m_gpLast = gp;
		}
		if (m_fPolygon)
		{
			if (m_fStarted)
			{
				int lat_d1 = m_gpLast.lat - m_gp.lat;
				int lon_d1 = m_gpLast.lon - m_gp.lon;
				int lat_d2 = gp.lat - m_gp.lat;
				int lon_d2 = gp.lon - m_gp.lon;
				if (lat_d1 == 0)
				{
					if (lat_d2 == 0)
					{
						if (sign(lon_d1) * sign(lon_d2) <= 0)
							m_fOnBorder = true;
						else
							0;
					}
					else
					{
						if (sign(lat_d2) * m_iLastSign < 0)
							m_fInside = !m_fInside;
						else
							0;
					}
				}
				else
				{
					if (lat_d2 == 0)
						m_iLastSign = sign(lat_d1);
					else
					{
						if (sign(lat_d1) * sign(lat_d2) < 0)
						{
							++m_iTotal;
							while (abs(lat_d1) > 0x7fff || abs(lat_d2) > 0x7fff || abs(lon_d1) > 0x7fff || abs(lon_d2) > 0x7fff)
							{
								lat_d1 /= 0x100;
								lat_d2 /= 0x100;
								lon_d1 /= 0x100;
								lon_d2 /= 0x100;
							}
							int prod = (lat_d1 * lon_d2 - lat_d2 * lon_d1) * sign(lat_d2);
							if (prod == 0)
							{
								m_fOnBorder = true;
								++m_iZero;
							}
							else if (prod > 0)
							{
								m_fInside = !m_fInside;
								++m_iPlus;
							}
							else
							{
								0;
								++m_iMinus;
							}
						}
						else
							0;
					}
				}
			}
			else
			{
				m_fStarted = true;
				m_gpFirst = gp;
			}
			m_gpLast = gp;
		}
	}
	virtual bool WillPaint(const GeoRect & rect) {return m_pRealPainter->WillPaint(rect);};
	virtual void SetView(const GeoPoint & gp, bool fManual) {};
	virtual void PaintPoint(UInt uiType, const GeoPoint & gp, const wchar_t * wcName)
	{
		if (IntDistance(gp, m_gp) < m_dRadius)
		{
			m_pinfo.wstrName = wcName;
			m_pinfo.gp = gp;
			m_pinfo.uiType = uiType;
			m_pinfo.fPresent = true;
			m_dRadius = IntDistance(gp, m_gp);
		}
	};
	virtual void SetLabelMandatory() {};
	virtual GeoRect GetRect() {return m_pRealPainter->GetRect();};
};

ObjectInfo CMapApp::FindNearestPoint(const ScreenPoint & sp, double dRadius)
{
	CObjectFinder finder(&m_painter, m_painter.ScreenToGeo(sp), dRadius);
	m_atlas.BeginPaint(PrepareScale(m_painter.GetScale256()), &finder, 0);
	m_atlas.Paint(maskPoints, m_Options[mcoDirectPaint]);
	return finder.GetObjectInfo();
}

ObjectInfo CMapApp::FindNearestPolyline(const GeoPoint & gp, double dRadius)
{
	CObjectFinder finder(&m_painter, gp, dRadius);
	m_atlas.BeginPaint(PrepareScale(m_painter.GetScale256()), &finder, 0);
	m_atlas.Paint(maskPolylines, m_Options[mcoDirectPaint]);
	return finder.GetObjectInfo();
}

ObjectInfo CMapApp::FindPolygon(const GeoPoint & gp)
{
	CObjectFinder finder(&m_painter, gp, 0);
	m_atlas.BeginPaint(PrepareScale(m_painter.GetScale256()), &finder, 0);
	m_atlas.Paint(maskPolygons, m_Options[mcoDirectPaint]);
	return finder.GetObjectInfo();
}

class CDistanceMeter : public IPainter
{
	GeoPoint m_pt1;
	GeoPoint m_pt2;
	double m_dResult;
	double m_dFrom1;
	double m_dFrom2;
	double m_dDelta;
	bool m_fPreviousPresent;
	GeoPoint m_gpPrevious;
public:
	CDistanceMeter(const GeoPoint & pt1, const GeoPoint & pt2)
	{
		m_pt1 = pt1;
		m_pt2 = pt2;
		NewTrack();
		m_dDelta = (std::max)(IntDistance(pt1, pt2) / 100, 50);
		m_dResult = 40000000;
	}
	void NewTrack()
	{
		m_fPreviousPresent = false;
		m_dFrom2 = m_dFrom1 = 40000000;
	}
	double GetResult()
	{
		return m_dResult;
	}
	//! Start painting polygon
	virtual void StartPolygon(UInt uiType, const wchar_t * wcName) {}
	//! Start painting polyline
	virtual void StartPolyline(UInt uiType, const wchar_t * wcName) {}
	//! Finish painting polygon
	virtual void FinishObject() {}
	//! Add point to currently painted object
	virtual void AddPoint(const GeoPoint & gp)
	{
		if (m_fPreviousPresent)
		{
			double d = IntDistance(gp, m_gpPrevious);
			m_dFrom1 += d;
			m_dFrom2 += d;
		}
		m_fPreviousPresent = true;
		m_gpPrevious = gp;
		if (IntDistance(m_pt1, gp) < m_dDelta)
		{
			if (m_dFrom2  + IntDistance(m_pt1, gp) < m_dResult)
				m_dResult = m_dFrom2  + IntDistance(m_pt1, gp);
			if (m_dFrom1 > IntDistance(m_pt1, gp))
				m_dFrom1 = IntDistance(m_pt1, gp);
		}
		if (IntDistance(m_pt2, gp) < m_dDelta)
		{
			if (m_dFrom1  + IntDistance(m_pt2, gp) < m_dResult)
				m_dResult = m_dFrom1  + IntDistance(m_pt2, gp);
			if (m_dFrom2 > IntDistance(m_pt2, gp))
				m_dFrom2 = IntDistance(m_pt2, gp);
		}
	}
	//! Check if will paint object
	virtual bool WillPaint(const GeoRect & rect) {return true;}
	//! Set view center coordinates
	virtual void SetView(const GeoPoint & gp, bool fManual) {}
	//! Paint one point
	virtual void PaintPoint(UInt uiType, const GeoPoint & gp, const wchar_t * wcName) {}
	virtual void SetLabelMandatory() {}
	virtual GeoRect GetRect() {return GeoRect();}
};

double CMapApp::GetDistanceByTrack(const GeoPoint & pt1, const GeoPoint & pt2)
{
	AutoLock l;
	if (pt1 == pt2)
		return 0;
	CDistanceMeter meter(pt1, pt2);
	m_Tracks.GetCurTrack().PaintUnlocked(&meter, 0);
	for (CTrackList::iterator it = m_Tracks.GetOldTracks().begin(); it != m_Tracks.GetOldTracks().end();++it)
	{
		meter.NewTrack();
		it->PaintUnlocked(&meter, 0);
	}
	return meter.GetResult();
}

void CMapApp::ProcessCmdLine(const wchar_t * wcCmdLine)
{
	const wchar_t * wcStart = wcCmdLine;
	if (!wcStart)
		return;
	while (*wcStart)
	{
		while (*wcStart && isspace(*wcStart))
			++wcStart;
		if (*wcStart == '"')
		{
			wcStart += 1;
			if (*wcStart)
			{
				const wchar_t * wcFinish = wcStart + 1;
				while (*wcFinish && *wcFinish != '"')
					++wcFinish;
				std::wstring wstrElement;
				wstrElement.assign(wcStart, wcFinish - wcStart);
				ProcessCmdLineElement(wstrElement.c_str());
				wcStart = wcFinish + 1;
			}
		}
		else
		{
			const wchar_t * wcFinish = wcStart;
			while (*wcFinish && !isspace(*wcFinish))
				++wcFinish;
			std::wstring wstrElement;
			wstrElement.assign(wcStart, wcFinish - wcStart);
			ProcessCmdLineElement(wstrElement.c_str());
			wcStart = wcFinish + 1;
		}
	}
}

void CMapApp::ProcessCmdLineElement(const wchar_t * wcCmdLine)
{
	std::wstring wstrCmdLine = wcCmdLine;
	std::wstring wstrExtention = wstrCmdLine.substr(wstrCmdLine.length() - 4, 4);
	for (std::wstring::iterator it = wstrExtention.begin(); it != wstrExtention.end(); ++it)
		*it = towlower(*it);
	if (wstrExtention == L".img")
		m_atlas.Add(wstrCmdLine.c_str(), &m_painter);
	if (wstrExtention == L".plt")
	{
		if (-1 != m_Tracks.OpenTracks(wstrCmdLine))
		{
			m_painter.SetView(m_Tracks.Last().GetLastPoint(), true);
		}
		m_painter.Redraw();

	}
	if (wstrExtention == L".vpc")
		m_painter.InitTools(wstrCmdLine.c_str());
	if (wstrExtention == L".wpt")
	{
		CWaypoints t;
		t.Read(wstrCmdLine.c_str());
		m_Waypoints.Import(t);
	}
}

void CMapApp::NewTrack()
{
	m_Tracks.StartNewTrack();
}

void CMapApp::Exit()
{
	m_fExiting = true;
	m_fStopHttpThread = true;
	if (WAIT_TIMEOUT == WaitForSingleObject(m_hPortThread, 5000))
		TerminateThread(m_hPortThread, 0);
	CloseHandle(m_hPortFile);
	if (WAIT_TIMEOUT == WaitForSingleObject(m_hHttpThread, 5000))
		TerminateThread(m_hHttpThread, 0);
	CloseHandle(m_hHttpThread);
	m_painter.SetFullScreen(false);
#ifdef SMARTPHONE
	if (m_fBluetoothWasTurnedOn)
		BthSetMode(BTH_POWER_OFF);
#endif // UNDER_CE
}

int GetTrackStep()
{
	return app.GetTrackStep();
}

bool IsConnectedStatus(IGPSClient::enumConnectionStatus iStatus)
{
	switch(iStatus) {
	case IGPSClient::csDisabled:
	case IGPSClient::csNotConnected:
		return false;
	case IGPSClient::csNoFix:
	case IGPSClient::csFix:
	case IGPSClient::csPaused:
		return true;
	}
	return false;
}

void CMapApp::SetConnectionStatus(enumConnectionStatus iStatus)
{
	if (iStatus != m_iConnectionStatus)
	{
#ifdef UNDER_CE
		if (IsConnectedStatus(m_iConnectionStatus) != IsConnectedStatus(iStatus) && m_Options[mcoWarnNoGPS] && m_Options[mcoSound])
			PlaySound(L"ProximitySound", g_hInst, SND_RESOURCE | SND_ASYNC);
#endif // UNDER_CE
		m_painter.Redraw();
		m_iConnectionStatus = iStatus;
	}
}
void CMapApp::RegisterFileTypes()
{
	RegisterFileType(L".img", L"gpsVP map file", L"gpsVPMap", m_wstrProgName.c_str());
	RegisterFileType(L".plt", L"gpsVP track file", L"gpsVPTrack", m_wstrProgName.c_str());
	RegisterFileType(L".wpt", L"gpsVP waypoints file", L"gpsVPWaypoints", m_wstrProgName.c_str());
	RegisterFileType(L".vpc", L"gpsVP colors file", L"gpsVPColors", m_wstrProgName.c_str());
}
void CMapApp::About()
{
	wchar_t buffer[10000];
	swprintf(buffer, 10000, L"%s %S\n%s %S\n%s.\n%s",
		L("gpsVP version"), g_gpsVPVersion.AsString().c_str(), L("Built on"), __DATE__, 
		L("GPS navigation software for PCs, handhelds and smartphones"),
		L"Copyright (c) 2005-2008, Vsevolod E. Shorin\nAll rights reserved.\n\n"
		L"Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:\n\n"
		L"* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.\n"
		L"* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.\n"
		L"* Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.\n\n"
		L"THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n"
		);
	MessageBox(m_hWnd, buffer, L("About"), MB_ICONINFORMATION);
}

std::wstring CMapApp::HeightFromFeet(const wchar_t * wcOriginal)
{
	if (!wcOriginal || !wcOriginal[0])
		return L"";
	for (const wchar_t * wc = wcOriginal; *wc; ++wc)
	{
		if (!iswdigit(*wc))
			return wcOriginal;
	}
	double dValue = myatof(wcOriginal);
	return HeightToText(dValue * cdFoot);
}

CKeymap & CMapApp::GetKeymap() 
{
	return m_MenuBar.GetKeymap();
}

CScreenButtons & CMapApp::GetButtons() 
{
	return m_MenuBar.GetButtons();
}

Dict & GetDict()
{
	return app.GetDict();
}

void CMapApp::ReplayTrack()
{
	AutoLock l;
	wchar_t strFile[MAX_PATH + 1] = {0};
	OPENFILENAME of;
	FillOpenFileName(&of, m_hWnd, L("Track files\0*.plt\0"), strFile, false, true);
	if (GetOpenFileName(&of))
	{
		m_ReplayTrack.ReadPLT(strFile);
	}
}

void CMapApp::DumpNMEA()
{
	AutoLock l;
	if (m_NMEAParser.GetFilename().empty())
	{
		wchar_t strFile[MAX_PATH + 1] = {0};
		OPENFILENAME of;
		FillOpenFileName(&of, m_hWnd, L("All files\0*.*\0"), strFile, false, false);
		if (GetSaveFileName(&of))
		{
			m_NMEAParser.SetFilename(strFile);
		}
	}
	else
	{
		m_NMEAParser.SetFilename(L"");
	}
	CheckMenu();
}

void CMapApp::HttpThreadRoutine()
{
	HANDLE h = 0;
	{
		AutoLock l;
		m_wstrHttpStatus = L"Idle";
	}

	
	CHttpRequest::SetProxy(app.m_rsProxy());
	std::string request;
	int request_source = 0;
	int ErrorWaitingTime = 1; // beginning with 1 second

	while (!m_fStopHttpThread)
	{
		{
			request = "";
			{
				AutoLock l;
				if (!m_searchurl.empty())
				{
					request = m_searchurl;
					request_source = 4;
				}
			}
			if (m_fCheckLatestVersion)
			{
				request = "http://";
				request += GetServerName(); 
				request += "/LatestVersion.php?version=";
				request += g_gpsVPVersion.AsString();
				request_source = 1;
			}
			if (request.empty() && !m_fStopHttpThread)
			{
				request = m_team.GetRequest();
				if (!request.empty())
					request_source = 5;
			}
			if (request.empty() && !m_fStopHttpThread)
			{
				request = m_TrafficNodes.GetRequest(m_painter.GetCenter());
				if (!request.empty() && ! m_fTrafficAgreement)
				{
					int res = MessageBox(m_hWnd, L("By accepting this you agree to the following: gpsVP is allowed to transfer some information about your movement to server, where the information is used to calculate traffic situation. The impersonated result belongs to Vsevolod Shorin and is publicly available via gpsVP."), L("Traffic information"), MB_OKCANCEL | MB_ICONEXCLAMATION);
					if (res == IDOK)
					{
						m_fTrafficAgreement.Set(true);
					}
					else
					{
						m_TrafficNodes.Disable();
						request = "";
					}
				}
				if (!request.empty())
					request_source = 2;
			}
			if (request.empty() && !m_fStopHttpThread)
			{
				request = m_pRasterMapPainter->GetRequest();
				if (!request.empty())
					request_source = 3;
			}
		}

		{
			AutoLock l;
			m_request = request;
		}

		if (!request.empty() && !m_fStopHttpThread && IsInternetAllowed())
		{
			bool fConnected = true;
#ifdef UNDER_CE
#ifndef BARECE
			if (h)
			{
				DWORD dwStatus;
				ConnMgrConnectionStatus(h, &dwStatus);
				switch (dwStatus)
				{
				case CONNMGR_STATUS_CONNECTED:
					break;
				case CONNMGR_STATUS_WAITINGFORPATH:
				case CONNMGR_STATUS_WAITINGFORRESOURCE:
				case CONNMGR_STATUS_WAITINGFORPHONE:
				case CONNMGR_STATUS_WAITINGFORNETWORK:
				case CONNMGR_STATUS_WAITINGCONNECTION:
				case CONNMGR_STATUS_WAITINGCONNECTIONABORT:
				case CONNMGR_STATUS_WAITINGDISCONNECTION:
					fConnected = false;
					break;
				default:
					ConnMgrReleaseConnection(h, 0);
					h = 0;
					{
						AutoLock l;
						m_wstrHttpStatus = L("Error");
					}
					for (int i = 0; i < 60 && !m_fStopHttpThread; ++i )
						Sleep(1000);
				}
			}
			if (!h)
			{
				fConnected = false;
				CONNMGR_CONNECTIONINFO cc;
				memset(&cc, 0, sizeof(cc));
				cc.cbSize = sizeof(cc);
				cc.dwParams = CONNMGR_PARAM_GUIDDESTNET;
				cc.dwPriority = CONNMGR_PRIORITY_USERINTERACTIVE;
				cc.guidDestNet = IID_DestNetInternet;
				HRESULT res = ConnMgrEstablishConnection(&cc, &h);
				if (res != S_OK)
				{
					AutoLock l;
					m_wstrHttpStatus = L("Error");
					for (int i = 0; i < 60 && !m_fStopHttpThread; ++i )
						Sleep(1000);
					return;
				}
				{
					AutoLock l;
					m_wstrHttpStatus = L"Connecting";
				}
			}
#endif // BARECE
#endif // UNDER_CE
			if (fConnected && !m_fStopHttpThread)
			{
				{
					AutoLock l;
					m_wstrHttpStatus = L"Requesting";
				}
				CHttpRequest req(0);
				CHttpRequest::m_useProxy=m_Options[mcoUseProxy];
				req.Request(request, g_gpsVPVersion.AsStringWithName());
				m_monDataIn += req.GetIncoming();
				m_monDataOut += req.GetOutgoing();
				m_monDataTotal += req.GetIncoming() + req.GetOutgoing();
				if (!m_fStopHttpThread && req.IsGood())
				{
					ErrorWaitingTime = 1;
					{
						AutoLock l;
						m_wstrHttpStatus = L"Ok";
					}
					if (request_source == 3)
					{
						m_pRasterMapPainter->RequestProcessed(request, req.GetResult(), req.GetSize());
						m_painter.Redraw();
					}
					else if (request_source == 1)
					{
						wchar_t buffer[10000];
						wsprintf(buffer, L"%.*S", req.GetSize(), req.GetResult());
						m_wstrVersionMessage = buffer;
						m_fCheckLatestVersion = false;
					}
					else if (request_source == 2)
					{
						AutoLock l;
						m_TrafficNodes.PopRequest(request);
						m_TrafficNodes.TrafficData(request, req.GetResult(), req.GetSize());
						m_painter.Redraw();
					} 
					else if (request_source == 4)
					{
						m_searchurl = "";
						ProcessOSMSearchResult(req.GetResult(), req.GetSize());
						::PostMessage(app.m_hWnd, WM_COMMAND, mcSearchResults, 0);
					}
					else if (request_source == 5)
					{
						m_team.Response(req.GetResult(), req.GetSize());
					}

				}
				else
				{
					{
						AutoLock l;
						m_wstrHttpStatus = L"Error";
					}
					if (request_source == 4)
					{
						m_searchurl = "";
						MessageBox(m_hWnd, L("Search failed"), L("Search"), MB_OK | MB_ICONERROR);
					}
					for (int i = 0; i < ErrorWaitingTime && !m_fStopHttpThread; ++i )
						Sleep(1000);
					ErrorWaitingTime = ErrorWaitingTime << 1; // If error next time then wait twice as long
					ErrorWaitingTime = (ErrorWaitingTime>60)?60:ErrorWaitingTime; // but max 60 seconds
				}
			}
			else if (!m_fStopHttpThread)
				Sleep(1000);
		}
		else if (!m_fStopHttpThread)
		{
			Sleep(1000);
		}
	}
}

void CMapApp::SetGMapType(int type)
{
	m_riGMapType.Set(type);
	m_painter.Redraw();
	CheckMenu();
}

const char * CMapApp::GetServerName()
{
	if (m_Options[mcoTestServer])
		return "beta.gpsvp.com";
	return "gpsvp.com";
}

void CMapApp::DRMAddCurrentView()
{
	m_pRasterMapPainter->DownloadAddCurrentView();
	CheckMenu();
}

void CMapApp::DRMStartWithCurrentZoom()
{
	m_pRasterMapPainter->DownloadStartWithCurrentZoom();
	CheckMenu();
}

void CMapApp::DRMRefreshInsideRegion()
{
	m_pRasterMapPainter->RefreshInsideRegion();
}

void CMapApp::DRMRefreshAll()
{
	m_pRasterMapPainter->RefreshAll();
}

void CMapApp::ProcessWMHIBERNATE()
{
	m_pRasterMapPainter->ProcessWMHIBERNATE();
}

void CMapApp::DRMByTrack()
{
	wchar_t strFile[MAX_PATH + 1] = {0};
	OPENFILENAME of;
	FillOpenFileName(&of, m_hWnd, L("Track files\0*.plt\0"), strFile, false, true);
	if (GetOpenFileName(&of))
	{
		CTrack track;
		track.ReadPLT(strFile);
		if (track.IsPresent())
		{
			// m_pRasterMapPainter->DownloadByTrack(track);
		}
	}
}

void CMapApp::InitCoreDll()
{
#ifdef UNDER_CE
	m_hCoreDll = LoadLibrary(L"coredll.dll");
	if (m_hCoreDll) {
		m_pfnGetIdleTime = 
			(GetIdleTimeProc) GetProcAddress(m_hCoreDll, L"GetIdleTime");
		if (m_pfnGetIdleTime) {
			m_dwLastTickTimer = GetTickCount();
			m_dwLastIdleTick = (*m_pfnGetIdleTime)();
			if (m_dwLastIdleTick == MAXDWORD) {
				// not supported
				m_pfnGetIdleTime = NULL;
			}
		}

		if (!m_pfnGetIdleTime) {
			FreeLibrary(m_hCoreDll);
			m_hCoreDll = NULL;
		}
	} else {
		m_pfnGetIdleTime = NULL;
	}
#else
	FILETIME tmCreatTime, tmExitTime, tmKernel, tmUser;
	if (GetProcessTimes(GetCurrentProcess(), &tmCreatTime, &tmExitTime, &tmKernel, &tmUser)) {
		__int64 *ptmUser = (__int64 *) &tmUser;
		__int64 *ptmKernel = (__int64 *) &tmKernel;
		m_nProcessorUsage = *ptmKernel + *ptmUser;
		m_dwLastTickTimer = GetTickCount();
	} else {
		m_nProcessorUsage = -1;
	}
#endif
}

void CMapApp::SetSearchURL(const char * url)
{
	m_searchurl = url;
}

void CMapApp::AddSearchResult(const std::string & name, const std::string & lat, const std::string & lon)
{
	wchar_t buffer[1001];
	int res = MultiByteToWideChar(CP_UTF8, 0, name.c_str(), name.length(), buffer, 1000);
	buffer[res] = 0;
	m_Found.AddPoint(CWaypoints::CPoint(myatof(lon.c_str()), myatof(lat.c_str()),0, buffer));
}

void CMapApp::ProcessOSMSearchResult(const char * data, int size)
{
	m_Found = CWaypoints();
	enum {nothing, tagname, tag, attrname, attrquote, attrvalue} state = nothing;
	std::string strtagname, strattrname, strattrvalue, strname, strlat, strlon;
	char charquote;
	int istackchange;
	int istack = 0;
	for (const char * end = data + size; data < end; ++data)
	{
		switch (state) 
		{
		case nothing:
			if (*data == '<') {
				strname = strlat = strlon = strtagname = "";
				istackchange = 1;
				state = tagname;
			}
			break;
		case tagname:
			if (isalpha(*data) || *data == '_')
				strtagname += *data;
			else if (*data == '>'){
				istack += istackchange;
				state = nothing;
			} else if (*data == '/') {
				if (strtagname.empty())
					istackchange = -1;
				else {
					istackchange = 0;
					state = tag;
				}
			} else
				state = tag;
			break;
		case tag:
			if (isalpha(*data) || *data == '_') {
				state = attrname;
				strattrname = "";
				strattrname += *data;
			} else if (*data == '>') {
				istack += istackchange;
				state = nothing;
			}
			else if (*data == '/')
				istackchange = 0;
			break;
		case attrname:
			if (isalpha(*data) || *data == '_')
				strattrname += *data;
			else if (*data == '=')
				state = attrquote;
			else
				state = nothing;
			break;
		case attrquote:
			if (*data == '"' || *data == '\'') {
				strattrvalue = "";
				state = attrvalue;
				charquote = *data;
			}
			else
				state = nothing;
			break;
		case attrvalue:
			if (*data == charquote) {
				if (strattrname == "name")
					strname = strattrvalue;
				else if (strattrname == "lat")
					strlat = strattrvalue;
				else if (strattrname == "lon")
					strlon = strattrvalue;
				if (strtagname == "named" && strname != "" && strlat != "" && strlon != "")
				{
					if (istack == 1)
					AddSearchResult(strname, strlat, strlon);
					strname = "";
				}
				state = tag;
			}
			else
				strattrvalue += *data;
			break;
		default:
			state = nothing;


		}
	}
}

const CVersionNumber& CMapApp::GetGpsVPVersion()
{
	return g_gpsVPVersion;
}

void CMapApp::GoToDemoPoint()
{
	double scale;
    GeoPoint demoPoint = m_pRasterMapPainter->GetDemoPoint(enumGMapType(m_riGMapType()), scale);
	m_painter.SetView(demoPoint, true);
	m_painter.SetXScale(scale);
}

void CMapApp::SetStartCursorOnNearestTrack(const GeoPoint & gp)
{
	int iIndex;
	CTrack& track = m_Tracks.GetNearestTrack(gp, iIndex);
	track.SetStartCursor(iIndex);
	m_painter.Redraw();
}

void CMapApp::SetEndCursorOnNearestTrack(const GeoPoint & gp)
{
	int iIndex;
	CTrack& track = m_Tracks.GetNearestTrack(gp, iIndex);
	track.SetEndCursor(iIndex);
	m_painter.Redraw();
}

void CMapApp::StartEditingRoute()
{
	m_Tracks.GetCurRoute().SetInsertMode(rimNearestSegment);
	m_painter.SetMoveCrossMode(true);
	CheckMenu();
}

void CMapApp::StopEditingRoute()
{
	m_Tracks.GetCurRoute().SetInsertMode(rimNone);
	m_painter.SetMoveCrossMode(false);
	SaveCurrentRoute();
	CheckMenu();
}

void CMapApp::SaveCurrentRoute()
{
	if (!m_rsRasterMapFolder().empty())
		m_Tracks.SaveRoute(m_rsRasterMapFolder()+L"\\route.plt", false, false);
}

void CMapApp::LoadCurrentRoute()
{
	// Try to open the file, if it doesn't exists it will simply open nothing...
	Int indexTrack = m_Tracks.OpenTracks(m_rsRasterMapFolder()+L"\\route.plt");
	if (-1 != indexTrack)
	{
		m_Tracks.NewRouteFromTrackIndex(indexTrack, false);
		m_Tracks.CloseTrack(indexTrack);
	}
}

bool CMapApp::FileOpenRoute()
{
	wchar_t strFile[MAX_PATH + 1] = {0};
	OPENFILENAME of;
	FillOpenFileName(&of, m_hWnd, L("Track files\0*.plt;*.gpx\0"), strFile, false, true);
	if (GetOpenFileName(&of))
	{
		StopEditingRoute();
		Int indexTrack = m_Tracks.OpenTracks(strFile);
		if (-1 != indexTrack)
		{
			bool bNewRouteCreated = m_Tracks.NewRouteFromTrackIndex(indexTrack);
			m_Tracks.CloseTrack(indexTrack);
			if (bNewRouteCreated)
			{
				SaveCurrentRoute(); // Save it as route.plt
				m_painter.SetView(m_Tracks.GetCurRoute().AsTrack().GetLastPoint(), true);
				ToolsNavigateRoute();
			}
			return bNewRouteCreated;
		}
		else
			return false;
	}
	else
	{
		return false;
	}
}

bool CMapApp::FileSaveRoute()
{
	// Current extension:
	std::wstring wstrExt = L".plt";
	if (1 == m_riTrackFormat())
		wstrExt = L".gpx";
	// Generating a default name:
	wchar_t strFile[MAX_PATH + 1] = {0};
	SYSTEMTIME st;
	GetLocalTime(&st);
	wsprintf(strFile, L"Route-%04d-%02d-%02d-%02d-%02d%s", st.wYear, st.wMonth, st.wDay,
		     st.wHour, st.wMinute, wstrExt.c_str());
	// Asking for confirmation of the name:
	OPENFILENAME of;
	if (1 == m_riTrackFormat())
		FillOpenFileName(&of, m_hWnd, L("Track files\0*.gpx\0"), strFile, false, false, true);
	else
		FillOpenFileName(&of, m_hWnd, L("Track files\0*.plt\0"), strFile, false, false, true);
	if (GetSaveFileName(&of))
	{
		StopEditingRoute();
		int iLen = wcslen(strFile);
		if (iLen >= 4 && !!_wcsicmp(strFile + iLen - wstrExt.length(), wstrExt.c_str()))
			wcscpy(strFile + iLen, wstrExt.c_str());
		m_Tracks.SaveRoute(strFile, true); // save to disk
		m_painter.Redraw();
		return true;
	}
	else
	{
		return false;
	}
}

void CMapApp::Navigate(const GeoPoint & gp, const wchar_t * wcName)
{
	m_riNavigate.Set(nmPoint);
	m_gpNavigate.Set(gp);
	m_painter.Redraw();
	m_MRUPoints.AddPoint(gp, wcName);
	CheckMenu();
}

void CMapApp::ToolsNavigateRoute()
{
	m_riNavigate.Set(nmRoute);
	m_painter.Redraw();
	CheckMenu();
}

void CMapApp::ToolsStopNavigating()
{
	m_riNavigate.Set(nmOff);
	m_painter.Redraw();
	CheckMenu();
}

void CMapApp::ToolsStopNavigateRoute()
{
	if (nmRoute == m_riNavigate())
		ToolsStopNavigating();
}

void CMapApp::CenterRouteTarget()
{
	if (!m_Tracks.GetCurRoute().IsEmpty())
		m_painter.SetView(m_Tracks.GetCurRoute().AsTrack().GetLastPoint(), true);
}
