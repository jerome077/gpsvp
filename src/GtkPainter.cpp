#include "Common.h"
#include "Header.h"
#include "IPainter.h"

#include <iostream>
#include <gtkmm.h>

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
			if (type != 0x4b)
				cr->fill();
		}
		else
			cr->stroke();
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
	
	Gtk::Window  window;
	window.add(dp);
	dp.show();
 	Gtk::Main::run (window);
 	// toolkit.run(dp);

	return 0;
}
