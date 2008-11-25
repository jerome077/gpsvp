#include "Common.h"
#include "Header.h"
#include "IPainter.h"
#include "PlatformDef.h"

#include <iostream>
#include <gtkmm.h>
#include <map>

Dict dict;
Dict & GetDict()
{
	return dict;
}

struct ScreenPoint
{
	ScreenPoint(int x_, int y_) : x(x_), y(y_) {}
	ScreenPoint() : x(0), y(0) {}
	int x, y;
};

struct ScreenRect
{
	int left, right, top, bottom;
	ScreenRect(){};
	// ScreenRect(const RECT & rc){ *(RECT*)this = rc;};
	/*ScreenRect(const ScreenPoint & sp, const ScreenSize & ss)
	{
		left = sp.x;
		top = sp.y;
		right = sp.x + ss.cx;
		bottom = sp.y + ss.cy;
	}*/
	void Init(const ScreenPoint & pt)
	{
		left = pt.x;
		right = pt.x;
		top = pt.y;
		bottom = pt.y;
	}
	void Append(const ScreenPoint & pt)
	{
		if (pt.x < left)
			left = pt.x;
		else if (pt.x > right)
			right = pt.x;
		if (pt.y < top)
			top = pt.y;
		else if (pt.y > bottom)
			bottom = pt.y;
	}
	bool Intersect(const ScreenRect & a)
	{
		if (a.top > bottom) return false;
		if (a.bottom < top) return false;
		if (a.left > right) return false;
		if (a.right < left) return false;
		return true;
	}
	bool IntersectHard(const ScreenRect & a)
	{
		if (a.top > (top + bottom) / 2) return false;
		if (a.bottom < top) return false;
		if (a.left > (right + left) / 2) return false;
		if (a.right < left) return false;
		return true;
	}
	int Side(const ScreenPoint & pt) const
	{
		if (pt.x > right)
			return 1;
		if (pt.x < left)
			return 2;
		if (pt.y > bottom)
			return 3;
		if (pt.y < top)
			return 4;
		return 0;
	}
	int Width()
	{
		return right - left;
	}
	int Height()
	{
		return bottom - top;
	}
	ScreenPoint Center()
	{
		return ScreenPoint((right + left) / 2, (top + bottom) / 2);
	}
	int mymax(int a, int b) { return (a>b)?a:b; }
	int mymin(int a, int b) { return (a<b)?a:b; }
	void Trim(const ScreenRect & r)
	{
		if (right > r.right)
			right = mymax(left, r.right);
		if (left < r.left)
			left = mymin(right, r.left);
		if (bottom > r.bottom)
			bottom = mymax(top, r.bottom);
		if (top < r.top)
			top = mymin(bottom, r.top);
	}
};


struct DumpPainter : public Gtk::DrawingArea, public IPainter
{
	CIMGFile & f;
	DumpPainter(CIMGFile & f_) : f(f_) {}
	bool polygon;
	UInt type;
	virtual void StartPolygon(UInt uiType, const wchar_t * wcName) 
	{ 
		// std::cerr << "StartPolygon" << std::endl;
		cr->begin_new_path();
		started = false;
		polygon = true;
		type = uiType;
	};
	virtual void StartPolyline(UInt uiType, const wchar_t * wcName) 
	{ 
		// std::cerr << "StartPolyline" << std::endl; 
		cr->begin_new_path();
		started = false;
		polygon = false;
		type = uiType;
	};
	virtual void FinishObject() 
	{ 
		// std::cerr << "FinishObject" << std::endl; 
		if (polygon)
		{
			if (type == 0x4b)
				return;
			PolygonToolMap::iterator it = m_PolygonTools.find(type);
			if (it != m_PolygonTools.end())
			{
				PolygonTools & t = it->second;
				t.m_hPen.Set(cr);
				t.m_hBrush.Set(cr);
			}
			else
			{
				m_hDefaultPen.Set(cr);
				m_hDefaultBrush.Set(cr);
			}
			cr->fill();
		}
		else 
		{
			PolylineToolMap::iterator it = m_PolylinePens.find(type);
			if (it != m_PolylinePens.end())
			{
				it->second.first.Set(cr);
				cr->stroke_preserve();
				if (it->second.second)
				{
					it->second.second.Set(cr);
					cr->stroke_preserve();
				}
			}
			else
			{
				m_hDefaultPen.Set(cr);
				cr->stroke();
			}
		}
	};
	virtual void AddPoint(const GeoPoint & gp) 
	{ 
		ScreenPoint p = GeoToScreen(gp);
		// std::cerr << "AddPoint (" << p.x << "," << p.y << ")" << std::endl;
		if (started)
			cr->line_to(p.x, p.y);
		else
			cr->move_to(p.x, p.y);
		started = true;
	};
	virtual bool WillPaint(const GeoRect & rect) { 
		ScreenRect sRect = GeoToScreen(rect);
		// std::cerr << sRect.top << ',' << sRect.left << ',' << sRect.bottom << ',' << sRect.right << std::endl;
		GeoRect grScreen = ScreenToGeo(m_srWindow);
		bool i1 = m_srWindow.Intersect(sRect);
		bool i2 = grScreen.Intersect(rect);
		if (i1 && i2)
		{
			// std::cerr << "WillPaint: true" << std::endl;
			return true;
		}
		// std::cerr << "WillPaint: false" << std::endl;
		return false;
	};
	virtual void SetView(const GeoPoint & gp, bool fManual) { std::cerr << "SetView" << std::endl; };
	virtual void PaintPoint(UInt uiType, const GeoPoint & gp, const wchar_t * wcName) 
	{ 
		// std::cerr << "PaintPoint" << std::endl; 
	}
	virtual void SetLabelMandatory() { std::cerr << "SetLabelMandatory" << std::endl; };
	virtual GeoRect GetRect() { std::cerr << "GetRect" << std::endl;};
	
	int iDegree360;
	GeoPoint m_gpCenter;
	int m_ruiScale10;

	Cairo::RefPtr<Cairo::Context> cr;
	bool started;
	int m_cos100;
	int m_sin100;
	ScreenPoint m_spWindowCenter;
	ScreenRect m_srWindow;

	virtual bool on_expose_event(GdkEventExpose* event)
	{
		PrepareScales();
		m_cos100 = int(cos(double(iDegree360) / 180 * pi) * 100);
		m_sin100 = int(sin(double(iDegree360) / 180 * pi) * 100);
		
		Glib::RefPtr<Gdk::Window> window = get_window();
		Gtk::Allocation allocation = get_allocation();
    	const int width = allocation.get_width();
    	const int height = allocation.get_height();
		m_spWindowCenter = ScreenPoint(width / 2, height / 2);
		m_srWindow.Init(ScreenPoint(0, 0));
		m_srWindow.Append(ScreenPoint(width, height));

    	cr = window->create_cairo_context();
    	cr->set_line_width(3.0);
    	cr->set_source_rgb(0.8, 0.0, 0.0);
    	
		f.Paint(this, 20, maskPolygons, true);
		f.Paint(this, 20, maskPolylines, true);
		cr.clear();
		return true;
	}
	GeoPoint m_gpCenterCache;
	long m_lXScale100;
	int m_uiScale10Cache;
	void PrepareScales()
	{
		m_gpCenterCache = m_gpCenter;
		m_uiScale10Cache = m_ruiScale10;
		m_lXScale100 = cos100(m_gpCenterCache.lat);
	}
	GeoRect ScreenToGeo(const ScreenRect & rect)
	{
		GeoRect res;
		GeoPoint gp1 = ScreenToGeo(ScreenPoint(rect.left, rect.top));
		GeoPoint gp2 = ScreenToGeo(ScreenPoint(rect.left, rect.bottom));
		GeoPoint gp3 = ScreenToGeo(ScreenPoint(rect.right, rect.top));
		GeoPoint gp4 = ScreenToGeo(ScreenPoint(rect.right, rect.bottom));
		res.minLat = min(min(gp1.lat, gp2.lat), min(gp3.lat, gp4.lat));
		res.minLon = min(min(gp1.lon, gp2.lon), min(gp3.lon, gp4.lon));
		res.maxLat = max(max(gp1.lat, gp2.lat), max(gp3.lat, gp4.lat));
		res.maxLon = max(max(gp1.lon, gp2.lon), max(gp3.lon, gp4.lon));
		return res;
	}
	GeoPoint ScreenToGeo(const ScreenPoint & pt)
	{
		GeoPoint res;
		int dx1 = (pt.x - m_spWindowCenter.x);
		int dy1 = (pt.y - m_spWindowCenter.y);

		int dx2;
		int dy2;
		if (m_cos100 != 100 || m_sin100 != 0)
		{
			dx2 = (dx1 * m_cos100 - dy1 * m_sin100) / 100;
			dy2 = (dx1 * m_sin100 + dy1 * m_cos100) / 100;
		}
		else
		{
			dx2 = dx1;
			dy2 = dy1;
		}

		res.lon = dx2 * m_ruiScale10 / 10 * 100 / m_lXScale100 + m_gpCenter.lon;
		res.lat = - dy2 * m_ruiScale10 / 10 + m_gpCenter.lat;
		return res;
	}
	ScreenRect GeoToScreen(const GeoRect & rect)
	{
		ScreenRect res;
		res.Init(GeoToScreen(GeoPoint(rect.minLon, rect.minLat)));
		res.Append(GeoToScreen(GeoPoint(rect.maxLon, rect.maxLat)));
		res.Append(GeoToScreen(GeoPoint(rect.minLon, rect.maxLat)));
		res.Append(GeoToScreen(GeoPoint(rect.maxLon, rect.minLat)));
		return res;
	}
	ScreenPoint GeoToScreen(const GeoPoint & pt)
	{
		ScreenPoint res;
		int dx1 = (pt.lon - m_gpCenterCache.lon) * m_lXScale100 / 10 /* * 10 / 100 */ / m_uiScale10Cache;
		int dy1 = (m_gpCenterCache.lat - pt.lat) * 10 / m_uiScale10Cache;
		
		int dx2;
		int dy2;
		if (m_cos100 != 100 || m_sin100 != 0)
		{
			dx2 = (dx1 * m_cos100 + dy1 * m_sin100) / 100;
			dy2 = (- dx1 * m_sin100 + dy1 * m_cos100) / 100;
		}
		else
		{
			dx2 = dx1;
			dy2 = dy1;
		}

		int dx = dx2 + m_spWindowCenter.x;
		int dy = dy2 + m_spWindowCenter.y;

		if (dx > 1000000) dx = 1000000;
		if (dx < -1000000) dx = -1000000;
		if (dy > 1000000) dy = 1000000;
		if (dy < -1000000) dy = -1000000;

		res.x = dx;
		res.y = dy;
		/*
		std::cerr << "GeoToScreen(" << pt.lon << ", " << pt.lat << ") = (" << res.x << ", " << res.y << ")" << std::endl;
		std::cerr << ""<<dy1<<" = ("<<m_gpCenterCache.lat<<" - "<<pt.lat<<") * 10 / "<<m_uiScale10Cache<<";" << std::endl;
		std::cerr << dx1 << ',' << dy1 << std::endl;
		std::cerr << dx2 << ',' << dy2 << std::endl;
		std::cerr << dx << ',' << dy << std::endl;
		std::cerr << "m_lXScale100 = " << m_lXScale100 << std::endl; 
		std::cerr << "m_uiScale10Cache = " << m_uiScale10Cache << std::endl; 
		std::cerr << "m_spWindowCenter = (" << m_spWindowCenter.x << ", " << m_spWindowCenter.y << ")" << std::endl;
		std::cerr << "m_cos100 = " << m_cos100 << ", m_sin100 = " << m_sin100 << std::endl;
		std::cerr << "m_gpCenterCache = (" << m_gpCenterCache.lon << ", " << m_gpCenterCache.lat << ")" << std::endl;
		std::cerr << std::endl;
		*/
		return res;
	}
	
	
	struct RGB
	{
		RGB() : r(0), g(0), b(0) {}
		RGB(int r_, int g_, int b_) : r(r_), g(g_), b(b_) {}
		int r,g,b;
		void Set(Cairo::RefPtr<Cairo::Context> &cr)
		{
			cr->set_source_rgb(double(r)/255, double(g)/255, double(b)/255);
		}
	};
	struct CPen
	{
		CPen() : style(-1), width(0), color(RGB(0,0,0)) {}
		CPen(int style_, int width_, RGB color_) : style(style_), width(width_), color(color_) {}
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
		CBrush() : color(RGB(0,0,0)) {}
		CBrush(RGB color_) : color(color_) {}
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
		int m_iDiffX;
		int m_iDiffY;

		PointTools() : m_iDiffX(0), m_iDiffY(0) {}
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
	
	void InitToolsCommon()
	{
		// Create default tools
		m_hDefaultPen = CPen(1, 1, RGB(0xff, 0x0, 0x0));
		m_hDefaultBrush = CBrush(RGB(0xff, 0x0, 0x0));
		// Prepare font structures
		/*
		m_mapIcons[1] = (HICON)LoadImage(m_hResourceInst, L"satelliteno", IMAGE_ICON, 32, 32, 0);
		m_mapIcons[2] = (HICON)LoadImage(m_hResourceInst, L"satellitenofix", IMAGE_ICON, 32, 32, 0);
		m_mapIcons[3] = (HICON)LoadImage(m_hResourceInst, L"satelliteyes", IMAGE_ICON, 32, 32, 0);
		m_mapIcons[4] = (HICON)LoadImage(m_hResourceInst, L"satellitewait", IMAGE_ICON, 32, 32, 0);
		m_mapIcons[5] = (HICON)LoadImage(m_hResourceInst, L"satellitedisabled", IMAGE_ICON, 32, 32, 0);
		*/
	}
	void InitTools(const fnchar_t * strFilename)
	{
		InitToolsCommon();

		char buff[100];
		FILE * pFile = fopen(strFilename, "rt");
		if (pFile == NULL)
			return;
		while(fgets(buff, sizeof(buff), pFile) != 0)
		{
			ParseString(buff, strFilename);
		}

		if (pFile != NULL)
			fclose(pFile);
	}
	void ParseString(const char * buff, const std::fnstring & wstrBase)
	{
		std::vector<long> vRecord;
		string strRecord;
		const char * pos = buff;
		while ((*pos != 0) && (*pos != '\n') && (*pos != '\r'))
		{
			while (isspace(*pos))
				++ pos;
			char * newpos = 0;
			long lNum = strtol(pos, &newpos, 16);
			if (newpos == pos)
			{
				if (*newpos != '"')
					break;
				pos = ++newpos;
				while (*newpos && *newpos != '\n' && *newpos != '\r' && *newpos != '"')
					++newpos;
				strRecord = string(pos, newpos - pos);
				break;
			}
			pos = newpos;
			vRecord.push_back(lNum);
		}
		if (vRecord.size() > 0)
		{
			switch(vRecord[0])
			{
			case maskPolygons:
				{
					if (vRecord.size() == 5)
					{
						PolygonTools & pt = m_PolygonTools[vRecord[1]];
						pt.m_hPen = CPen(0, 1, RGB(vRecord[2], vRecord[3], vRecord[4]));
						pt.m_hBrush = CBrush(RGB(vRecord[2], vRecord[3], vRecord[4]));
					}
					if (vRecord.size() == 10)
					{
						PolygonTools & pt = m_PolygonTools[vRecord[1]];
						pt.m_hPen = CPen(vRecord[8] * 1, vRecord[9], RGB(vRecord[5], vRecord[6], vRecord[7]));
						pt.m_hBrush = CBrush(RGB(vRecord[2], vRecord[3], vRecord[4]));
					}
					break;
				}
			case maskPoints:
				{
					if (vRecord.size() == 4 && strRecord != "")
					{
						PointTools & pt = m_PointTools[vRecord[1]];
						/*
						// LINUXTODO:
						pt.m_hIcon = (HICON)LoadImage(m_hResourceInst, wstrRecord.c_str(), IMAGE_ICON, 32, 32, 0);
						if (!pt.m_hIcon)
						{
							int i = wcstol(wstrRecord.c_str(), 0, 10);
							if (i)
							pt.m_hIcon = (HICON)LoadImage(m_hResourceInst, MAKEINTRESOURCE(i), IMAGE_ICON, 32, 32, 0);
						}
						*/
						pt.m_iDiffX = vRecord[2];
						pt.m_iDiffY = vRecord[3];
					}
					if (vRecord.size() == 2 && strRecord != "")
					{
						PointTools & pt = m_PointTools[vRecord[1]];
						/*
						// LINUXTODO:
						pt.m_hIcon = (HICON)LoadImage(m_hResourceInst, wstrRecord.c_str(), IMAGE_ICON, 32, 32, 0);
						if (!pt.m_hIcon)
						{
							int i = wcstol(wstrRecord.c_str(), 0, 10);
							if (i)
							pt.m_hIcon = (HICON)LoadImage(m_hResourceInst, MAKEINTRESOURCE(i), IMAGE_ICON, 32, 32, 0);
						}
						*/
						pt.m_iDiffX = 0;
						pt.m_iDiffY = 0;
					}
					break;
				}
			case maskPolylines:
				{
					if (vRecord.size() == 7)
					{
						m_PolylinePens[vRecord[1]].first = CPen(vRecord[5] * 1, vRecord[6], RGB(vRecord[2], vRecord[3], vRecord[4]));
					}
					if (vRecord.size() == 12)
					{
						m_PolylinePens[vRecord[1]].first = CPen(vRecord[5] * 1, vRecord[6], RGB(vRecord[2], vRecord[3], vRecord[4]));
						m_PolylinePens[vRecord[1]].second = CPen(vRecord[10] * 1, vRecord[11], RGB(vRecord[7], vRecord[8], vRecord[9]));
					}
					break;
				}
			case 0:
				{
					if (vRecord.size() == 4)
					{
						m_crBg = RGB(vRecord[1], vRecord[2], vRecord[3]);
						m_hBgBrush = CBrush(m_crBg);	
					}
					break;
				}
			case 1:
				{
					if (vRecord.size() == 4)
					{
						m_crText = RGB(vRecord[1], vRecord[2], vRecord[3]);
					}
					break;
				}
			case 2:
				{
					if (vRecord.size() == 4)
					{
						m_hCursorPen = CPen(0, 1, RGB(vRecord[1], vRecord[2], vRecord[3]));
						m_hCursorBrush = CBrush(RGB(vRecord[1], vRecord[2], vRecord[3]));
					}
					break;
				}
			case 3:
				{
					/*
					// LINUXTODO:
					if (vRecord.size() == 4)
						m_FontCache.SetTemplate(vRecord[1], vRecord[2], vRecord[3] != 0);
					*/
					break;
				}
			}
		}
		/*
		else if (!wstrRecord.empty())
		{
			m_hResourceInst = LoadLibraryEx(MakeFilename(wstrRecord, wstrBase).c_str(), 0, LOAD_LIBRARY_AS_DATAFILE);
		}
		*/
	}
};

int main(int argc, char ** argv)
{
	Gtk::Main    toolkit (argc, argv);

	if (!argv[1])
	{
		return -1;
	}
	CIMGFile f;
	f.Parse(argv[1]);
	DumpPainter dp(f);
	dp.m_gpCenter = f.GetCenter();
	dp.m_ruiScale10 = 100;
	dp.iDegree360 = 0;
	dp.InitTools("Resources/Normal.vpc");
	
	Gtk::Window  window;
	window.add(dp);
	dp.show();
 	Gtk::Main::run (window);
 	// toolkit.run(dp);

	return 0;
}
