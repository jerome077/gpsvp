#include "GtkPainter.h"
#include "MapApp.h"

CGTKPainter::~CGTKPainter()
{
}

void CGTKPainter::NoFix() {
	std::cerr << "NoDix" << std::endl;
}

CMapApp app;

int main(int argc, char ** argv)
{
	Gtk::Main    toolkit (argc, argv);
	CGTKPainter dp;


	while (*++argv)
		dp.AddMap(*argv);
	// Glib::Thread::create(sigc::mem_fun(&dp, &DumpPainter::ThreadRoutine), true);
 	Gtk::Main::run (dp);
 	// toolkit.run(dp);

	return 0;
}


void CGTKPainter::Init(HKEY, HWND) {}
void CGTKPainter::RedrawMonitors() {}
void CGTKPainter::SetShowMonitors(bool fShow) {}
bool CGTKPainter::IsFullScreen() {return false;}
void CGTKPainter::SetFullScreen(bool fFull) {}
void CGTKPainter::BeginPaint() {}
void CGTKPainter::EndPaint() {}
void CGTKPainter::PaintScale() {}
void CGTKPainter::PaintStatusLine(const tchar_t * wcName) {};
void CGTKPainter::PaintLowMemory(const tchar_t * wcString1, const tchar_t * wcString2){}
void CGTKPainter::PaintStatusIcon(int iIcon) {};
void CGTKPainter::PaintCompass() {};
ScreenRect CGTKPainter::GetMonitorsBar() {return ScreenRect();};
void CGTKPainter::ClearButtons() {};

//IMonitorPainter
void CGTKPainter::DrawTextMonitor(const tchar_t * wcLabel, const tchar_t * wcText){}
void CGTKPainter::DrawMonitorLabel(const tchar_t * wcLabel){}
ScreenPoint CGTKPainter::GetMonitorSize(){return ScreenPoint();}
void CGTKPainter::DrawBar(const ScreenRect & srBar){}
void CGTKPainter::SetCurrentMonitor(const ScreenRect & srRect, bool fActive){}

// IButtonPainter
void CGTKPainter::AddButton(const tchar_t * wcLabel, int iCommand, bool fSelected) {};


CGTKPainter::CGTKPainter()
	: pressed(false)
{
	m_ruiScale10 = 100;
	iDegree360 = 0;
	InitTools("Resources/Normal.vpc");
	m_fExiting = false;
	
	Glib::thread_init();
	signal_expose_event().connect(sigc::mem_fun(this, &CGTKPainter::on_expose_event), false);
	signal_scroll_event().connect(sigc::mem_fun(this, &CGTKPainter::on_scroll));
	signal_button_press_event().connect(sigc::mem_fun(this, &CGTKPainter::on_press));
	signal_button_release_event().connect(sigc::mem_fun(this, &CGTKPainter::on_release));
	signal_motion_notify_event().connect(sigc::mem_fun(this, &CGTKPainter::on_motion));
	// window.signal_scroll_event().connect(sigc::mem_fun(dp, &CGTKPainter::on_scroll));
	set_events(Gdk::SCROLL_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);
}
void CGTKPainter::AddMap(const tchar_t * name)
{
	a.Add(name, this);
}
void CGTKPainter::StartPolygon(UInt uiType, const tchar_t * wcName) 
{ 
	cr->begin_new_path();
	started = false;
	polygon = true;
	type = uiType;
	name = wcName;
	m_pointList.resize(0);
};
void CGTKPainter::StartPolyline(UInt uiType, const tchar_t * wcName) 
{ 
	cr->begin_new_path();
	started = false;
	polygon = false;
	type = uiType;
	name = wcName;
	m_pointList.resize(0);
};

void CGTKPainter::CalculateLabelSize()
{
	Check(name != 0);
	cr->set_font_size(14);
	cr->get_text_extents(name, m_LabelSize);
}

void CGTKPainter::FinishObject() 
{ 
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
		cr->fill_preserve();
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
			cr->stroke_preserve();
		}
		if (name != NULL)
		{
			int len2 = sqr(m_pointList.front().x - m_pointList.back().x) + sqr(m_pointList.front().y - m_pointList.back().y);
			CalculateLabelSize();
			if (len2 > sqr(100) || len2 > sqr(m_LabelSize.width) )
			{
				int m_iWriteSegment = (m_pointList.size() - 1) / 2;
				int x1 = m_pointList[m_iWriteSegment].x;
				int y1 = m_pointList[m_iWriteSegment].y;
				int x2 = m_pointList[m_iWriteSegment + 1].x;
				int y2 = m_pointList[m_iWriteSegment + 1].y;
				if (x1 > x2)
				{
					swap(x1, x2);
					swap(y1, y2);
				}
				double dx = x2 - x1;
				double dy = y2 - y1;
				double d = sqrt(double(dx * dx + dy * dy));
				
				if (d > 0)
				{
					int x, y;
					x = int(x1 + double(dy) * (m_LabelSize.height / 2 - 2) / d);
					y = int(y1 - double(dx) * (m_LabelSize.height / 2 - 2) / d);

					x += int(dx * (d - m_LabelSize.width) / 2 / d);
					y += int(dy * (d - m_LabelSize.width) / 2 / d);

					// HFONT hFont = m_FontCache.GetFont(1, angle);
					// m_hdc.SelectObject(hFont);
					// m_hdc.SetTextColor(m_crBg);
					// m_hdc.ExtTextOut(x-1, y, 0, 0, m_wcName, 0);
					// m_hdc.ExtTextOut(x+1, y, 0, 0, m_wcName, 0);
					// m_hdc.ExtTextOut(x, y-1, 0, 0, m_wcName, 0);
					// m_hdc.ExtTextOut(x, y+1, 0, 0, m_wcName, 0);
					// m_hdc.SetTextColor(m_crText);
					// m_hdc.ExtTextOut(x, y, 0, 0, m_wcName, 0);

					Cairo::Matrix m;
					m.xx = dx / d * 14;
					m.xy = -dy / d * 14;
					m.yx = dy / d * 14;
					m.yy = dx / d * 14;
					m.x0 = 0;
					m.y0 = 0;
					cr->set_font_matrix(m);
					m_crBg.Set(cr);
					cr->move_to(x-1, y);
					cr->show_text(name);
					cr->move_to(x+1, y);
					cr->show_text(name);
					cr->move_to(x, y-1);
					cr->show_text(name);
					cr->move_to(x, y+1);
					cr->show_text(name);
					m_crText.Set(cr);
					cr->move_to(x, y);
					cr->show_text(name);
				}
			}
		}		
	}
};
void CGTKPainter::AddPoint(const ScreenPoint & p)
{
	// std::cerr << "AddPoint (" << p.x << "," << p.y << ")" << std::endl;
	if (started)
		cr->line_to(p.x, p.y);
	else
		cr->move_to(p.x, p.y);
	started = true;
	m_pointList.push_back(p);
}
void CGTKPainter::AddPoint(const GeoPoint & gp) 
{ 
	AddPoint(GeoToScreen(gp));
};
bool CGTKPainter::WillPaint(const GeoRect & rect) { 
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
void CGTKPainter::PaintPoint(UInt uiType, const GeoPoint & gp, const tchar_t * wcName) 
{ 
	ScreenPoint sp = GeoToScreen(gp);
	/*
	if (false == WillPaint(sp))
		return;
	*/

	if (uiType < 0x8000 || uiType == 0xfffe || uiType == 0xffff)
	{
		if (!m_srsPoints.Fit(ScreenRect(sp, ScreenSize(32,32))))
			return;
	}

	PointToolMap::iterator it = m_PointTools.find(uiType);
	if (it != m_PointTools.end())
	{
		sp.x -= it->second.m_iDiffX;
		sp.y -= it->second.m_iDiffY;
		if (it->second.s)
		{
			cr->set_source(it->second.s, sp.x - 16, sp.y - 16);
			cr->paint();
		}
	}
	else
	{
		// std::cerr << "Not found" << std::endl;
	}
}
void CGTKPainter::SetLabelMandatory() { std::cerr << "SetLabelMandatory" << std::endl; };
GeoRect CGTKPainter::GetRect() { std::cerr << "GetRect" << std::endl;};

bool CGTKPainter::on_expose_event(GdkEventExpose* event)
{
	PrepareScales();
	m_cos100 = int(cos(double(iDegree360) / 180 * pi) * 100);
	m_sin100 = int(sin(double(iDegree360) / 180 * pi) * 100);
	
	GdkWindow* window = event->window;
	int width, height;
	gdk_drawable_get_size(window, &width, &height);
	m_spWindowCenter = ScreenPoint(width / 2, height / 2);
	m_srWindow.Init(ScreenPoint(0, 0));
	m_srWindow.Append(ScreenPoint(width, height));

	pixmap = Gdk::Pixmap::create(get_window(), width, height);
	// pixmap->set_colormap(window->get_colormap());
	cr = pixmap->create_cairo_context();
	// window->create_cairo_context();
	cr->set_source_rgb(1, 1, 1);
	cr->begin_new_path();
	cr->move_to(0, 0);
	cr->line_to(width, 0);
	cr->line_to(width, height);
	cr->line_to(0, height);
	cr->line_to(0, 0);
	cr->fill_preserve();
	
	a.BeginPaint(m_uiScale10Cache, this, this);
	a.PaintMapPlaceholders(this);
	a.Paint(maskPolygons, true);
	a.Paint(maskPolylines, true);
	m_srsPoints.Reset();
	a.Paint(maskPoints, true);
	cr.clear();
	static Glib::RefPtr<Gdk::GC> dc = Gdk::GC::create(get_window());
	get_window()->draw_drawable(dc, pixmap, 0, 0, 0, 0, width, height);
	return true;
}
void CGTKPainter::PrepareScales()
{
	m_gpCenterCache = m_gpCenter;
	m_uiScale10Cache = m_ruiScale10;
	m_lXScale100 = cos100(m_gpCenterCache.lat);
}

void CGTKPainter::InitToolsCommon()
{
	// Create default tools
	m_hDefaultPen = CPen(1, 1, RGB(0xff, 0x0, 0x0));
	m_hDefaultBrush = CBrush(RGB(0xff, 0x0, 0x0));
	// Prepare font structures
	/*
	m_mapIcons[1] = (HICON)LoadImage(m_hResourceInst, L("satelliteno"), IMAGE_ICON, 32, 32, 0);
	m_mapIcons[2] = (HICON)LoadImage(m_hResourceInst, L("satellitenofix"), IMAGE_ICON, 32, 32, 0);
	m_mapIcons[3] = (HICON)LoadImage(m_hResourceInst, L("satelliteyes"), IMAGE_ICON, 32, 32, 0);
	m_mapIcons[4] = (HICON)LoadImage(m_hResourceInst, L("satellitewait"), IMAGE_ICON, 32, 32, 0);
	m_mapIcons[5] = (HICON)LoadImage(m_hResourceInst, L("satellitedisabled"), IMAGE_ICON, 32, 32, 0);
	*/
}
void CGTKPainter::InitTools(const tchar_t * strFilename)
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
void CGTKPainter::ParseString(const char * buff, const std::fnstring & wstrBase)
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
					try {
						pt.s = Cairo::ImageSurface::create_from_png((MakeFilename(strRecord, wstrBase) + L(".png")).c_str());
					} catch (std::exception &) {}
					/*
					// LINUXTODO:
					pt.m_hIcon = (HICON)LoadImage(m_hResourceInst, wstrRecord.c_str(), IMAGE_ICON, 32, 32, 0);
					if (!pt.m_hIcon)
					{
						int i = tcstol(wstrRecord.c_str(), 0, 10);
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
					try {
						pt.s = Cairo::ImageSurface::create_from_png((MakeFilename(strRecord, wstrBase) + L(".png")).c_str());
					} catch (std::exception &) {}
					/*
					// LINUXTODO:
					pt.m_hIcon = (HICON)LoadImage(m_hResourceInst, wstrRecord.c_str(), IMAGE_ICON, 32, 32, 0);
					if (!pt.m_hIcon)
					{
						int i = tcstol(wstrRecord.c_str(), 0, 10);
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
bool CGTKPainter::on_scroll(GdkEventScroll * event)
{
	if (event->direction == GDK_SCROLL_UP)
		ZoomIn();
	else
		ZoomOut();
}
bool CGTKPainter::on_press(GdkEventButton* event) 
{
	orig_x = event->x;
	orig_y = event->y;
	pressed = true;
	return true;
}
bool CGTKPainter::on_release(GdkEventButton* event) 
{
	Move(ScreenDiff(event->x - orig_x, event->y - orig_y));
	pressed = false;
	return true;
}
bool CGTKPainter::on_motion(GdkEventMotion* event)
{
	if (pressed) {
		Glib::RefPtr<Gdk::Window> window = get_window();
		Gtk::Allocation allocation = get_allocation();
		const int width = allocation.get_width();
		const int height = allocation.get_height();
		static Glib::RefPtr<Gdk::GC> dc = Gdk::GC::create(window);
		window->clear();
		window->draw_drawable(dc, pixmap, 0, 0, event->x - orig_x, event->y - orig_y, width, height);
	}
}
void CGTKPainter::Move(ScreenDiff d)
{
	SetView(ScreenToGeo(m_spWindowCenter - d), true);
}

void CGTKPainter::Redraw()
{
	if (get_window())
		get_window()->invalidate_rect(get_allocation(), true);
}

void CGTKPainter::PaintText(const tchar_t * wcText){}
void CGTKPainter::SetProgressItems(int iLevel, int iCount){}
void CGTKPainter::SetProgress(int iLevel, int iProgress){}
void CGTKPainter::Advance(int iLevel){}

void CGTKPainter::NoFix();
void CGTKPainter::Fix(GeoPoint gp, double, double dHDOP) { m_gpCenter = gp; Redraw(); }
void CGTKPainter::NoVFix() {}
void CGTKPainter::VFix(double dAltitude, double) {}
void CGTKPainter::SetConnectionStatus(enumConnectionStatus iStatus) {}
