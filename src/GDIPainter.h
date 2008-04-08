#ifndef GDIPAINTER_H
#define GDIPAINTER_H

#include "ScreenPoint.h"
#include <map>
#include <hash_map>
#include <set>
#include <vector>
#include <list>
#include "IPainter.h" // for IPainter
#include <windows.h>
#include "RegValues.h"
#include "GeoPoint.h"
#include "FontCache.h"
#include "vpGDI.h"
#ifdef UNDER_CE
#	include <aygshell.h>
#ifndef NO_COMMAND_BAR
extern HWND g_hwndCB;
#endif
#endif

struct IMonitorPainter
{
	virtual void DrawTextMonitor(const wchar_t * wcLabel, const wchar_t * wcText) = 0;
	virtual void DrawMonitorLabel(const wchar_t * wcLabel) = 0;
	virtual ScreenPoint GetMonitorSize() = 0;
	virtual void DrawBar(const ScreenRect & srBar) = 0;
	virtual void SetCurrentMonitor(const ScreenRect & srRect, bool fActive) = 0;
};

struct IMonitor
{
	virtual void Paint(IMonitorPainter * pPainter) = 0;
	virtual const wchar_t * GetId() = 0;
	virtual void PrepareContextMenu(IListAcceptor * pMenu) = 0;
	virtual void ProcessMenuCommand(int i) = 0;
};

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
class CGDIPainter : public IPainter, public IMonitorPainter, public IButtonPainter
{
	//! List of points for painting poly- objec
	myvector<ScreenPoint, 10000> m_pointList;
	//! Type of object
	unsigned int m_uiType;
	//! Limiting rectangle for current poly-object
	ScreenRect m_curRect;
	//! View center
	CRegScalar<GeoPoint, REG_BINARY> m_gpCenter;
	GeoPoint m_gpCenterCache;
	GeoPoint m_gpCenterView;
	int m_sin100;
	int m_cos100;
	bool m_fViewSet;
	//! Scale in garmin points per screen point
	CRegScalar<int, REG_DWORD> m_ruiScale10;
	int m_uiScale10Cache;
	//! Scale of x axis for the latitude
	long m_lXScale100;
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
	//! coordinate of window center
	ScreenPoint m_spWindowCenter;
	//! Window rect
	ScreenRect m_srWindow;
	//! Device context handle for painting
	VP::DC m_hdc;

	enum { 
		ciMinZoom = 1,		//!< Minimum zoom
		ciMaxZoom = 100000	//!< Maximum zoom
	};
	const wchar_t * m_wcName;
	SIZE m_LabelSize;
	Int m_iWriteSegment;
	bool m_fLabelSizeKnown;
	Int m_iWriteSegmentD2;
	HWND m_hWnd;
	Int m_iOffTypePrev;
	Int m_iOffTypePrevPrev;
	HBRUSH m_hBgBrush;
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
	set<int> m_setUnknownTypes;
	ScreenRect m_srActiveMonitor;
	map<int, HICON> m_mapIcons;
	int m_iCurrentStatusIcon;
	CGDIPainter & operator = (const CGDIPainter &);
	bool m_fFullScreen;
	bool m_fVertical;
	CFontCache m_FontCache;
	int m_iManualTimer;
	int m_iCurrentButton;
	typedef std::list<std::pair<ScreenRect, int> > Buttons;
	Buttons m_buttons;
	bool m_fShowUnknownTypes;
	bool m_fShowPolygonLabels;
	int m_iStatusLineOffset;
public:
	virtual ~CGDIPainter();
	void Init(HWND hWnd, HKEY hRegKey);
	// IPainter
	virtual void StartPolygon(UInt uiType, const wchar_t * wcName);
	virtual void StartPolyline(UInt uiType, const wchar_t * wcName);
	virtual void FinishObject();
	virtual void AddPoint(const GeoPoint & pt);
	virtual bool WillPaint(const GeoRect & rect);
	virtual void SetView(const GeoPoint & gpCenter, bool fManual);
	virtual void PaintPoint(UInt uiType, const GeoPoint & gpPoint, const wchar_t * wcName);
	virtual void SetLabelMandatory();
	virtual GeoRect GetRect();

	
	void AddPoint(ScreenPoint pt);
	//! Init before painting
	void BeginPaint(HWND hWnd, VP::DC hdc, RECT srRegion, int iDegree360, bool fLowCenter);
	void BeginPaintLite(VP::DC hdc);
	void EndPaint();
	//! Initialize tools
	void InitTools(const wchar_t * strFilename);
	//! Zoom view in
	void ZoomIn();
	//! Zoom view out
	void ZoomOut();
	//! Move view center left
	void Left();
	//! Move view center right
	void Right();
	//! Move view center up
	void Up();
	//! Move view center down
	void Down();
	//! Move view center by random vector
	void Move(ScreenDiff d);
	//! Get-method for m_uiScale
	int GetScale() {return m_ruiScale10();}
	//! Check if rectangle intersects window
	bool WillPaint(const ScreenRect & rect);
	bool WillPaint(const ScreenPoint & pt);
	Int WillPaintEx(const ScreenPoint & pt);
	bool WillPaintLine(const GeoPoint & pt1, const GeoPoint & pt2);
	void CalculateLabelSize();
	void Redraw();
	void RedrawMonitors();
	GeoPoint ScreenToGeo(const ScreenPoint & pt);
	ScreenPoint GeoToScreen(const GeoPoint & pt);
	ScreenRect GeoToScreen(const GeoRect & rect);
	GeoRect ScreenToGeo(const ScreenRect & rect);
	const GeoPoint GetCenter();
	void PaintScale();

	void PaintStatusLine(const wchar_t * wcName);
	void PaintLowMemory(const wchar_t * wcString1, const wchar_t * wcString2);
	virtual void DrawTextMonitor(const wchar_t * wcLabel, const wchar_t * wcText);
	virtual void DrawMonitorLabel(const wchar_t * wcLabel);
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
	ScreenRect GetScreenRect(){return m_srWindow;}
	ScreenPoint GetScreenCenter(){return m_spWindowCenter;}
	ScreenPoint GetActiveMonitorCenter(){return m_srActiveMonitor.Center();}
	void PaintStatusIcon(int iIcon);
	void ParseString(const char * buff, const std::wstring & wstrBase);
	void InitToolsCommon();
	void InitTools(int iScheme);
	bool IsFullScreen(){return m_fFullScreen;}
	bool IsVertical(){return m_fVertical;}
	void OnTimer() {AutoLock l; if (m_iManualTimer > 0) --m_iManualTimer;}
	bool ManualMode() {AutoLock l; return m_iManualTimer > 0;}
	void ResetManualMode() {AutoLock l; m_iManualTimer = 0;}
	virtual void AddButton(const wchar_t * wcLabel, int iCommand, bool fSelected);
	void ClearButtons();
	int CheckButton(const ScreenPoint & sp);
	CFontCache & GetFontCache() {return m_FontCache;}
	void PaintCompass();
	void SetShowUnknownTypes(bool f) {m_fShowUnknownTypes = f;}
	void SetShowPolygonLabels(bool f) {m_fShowPolygonLabels = f;}
	double GetXScale();
	void SetXScale(double scale);
	void PrepareScales();
};

#endif // GDIPAINTER_H