/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef MAPAPP_H
#define MAPAPP_H

#define DONT_DEFINE_MIN
#include "Common.h"
#include "Header.h"
#include "GDIPainter.h"
#include "FileDialogs.h"
#include "Track.h"
#include "NMEAParser.h"
#include "WayPoints.h"
#include "Atlas.h"
#include "MonitorSet.h"
#include "Keymap.h"
#include "MRUPoints.h"
#ifdef UNDER_CE
#	include <pm.h>
#endif UNDER_CE
#include "Commands.h"
#include "TypeInfo.h"
#include "Lock.h"
#include "ScreenButtons.h"
#include "OptionSet.h"
#include "Traffic.h"
#include "TrackCompetition.h"
#include "Sun.h"
#include "Team.h"

struct ObjectInfo
{
	wstring wstrName;
	GeoPoint gp;
	UInt uiType;
	bool fPresent;
	ObjectInfo() : fPresent(false) {}
	wstring GetDescription()
	{
		if (wstrName == L"")
			return L("(No label)");
		return wstrName;
	}
};

class CGMPainter;

class CMapApp : public IGPSClient
{
private:
	Dict m_dict;
public:
	CGDIPainter m_painter;
	CRegString m_rsWaypointsFile;
	CRegString m_rsToolsFile;
	CRegString m_rsTranslationFile;
	CRegScalar<int, REG_BINARY> m_riScheme;
	CRegString m_rsTrackFolder;
	CRegString m_rsRasterMapFolder;
	CAtlas m_atlas;
	CGMPainter *m_pRasterMapPainter;
	ScreenPoint m_spFrom;
	bool m_fMoving;
	int m_iPressedButton;
	HWND m_hWnd;
	wstring m_wstrHome;
	list<CTrack> m_Tracks;
	CTrack m_CurTrack;
	HANDLE m_hPortFile;
	CRegString m_rsPort;
	CRegString m_rsPortSpeed;
	CRegString m_rsCurrentFolder;
	CRegString m_rsProxy;
	HANDLE m_hPortThread;
	HANDLE m_hHttpThread;
	CNMEAParser m_NMEAParser;
	CWaypoints m_Waypoints;
	CWaypoints m_Found;
	COptionSet m_Options;
	CRegScalar<int, REG_BINARY> m_riTrackStep;
	CRegScalar<int, REG_BINARY> m_riCoordFormat;
	CRegScalar<int, REG_BINARY> m_riMetrics;
	CRegScalar<int, REG_BINARY> m_riWaypointsRadius;
	CRegScalar<int, REG_BINARY> m_riDetail;
	CRegScalar<int, REG_BINARY> m_riConnectPeriodMin;
	CRegScalar<int, REG_BINARY> m_riGMapType;
	bool volatile m_fExiting;
	bool volatile m_fStopHttpThread;
	CDistanceMonitor m_monDistance;
	CAzimuthMonitor m_monAzimuth;
	CAzimuthMonitor m_monCourse;
	CHeightMonitor m_monAltitude;
	CDegreeMonitorLon m_monLongitude;
	CDegreeMonitorLat m_monLatitude;
	CDistanceMonitor m_monOdometerTotal;
	CDistanceMonitor m_monOdometer1;
	CDistanceMonitor m_monOdometer2;
	// CDistanceMonitor m_monTrackDistance;
	CMemoryMonitor m_monMemory;
	CMemoryMonitor m_monDataTotal;
	CMemoryMonitor m_monDataIn;
	CMemoryMonitor m_monDataOut;
	CPercentMonitor m_monBattery;
	CPercentMonitor m_monCPU;
	CDoubleMonitor m_monHDOP;
	CDoubleMonitor m_monProfile[8];
	CDoubleMonitor m_monSleepCounter;
	CSatellitesMonitor m_monSatellites;
	CTextMonitor m_monInternet;
	CTextMonitor m_monDLRemaining;
	//! Cursor position
	GeoPoint m_gpCursor;
	bool m_fCursorVisible;
	CRegScalar<GeoPoint, REG_BINARY> m_gpNavigate;
	CRegScalar<bool, REG_BINARY> m_fNavigate;
	CMRUPoints m_MRUPoints;
	CMonitorSet m_MonitorSet;
	bool m_fFix;
	bool m_fCheckLatestVersion;
	bool m_fCoursePointPresent;
	GeoPoint m_gpCoursePoint;
	void FillOpenFileName(OPENFILENAME * of, HWND hwndOwner, wchar_t * wstrFilter, 
							   wchar_t * strFile, bool fDirectory, bool fMustExist);
	void AddOdometer(double dDist);
	wchar_t * m_wstrCmdLine;
	wstring m_wstrProgName;
	DWORD m_dwConnected;
	enumConnectionStatus m_iConnectionStatus;
	bool m_fActive;
	bool m_fNeedPaintOnActivate;
	CTypeInfo m_TypeInfo;
	bool m_fMemoryLow;
	bool m_fMemoryVeryLow;
	std::string m_request;
	std::string m_searchurl;
	TrackCompetition m_TrackCompetition;
	CSun m_Sun;
	CTeam m_team;

	// For CPU load monitor
	DWORD m_dwLastTickTimer;
#ifdef UNDER_CE
	typedef DWORD (_stdcall *GetIdleTimeProc) (void);
	HINSTANCE m_hCoreDll;
	GetIdleTimeProc m_pfnGetIdleTime;
	DWORD m_dwLastIdleTick;
#else // UNDER_CE
	__int64 m_nProcessorUsage;
#endif // UNDER_CE

#ifdef SMARTPHONE
	bool m_fBluetoothWasTurnedOn;
#endif // SMARTPHONE

	VP::Buffer m_screenBuffer;
	VP::Buffer m_alphaBuffer;

	CTrack m_ReplayTrack;
	std::wstring m_wstrReplayNMEA;
	TrafficNodes m_TrafficNodes;
	std::wstring m_wstrVersionMessage;

public:
	CMapApp();
	~CMapApp();
	virtual void NoFix();
	virtual void Fix(GeoPoint gp, double dHDOP);
	virtual void NoVFix();
	virtual void VFix(double dAltitude);
	void OnLButtonDown(ScreenPoint pt);
	void OnLButtonUp(ScreenPoint pt);
	int AddPointScreen(ScreenPoint pt, wchar_t * wcName);
	int AddPointCenter(wchar_t * wcName);
	void ViewZoomIn();
	void ViewZoomOut();
	void ViewUp();
	void ViewDown();
	void ViewLeft();
	void ViewRight();
	void FileExit();
	void FileOpenMap();
	void FileOpenMapFolder();
	void FileCloseAllMaps();
	void FileOpenTrack();
	void FileOpenWaypoints();
	void FileImportWaypoints();
	void FileSaveWaypoints();
	void FileNextColors();
	void FileOpenColors();
	void FileCloseColors();
	void FileOpenTranslation();
	void FileCloseTranslation();
	void ToolsTracks();
	void SearchOSM();
	void SearchResults();
	void SetGMapType(int type);
	void FileIndexDirectory();
	void OptionsSetTrackFolder();
	void SetRasterMapFolder();
	void DRMAddCurrentView();
	void DRMStartWithCurrentZoom();
	void DRMByTrack();
	void InitCoreDll();

	void Create(HWND hWnd, wchar_t * wcHome = L"./");
	void InitMenu();
	void InitMenuAllWMSMaps(CMenu& baseMenu);
	void SetWMSMapType(WPARAM wp);
	void Paint();
	void ThreadRoutine();
	void StartListening();
	void CloseTrack(Int iIndex);
	CWaypoints & GetWaypoints() {return m_Waypoints;}
	CKeymap & GetKeymap();
	CScreenButtons & GetButtons();
	void Navigate(ScreenPoint pt, const wchar_t * wcName);
	void Navigate(const GeoPoint & gp, const wchar_t * wcName)
	{
		m_gpNavigate.Set(gp);
		m_fNavigate.Set(true);
		m_painter.Redraw();
		m_MRUPoints.AddPoint(gp, wcName);
	};
	void SetMenu(HMENU hMenu);
	void SetMenu(HWND hWndMenu);
	void CheckMenu();
	void NextMonitorsRow();
	void PrevMonitorsRow();
	void UpdateMonitors();
	void PaintCursor(const GeoPoint & gp, bool fCursorVisible);
	void ToolsStopNavigating()
	{
		m_fNavigate.Set(false);
		m_painter.Redraw();
	}
	void ToolsWaypoints();
	void ToolsWaypointsEx();
	void ToolsMaps();
	bool ToolsWaypointProperties(int iPoint);
	void ToolsNavigateRecent()
	{
		m_MRUPoints.Navigate(ScreenPoint(20, 20), m_hWnd);
	}
	void OptionsSettings();
	void TeamSettings();
	void OptionsKeymap();
	void DebugNmeaCommands();
	void DebugUnknownPointTypes();
	void DebugCursorHere() {Fix(m_painter.GetCenter(), 50.0);}
	void DebugNoFix() {NoFix();}
	void DebugShowTime() {m_NMEAParser.DebugShowTime();}
	CAtlas & GetAtlas() {return m_atlas;}
	int GetTrackStep() {return m_riTrackStep();}
	bool ProcessCommand(WPARAM wp);
	void ProcessWMHIBERNATE();
	void SetDetail(int iDetail);
	int PrepareScale(unsigned int uiScale10)
	{
		switch (m_riDetail())
		{
		case -2: return uiScale10 * 4;
		case -1: return uiScale10 * 2;
		case 0: return uiScale10;
		case 1: return max(uiScale10 / 2, 1u);
		case 2: return max(uiScale10 / 4, 1u);
		}
		return uiScale10;
	}
	void GetTrackList(IListAcceptor * pAcceptor);
	void SetConnectPeriod(int nPeriod);
	void OnTimer();
	void ToggleConnect();
	void ToggleShowCenter();
	void ContextMenu();
	void NewTrack();
	void ContextMenu(ScreenPoint sp);
	void AdjustZoom();
	ObjectInfo FindNearestPoint(const ScreenPoint & sp, double dRadius);
	ObjectInfo FindNearestPolyline(const GeoPoint & gp, double dRadius);
	ObjectInfo FindPolygon(const GeoPoint & gp);
	double GetDistanceByTrack(const GeoPoint & pt1, const GeoPoint & pt2);
	void CheckTrackDistance();
	void ProcessCmdLine(const wchar_t * wcCmdLine);
	void ProcessCmdLineElement(const wchar_t * wcCmdLine);
	HANDLE m_hPwrReq;
	void CheckOptions();
	void SetConnectionStatus(enumConnectionStatus iStatus);
	void RegisterFileTypes();
	void About();
	void Exit();
	wstring FileDialog(wstring wstrMask);
	void SetActive(bool fActive)
	{
		m_fActive = fActive;
		CheckOptions();
		if (m_fActive && m_fNeedPaintOnActivate)
		{
			m_fNeedPaintOnActivate = false;
			m_painter.Redraw();
		}
	}
	wstring HeightFromFeet(const wchar_t * wcOriginal);
	wchar_t * GetTitle() {return L"gpsVP";}
	void ExportWaypoint(int id, HWND hWnd);
	Dict & GetDict() {return m_dict;}
	void ReplayTrack();
	void DumpNMEA();
	void ReplayNMEA();
	void StartHttpThread();
	void HttpThreadRoutine();
	std::wstring m_wstrHttpStatus;
	CRegScalar<bool, REG_BINARY> m_fTrafficAgreement;
	const char * GetServerName();
	void SetSearchURL(const char * url);
	void ProcessOSMSearchResult(const char * data, int size);
	void AddSearchResult(const std::string & name, const std::string & lat, const std::string & lon);
};

extern CMapApp app;

#endif // MAPAPP_H
