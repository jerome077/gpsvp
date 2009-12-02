#ifndef gtkpainter_h
#define gtkpainter_h

#include <iostream>
#include <gtkmm.h>
#include <hildonmm.h>
#include <map>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>

#include "Common.h"
#include "Header.h"
#include "IPainter.h"
#include "PlatformDef.h"
#include "Atlas.h"
#include "NMEAParser.h"
#include "GeoPoint.h"
#include "RegValues.h"
#include "Monitors.h"
#include "IPainter.h"
#include "ScreenToGeo.h"

typedef void* HWND;

class CGTKPainter
	: public IMonitorPainter
	, public IButtonPainter
	, public CScreenToGeo
	, public Hildon::Window
	, public IGPSClient
	, public IStatusPainter
{
private:
	CAtlas a;
public:
	void Init(HKEY, HWND);
	void RedrawMonitors();
	void SetShowMonitors(bool fShow);
	bool IsFullScreen();
	void SetFullScreen(bool fFull);
	void BeginPaint();
	void EndPaint();
	void PaintScale();
	void PaintStatusLine(const tchar_t * wcName);
	void PaintLowMemory(const tchar_t * wcString1, const tchar_t * wcString2);
	void PaintStatusIcon(int iIcon);;
	void PaintCompass();;
	ScreenRect GetMonitorsBar();
	void ClearButtons();;

	//IMonitorPainter
	void DrawTextMonitor(const tchar_t * wcLabel, const tchar_t * wcText);
	void DrawMonitorLabel(const tchar_t * wcLabel);
	ScreenPoint GetMonitorSize();
	void DrawBar(const ScreenRect & srBar);
	void SetCurrentMonitor(const ScreenRect & srRect, bool fActive);

	// IButtonPainter
	void AddButton(const tchar_t * wcLabel, int iCommand, bool fSelected);


	CGTKPainter();
	~CGTKPainter();
	void AddMap(const tchar_t * name);
	bool polygon;
	UInt type;
	const tchar_t * name;
	std::vector<ScreenPoint> m_pointList;
	virtual void StartPolygon(UInt uiType, const tchar_t * wcName);
	virtual void StartPolyline(UInt uiType, const tchar_t * wcName);
	Cairo::TextExtents m_LabelSize;
	void CalculateLabelSize();
	virtual void FinishObject();
	void AddPoint(const ScreenPoint & p);
	virtual void AddPoint(const GeoPoint & gp);
	virtual bool WillPaint(const GeoRect & rect);
	virtual void PaintPoint(UInt uiType, const GeoPoint & gp, const tchar_t * wcName);
	virtual void SetLabelMandatory();
	virtual GeoRect GetRect();
	
	int iDegree360;
	GeoPoint m_gpCenter;
	int m_ruiScale10;

	Cairo::RefPtr<Cairo::Context> cr;
	Glib::RefPtr<Gdk::Pixmap> pixmap;
	bool started;
	int m_cos100;
	int m_sin100;
	ScreenPoint m_spWindowCenter;
	ScreenRect m_srWindow;
	ScreenRectSet m_srsPoints;

	virtual bool on_expose_event(GdkEventExpose* event);
	struct RGB
	{
		RGB() : r(0), g(0), b(0) {};
		RGB(int r_, int g_, int b_) : r(r_), g(g_), b(b_) {};
		int r,g,b;
		void Set(Cairo::RefPtr<Cairo::Context> &cr)
		{
			cr->set_source_rgb(double(r)/255, double(g)/255, double(b)/255);
		}
	};
	struct CPen
	{
		CPen() : style(-1), width(0), color(RGB(0,0,0)) {};
		CPen(int style_, int width_, RGB color_) : style(style_), width(width_), color(color_) {};
		operator bool() const {return style != -1;}
		void Set(Cairo::RefPtr<Cairo::Context> &cr)
		{
			if (style)
			{
				static std::vector<double> dashes;
				dashes.resize(1);
				dashes[0] = 3;
				cr->set_dash(dashes, 0);
			}
			else
				cr->unset_dash();
			cr->set_line_width(width);
			color.Set(cr);
		}
		int style;
		int width;
		RGB color;
	};
	
	struct CBrush
	{
		CBrush() : color(RGB(0,0,0)) {};
		CBrush(RGB color_) : color(color_) {};
		RGB color;
		void Set(Cairo::RefPtr<Cairo::Context> &cr)
		{
			color.Set(cr);
		}
	};
	CPen m_hDefaultPen;
	CBrush m_hDefaultBrush;
	
	struct PolygonTools
	{ 
		CPen m_hPen;
		CBrush m_hBrush;
	};
	typedef std::map<UInt, PolygonTools> PolygonToolMap;
	PolygonToolMap m_PolygonTools;

	struct PointTools
	{
		// HICON m_hIcon;
		// HBITMAP m_hBmp;
		Cairo::RefPtr<Cairo::ImageSurface> s;
		int m_iDiffX;
		int m_iDiffY;

		PointTools() : m_iDiffX(0), m_iDiffY(0) {};
	};
	typedef std::map<UInt, PointTools> PointToolMap;
	PointToolMap m_PointTools;
	
	struct PolylineTools
	{
		CPen first;
		CPen second;
	};
	typedef std::map<UInt, PolylineTools> PolylineToolMap;
	PolylineToolMap m_PolylinePens;
	
	RGB m_crBg;
	CBrush m_hBgBrush;
	RGB m_crText;
	CPen m_hCursorPen;
	CBrush m_hCursorBrush;
	
	void InitToolsCommon();
	void InitTools(const tchar_t * strFilename);
	void ParseString(const char * buff, const std::fnstring & wstrBase);
	bool on_scroll(GdkEventScroll * event);
	int orig_x, orig_y;
	bool pressed;
	bool on_press(GdkEventButton* event);
	bool on_release(GdkEventButton* event);
	bool on_motion(GdkEventMotion* event);
	void Move(ScreenDiff d);
	
	enum { 
		ciMinZoom = 1,		//!< Minimum zoom
		ciMaxZoom = 100000	//!< Maximum zoom
	};
	void Redraw();
	
	virtual void PaintText(const tchar_t * wcText);
	virtual void SetProgressItems(int iLevel, int iCount);
	virtual void SetProgress(int iLevel, int iProgress);
	virtual void Advance(int iLevel);

	int m_hPortFile;
	CNMEAParser m_NMEAParser;
	bool m_fExiting;
	std::string m_rsPort;

	void NoFix();
	void Fix(GeoPoint gp, double, double dHDOP);
	void NoVFix();
	void VFix(double dAltitude, double);
	void SetConnectionStatus(enumConnectionStatus iStatus);
};

#endif // gtkpainter_h
