/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef GDIPAINTER_H
#define GDIPAINTER_H

#include "screenpoint.h"
#include <map>
#ifdef LINUX
	#include <ext/hash_map>
	#define stdext __gnu_cxx
#else
	#include <hash_map>
#endif
#include <set>
#include <vector>
#include <list>
#include "IPainter.h" // for IPainter
#include "PlatformDef.h"
#include "RegValues.h"
#include "GeoPoint.h"
#include "FontCache.h"
#include "vpGDI.h"
#include "ScreenToGeo.h"
#ifdef UNDER_CE
#	include <aygshell.h>
#ifndef NO_COMMAND_BAR
extern HWND g_hwndCB;
#endif
#endif

template<class T, int n> class myvector
{
private:
	T m_Value[n];
	int m_nSize;
public:
	myvector() {m_nSize = 0;}
	void resize(int nSize) {m_nSize = nSize;}
	T & operator [](int nIndex) {return m_Value[nIndex];}
	int size() {return m_nSize;}
	T & front() {return m_Value[0];}
	T & back() {return m_Value[m_nSize - 1];}
	void push_back(T & value) {if (m_nSize < n) m_Value[m_nSize++] = value;}
};

//! Painter, which uses GDI to paint
class CGDIPainter : public IPainter, public IMonitorPainter, public IButtonPainter, public CScreenToGeo
{
	//! List of points for painting poly- objec
	myvector<ScreenPoint, 10000> m_pointList;
	//! Type of object
	unsigned int m_uiType;
	//! Limiting rectangle for current poly-object
	ScreenRect m_curRect;
	//! Are we painting polygon {or polyline)
	bool m_fPolygon;
	//! Default pen
	HPEN m_hDefaultPen;
	//! Default brush
	HBRUSH m_hDefaultBrush;
	//! Cursor pen
	HPEN m_hCursorPen;
	//! Cursor brush
	HBRUSH m_hCursorBrush;
	//! Polyline tools
	struct PolylineTools
	{
		HPEN first;
		HPEN second;
	};
	//! Map of polyline tools type
	typedef stdext::hash_map<UInt, PolylineTools> PolylineToolMap;
	//! Polygon tools
	struct PolygonTools
	{ 
		HPEN m_hPen;
		HBRUSH m_hBrush;
	};
	//! Map of polygon tools type
	typedef stdext::hash_map<UInt, PolygonTools> PolygonToolMap;
	//! Point tools
	struct PointTools
	{
		HICON m_hIcon;
		HBITMAP m_hBmp;
		int m_iDiffX;
		int m_iDiffY;

		PointTools() : m_hIcon(0), m_hBmp(0), m_iDiffX(0), m_iDiffY(0) {}
	};
	//! Map of point tools type
	typedef stdext::hash_map<UInt, PointTools> PointToolMap;
	//! Map of polygon tools
	PolygonToolMap m_PolygonTools;
	//! Map of point tools
	PointToolMap m_PointTools;
	//! Map of polyline tools
	PolylineToolMap m_PolylinePens;
	//! Device context handle for painting
	VP::DC m_hdc;

	const tchar_t * m_wcName;
	SIZE m_LabelSize;
	Int m_iWriteSegment;
	bool m_fLabelSizeKnown;
	Int m_iWriteSegmentD2;
	HWND m_hWnd;
	Int m_iOffTypePrev;
	Int m_iOffTypePrevPrev;
	HBRUSH m_hBgBrush;
	COLORREF m_crBg;
	COLORREF m_crText;
	bool m_fMandatoryLabel;
	ScreenRectSet m_srsLabels;
	ScreenRectSet m_srsPoints;
	bool m_fBottomBar;
	int m_iBottomBar;
	RECT m_rectLastWinSize;
	RECT m_rectLastCBSize;
	ScreenRect m_srCurrentMonitor;
	

	HINSTANCE m_hResourceInst;
	std::set<int> m_setUnknownTypes;
	ScreenRect m_srActiveMonitor;
	std::map<int, HICON> m_mapIcons;
	int m_iCurrentStatusIcon;
	CGDIPainter & operator = (const CGDIPainter &);
	bool m_fFullScreen;
	CFontCache m_FontCache;
	int m_iCurrentButton;
	typedef std::list<std::pair<ScreenRect, int> > Buttons;
	Buttons m_buttons;
	bool m_fShowUnknownTypes;
	bool m_fShowPolygonLabels;
	bool m_fShowAreaAsOutline;
	int m_iStatusLineOffset;
public:
	virtual ~CGDIPainter();
	void Init(HWND hWnd, HKEY hRegKey);
	// IPainter
	virtual void StartPolygon(UInt uiType, const tchar_t * wcName);
	virtual void StartPolyline(UInt uiType, const tchar_t * wcName);
	virtual void FinishObject();
	virtual void AddPoint(const GeoPoint & pt);
	virtual bool WillPaint(const GeoRect & rect);
	virtual void PaintPoint(UInt uiType, const GeoPoint & gpPoint, const tchar_t * wcName);
	virtual void SetLabelMandatory();
	virtual GeoRect GetRect();

	
	//! Init before painting
	void BeginPaint(HWND hWnd, VP::DC hdc, RECT srRegion, int iDegree360, bool fLowCenter);
	void BeginPaintLite(VP::DC hdc);
	void EndPaint();
	//! Initialize tools
	void InitTools(const tchar_t * strFilename);
	//! Check if rectangle intersects window
	bool WillPaint(const ScreenRect & rect);
	bool WillPaint(const ScreenPoint & pt);
	Int WillPaintEx(const ScreenPoint & pt);
	bool WillPaintLine(const GeoPoint & pt1, const GeoPoint & pt2);
	void CalculateLabelSize();
	void Redraw();
	void RedrawMonitors();
	void PaintScale();
	void PaintStatusLine(const tchar_t * wcName);
	void PaintLowMemory(const tchar_t * wcString1, const tchar_t * wcString2);
	virtual void DrawTextMonitor(const tchar_t * wcLabel, const tchar_t * wcText);
	virtual void DrawMonitorLabel(const tchar_t * wcLabel);
	virtual ScreenPoint GetMonitorSize();
	virtual void DrawBar(const ScreenRect & srBar);
	void SetShowMonitors(bool fShow)
	{
		bool fRedraw = (fShow != m_fBottomBar);
		m_fBottomBar = fShow;
		if (fRedraw)
			Redraw();
	}
	void SetFullScreen(bool fFull);
	virtual ScreenRect GetMonitorsBar();
	virtual void SetCurrentMonitor(const ScreenRect & srRect, bool fActive);
	void GetUnknownTypes(IListAcceptor * pAcceptor);
	ScreenPoint GetScreenCenter(){return m_spWindowCenter;}
	ScreenPoint GetActiveMonitorCenter(){return m_srActiveMonitor.Center();}
	void PaintStatusIcon(int iIcon);
	void ParseString(const char * buff, const std::tstring & wstrBase);
	void InitToolsCommon();
	void InitTools(int iScheme);
	bool IsFullScreen(){return m_fFullScreen;}
	virtual void AddButton(const tchar_t * wcLabel, int iCommand, bool fSelected);
	void ClearButtons();
	int CheckButton(const ScreenPoint & sp);
	CFontCache & GetFontCache() {return m_FontCache;}
	void PaintCompass();
	void SetShowUnknownTypes(bool f) {m_fShowUnknownTypes = f;}
	void SetShowPolygonLabels(bool f) {m_fShowPolygonLabels = f;}
	void SetShowAreaAsOutline(bool f) {m_fShowAreaAsOutline = f;}
	void SetXScale(double scale);
	void PrepareScales();
};

#endif // GDIPAINTER_H
