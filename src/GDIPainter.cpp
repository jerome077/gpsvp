/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "GDIPainter.h"

#include <math.h>
#include "Common.h"
#include "MapApp.h"

extern HINSTANCE g_hInst;

void CGDIPainter::StartPolygon(UInt uiType, const wchar_t * wcName)
{
	// Record type
	m_uiType = uiType;
	// Clean list of points
	m_pointList.resize(0);
	// It is polygon
	m_fPolygon = true;
	m_wcName = wcName;
	m_fMandatoryLabel = false;
};

void CGDIPainter::CalculateLabelSize()
{
	// This method should only be called when name is present
	Check(m_wcName != 0);
	// Caller should check it himself
	Check(!m_fLabelSizeKnown);
	// Select standard font
	HFONT hFont = m_FontCache.GetFont(1, 0);
	m_hdc.SelectObject(hFont);
	// Get text box size
	m_hdc.getTextExtentPoint(m_wcName, &m_LabelSize);
	// Size is know now
	m_fLabelSizeKnown = true;
}


void CGDIPainter::SetLabelMandatory()
{
	m_fMandatoryLabel = true;
}

void CGDIPainter::StartPolyline(UInt uiType, const wchar_t * wcName)
{
	// Record type
	m_uiType = uiType;
	// Clean list of points
	m_pointList.resize(0);
	// It is not polygon
	m_fPolygon = false;
	// Record name
	// Elevation can be specified for POI objects like summit (Type 0x6616) and depth / height
	// points (Types 0x6200 & 0x6300) as well as for polyline objects like land / depth contours
	// (Types=0x20 to 0x25).
	static wstring wstrReplace;
	if (wcName && (uiType >= 0x20 && uiType <= 0x25))
	{
		wstrReplace = app.HeightFromFeet(wcName);
		m_wcName = wstrReplace.c_str();
	}
	else
		m_wcName = wcName;
	// No write segment yet
	m_iWriteSegment = -1;
	// No write segment size also
	m_iWriteSegmentD2 = 0;
	// Label size is unknown
	m_fLabelSizeKnown = false;
	m_fMandatoryLabel = false;
};
void CGDIPainter::FinishObject()
{
	// It is background. We dont paint it
	if (m_uiType == 75)
		return;
	// Эта проверка отключена, так как её теперь выполняет сам 
	// объект до начала рисования. Это дешевле, так как он может
	// закешировать свои границы
//	if (!WillPaint(m_curRect))
//		return;
	if (m_fPolygon == true)
	{
		// For polygons
		// We look for painting tools
		PolygonToolMap::iterator it = m_PolygonTools.find(m_uiType);
		if (it != m_PolygonTools.end())
		{
			PolygonTools & t = it->second;
			// If found, use 'em
			m_hdc.SelectObject(t.m_hPen);
			m_hdc.SelectObject(t.m_hBrush);
		}
		else
		{
			if (!m_fShowUnknownTypes)
				return;
			// Else use default
			m_hdc.SelectObject(m_hDefaultPen);
			m_hdc.SelectObject(m_hDefaultBrush);
		}
		// GDI function for painting polygons
		if (m_fShowAreaAsOutline)
		{
			// Polygons implicitely draw the closing line, for 
			// Polylines, the closing needs to be done explicit
			m_pointList[m_pointList.size()]=m_pointList[0];
			m_hdc.Polyline(&m_pointList[0], m_pointList.size()+1);
			// (not sure if I should have declared the array as 1 larger....)
		}
		else
		{
			m_hdc.Polygon(&m_pointList[0], m_pointList.size());
		}
		if (m_fShowPolygonLabels && m_wcName && m_wcName[0])
		{
			m_hdc.SelectObject(m_FontCache.GetFont(2, 0));
			ScreenSize sz;
			m_hdc.getTextExtentPoint(m_wcName, &sz);
			if (m_curRect.Height() * 2 > sz.cy && m_curRect.Width() * 2 > sz.cx)
			{
				m_hdc.SetTextColor(m_crBg);
				m_hdc.ExtTextOut(m_curRect.Center().x - sz.cx / 2 + 1, m_curRect.Center().y - sz.cy / 2, 0, 0, m_wcName, 0);
				m_hdc.ExtTextOut(m_curRect.Center().x - sz.cx / 2 - 1, m_curRect.Center().y - sz.cy / 2, 0, 0, m_wcName, 0);
				m_hdc.ExtTextOut(m_curRect.Center().x - sz.cx / 2, m_curRect.Center().y - sz.cy / 2 + 1, 0, 0, m_wcName, 0);
				m_hdc.ExtTextOut(m_curRect.Center().x - sz.cx / 2, m_curRect.Center().y - sz.cy / 2 - 1, 0, 0, m_wcName, 0);
				m_hdc.SetTextColor(m_crText);
				m_hdc.ExtTextOut(m_curRect.Center().x - sz.cx / 2, m_curRect.Center().y - sz.cy / 2, 0, 0, m_wcName, 0);
			}
		}
	}
	else
	{
		// A little bit different for polylines
		// We look for painting tools
		PolylineToolMap::iterator it = m_PolylinePens.find(m_uiType);
		if ((m_uiType & 0x100) != 0)
		{
			if (m_pointList.size() > 1)
			{
				ScreenPoint p1 = m_pointList[m_pointList.size() - 2];
				ScreenPoint p2 = m_pointList[m_pointList.size() - 1];
				ScreenDiff d1 = p2 - p1;
				int l = int(sqrt(double(d1.dx * d1.dx + d1.dy * d1.dy)));
				if (l > 0)
				{
					d1 *= 6;
					d1 /= l;
					ScreenDiff d2 = ScreenDiff(-d1.dy, d1.dx);
					ScreenDiff d3 = d2;
					d3 /= 3;
					p1 += d3;
					p2 += d3;

					m_pointList[m_pointList.size() - 2] = p1;
					m_pointList[m_pointList.size() - 1] = p2;
					if (l > 25)
					{
						ScreenPoint p3 = p2 - d1 - d2;
						ScreenPoint p4 = p2 - d1 + d2;
						m_pointList.push_back(p3);
						m_pointList.push_back(p2);
						m_pointList.push_back(p4);
						m_pointList.push_back(p2);
					}
				}
			}
		}

		if (it != m_PolylinePens.end())
		{
			// Use first pen
			m_hdc.SelectObject(it->second.first);
			// To paint polyline
			m_hdc.Polyline(&m_pointList[0], m_pointList.size());
			// And if second is present
			if (it->second.second)
			{
				// Use it
				m_hdc.SelectObject(it->second.second);
				// To paint the same line, e.g. some internal color or pattern
				m_hdc.Polyline(&m_pointList[0], m_pointList.size());
			}
		}
		else
		{
			if (!m_fShowUnknownTypes)
				return;
			// Else use default pen
			m_hdc.SelectObject(m_hDefaultPen);
			// To paint the polyline
			m_hdc.Polyline(&m_pointList[0], m_pointList.size());
		}

//		if (m_iWriteSegment != -1)
		if (m_wcName != NULL)
		{
			int len2 = sqr(m_pointList.front().x - m_pointList.back().x) + sqr(m_pointList.front().y - m_pointList.back().y);
			if (m_fLabelSizeKnown == false)
				CalculateLabelSize();
			if (len2 > sqr(100) || true == m_fMandatoryLabel || len2 > sqr(m_LabelSize.cx) )
			{
				m_iWriteSegment = (m_pointList.size() - 1) / 2;
				int x1 = m_pointList[m_iWriteSegment].x;
				int y1 = m_pointList[m_iWriteSegment].y;
				int x2 = m_pointList[m_iWriteSegment + 1].x;
				int y2 = m_pointList[m_iWriteSegment + 1].y;
				if (x1 > x2)
				{
					swap(x1, x2);
					swap(y1, y2);
				}
				int dx = x2 - x1;
				int dy = y2 - y1;
				double tan = - double(dy) / (dx);
				int angle = int(atan(tan) * 1800 / 3.14159);
				if (angle < 0)
					angle += 3600;
				double d = sqrt(double(dx * dx + dy * dy));

				int x, y;
				x = int(x1 + double(dy) * (m_LabelSize.cy + 2) / d);
				y = int(y1 - double(dx) * (m_LabelSize.cy + 2) / d);

				x += int(dx * (d - m_LabelSize.cx) / 2 / d);
				y += int(dy * (d - m_LabelSize.cx) / 2 / d);

				HFONT hFont = m_FontCache.GetFont(1, angle);
				m_hdc.SelectObject(hFont);
				m_hdc.SetTextColor(m_crBg);
				m_hdc.ExtTextOut(x-1, y, 0, 0, m_wcName, 0);
				m_hdc.ExtTextOut(x+1, y, 0, 0, m_wcName, 0);
				m_hdc.ExtTextOut(x, y-1, 0, 0, m_wcName, 0);
				m_hdc.ExtTextOut(x, y+1, 0, 0, m_wcName, 0);
				m_hdc.SetTextColor(m_crText);
				m_hdc.ExtTextOut(x, y, 0, 0, m_wcName, 0);
			}
		}		
	}
}

void CGDIPainter::AddPoint(const GeoPoint & gpt) 
{
	AddPoint(GeoToScreen(gpt));
}

void CGDIPainter::AddPoint(ScreenPoint pt)
{
	bool fReplace = false;
	// We should check all but the first point
	if (m_pointList.size() > 0)
	{
		ScreenPoint spPrev = m_pointList.back();
		// Maybe it's the same point
		if (spPrev == pt)
			return;
		// And calculate min and max of coordinates
		// TODO: move down
		m_curRect.Append(pt);

		Int iOffType = WillPaintEx(pt);
		if (0 != iOffType && (iOffType == m_iOffTypePrev) && (iOffType == m_iOffTypePrevPrev))
			fReplace = true;
		m_iOffTypePrevPrev = m_iOffTypePrev;
		m_iOffTypePrev = iOffType;
	}
	else
	{
		// And it is min and max now
		m_curRect.Init(pt);
		if (m_fPolygon == true)
			m_iOffTypePrev = 0;
		else
			m_iOffTypePrev = WillPaintEx(pt);
		m_iOffTypePrevPrev = 0;
	}
	// And add the point to list
	if (fReplace == true)
		m_pointList.back() = pt;
	else
		m_pointList.push_back(pt);
}

bool CGDIPainter::WillPaint(const GeoRect & gRect)
{
	ScreenRect sRect = GeoToScreen(gRect);
	GeoRect grScreen = ScreenToGeo(m_srWindow);
	bool i1 = m_srWindow.Intersect(sRect);
	bool i2 = grScreen.Intersect(gRect);
	return i1 && i2;
}

bool CGDIPainter::WillPaintLine(const GeoPoint & pt1, const GeoPoint & pt2)
{
	ScreenRect rect;
	rect.Init(GeoToScreen(pt1));
	rect.Append(GeoToScreen(pt2));
	return WillPaint(rect);
}

void CGDIPainter::EndPaint()
{
	m_hdc = VP::DC();
}

void CGDIPainter::BeginPaintLite(VP::DC hdc)
{
	m_hdc = hdc;
	m_iCurrentButton = m_srWindow.bottom - 20;
}

void CGDIPainter::PrepareScales()
{
	m_gpCenterCache = m_gpCenter();
	m_uiScale10Cache = m_ruiScale10();
	m_lXScale100 = cos100(m_gpCenterCache.lat);
}

void CGDIPainter::BeginPaint(HWND hWnd, VP::DC hdc, RECT srRegion, int iDegree360, bool fLowCenter)
{
	m_cos100 = int(cos(double(iDegree360) / 180 * pi) * 100);
	m_sin100 = int(sin(double(iDegree360) / 180 * pi) * 100);
	if (m_fViewSet)
	{
		m_gpCenter.Set(m_gpCenterView);
	}

	PrepareScales();

	// Get client rect
	GetClientRect(hWnd, &m_srWindow);
	m_fVertical = m_srWindow.right - m_srWindow.left < m_srWindow.bottom - m_srWindow.top;
	if (m_fBottomBar)
	{
		if (m_fVertical)
			m_iBottomBar = (m_srWindow.bottom - m_srWindow.top) / (app.m_Options[mcoLargeMonitors] ? 7 : 10);
		else
			m_iBottomBar = (m_srWindow.right - m_srWindow.left) / (app.m_Options[mcoLargeMonitors] ? 3 : 4);
	}
	else
		m_iBottomBar = 0;
	
	if (m_fVertical)
		m_srWindow.bottom -= m_iBottomBar;
	else
		m_srWindow.right -= m_iBottomBar;
	if (m_srWindow.Intersect(ScreenRect(srRegion)))
		hdc.FillRect(&m_srWindow, m_hBgBrush);
	// Calculate center coordinates
	if (fLowCenter)
	{
		m_spWindowCenter = ScreenPoint((m_srWindow.right + m_srWindow.left) / 2,
			(m_srWindow.bottom * 3 + m_srWindow.top) / 4);
	}
	else
	{
		m_spWindowCenter = ScreenPoint((m_srWindow.right + m_srWindow.left) / 2,
			(m_srWindow.bottom + m_srWindow.top) / 2);
	}
	// Remember device context
	m_hdc = hdc;
	// Set transparent background
	hdc.SetBkMode(0);
	m_srsLabels.Reset();
	m_srsPoints.Reset();
	m_iCurrentStatusIcon = m_srWindow.top;
	m_iCurrentButton = m_srWindow.bottom - 20;
	m_fShowUnknownTypes = true;
	m_fShowPolygonLabels = true;
	m_fShowAreaAsOutline = true;
	m_iStatusLineOffset = 0;
}
void CGDIPainter::SetView(const GeoPoint & gp, bool fManual)
{
	AutoLock l;
	if (fManual && app.m_Options[mcoFollowCursor])
		m_iManualTimer = 60;
	else if (m_iManualTimer > 0)
		return;
	GeoPoint actual = gp;
	const int cnMaxLat = 0x360000;
	const int cnMaxLon = 0x800000;
	if (actual.lat > cnMaxLat)
		actual.lat = cnMaxLat;
	if (actual.lat < -cnMaxLat)
		actual.lat = -cnMaxLat;
	if (actual.lon > cnMaxLon)
		actual.lon = cnMaxLon;
	if (actual.lon < -cnMaxLon)
		actual.lon = -cnMaxLon;

	m_gpCenterView = actual;
	m_fViewSet = true;
	Redraw();
}

void CGDIPainter::InitTools(int iScheme)
{
	wchar_t wcFilename[MAX_PATH + 1] = {0};
	GetModuleFileName(0, wcFilename, MAX_PATH);

	InitToolsCommon();
	HRSRC hResource;
	hResource = FindResource(m_hResourceInst, L"VPC1", RT_RCDATA);
	HGLOBAL hGlobal = LoadResource(m_hResourceInst, hResource);
	DWORD dwSize = SizeofResource(m_hResourceInst, hResource);
	char * data = (char *)LockResource(hGlobal);
	char * from = data;
	while (data != NULL && *data != 0 && (data < from + dwSize) )
	{
		ParseString(data, wcFilename);
		while (*data != 0)
		{
			++data;
			if (*data == '\n')
			{
				while (isspace(*data))
					++data;
				break;
			}
		}
	}
	if (iScheme == 1)
	{
		hResource = FindResource(m_hResourceInst, L"VPC2", RT_RCDATA);
		HGLOBAL hGlobal = LoadResource(m_hResourceInst, hResource);
		char * data = (char *)LockResource(hGlobal);
		char * from = data;
		while (data != NULL && *data != 0 && (data < from + dwSize) )
		{
			ParseString(data, wcFilename);
			while (*data != 0)
			{
				++data;
				if (*data == '\n')
				{
					while (isspace(*data))
						++data;
					break;
				}
			}
		}
	}
}

void CGDIPainter::InitTools(const wchar_t * strFilename)
{
	InitToolsCommon();

	char buff[100];
	FILE * pFile = wfopen(strFilename, L"rt");
	if (pFile == NULL)
		return;
	while(fgets(buff, sizeof(buff), pFile) != 0)
	{
		ParseString(buff, strFilename);
	}

	if (pFile != NULL)
		fclose(pFile);
}

void CGDIPainter::ZoomIn()
{
	if (m_ruiScale10() == ciMinZoom)
		return;
	// Decrease scale twice
	// Correct if minimum reached
	m_ruiScale10.Set(max((int)(ciMinZoom), m_ruiScale10() / 2));
	Redraw();
}
void CGDIPainter::ZoomOut()
{
	if (m_ruiScale10() == ciMaxZoom)
		return;
	// Increase scale twice
	// Correct if maximum reached
	m_ruiScale10.Set(min((int)(ciMaxZoom), m_ruiScale10() * 2));
	Redraw();
}
void CGDIPainter::Left()
{
	// Move view left by 30 screen points
	Move(ScreenDiff(30, 0));
}
void CGDIPainter::Right()
{
	// Move view right by 30 screen points
	Move(ScreenDiff(-30, 0));
}
void CGDIPainter::Up()
{
	// Move view up by 30 screen points
	Move(ScreenDiff(0, 30));
}
void CGDIPainter::Down()
{
	// Move view down by 30 screen points
	Move(ScreenDiff(0, -30));
}
void CGDIPainter::Move(ScreenDiff d)
{
	if (d.Null() == true)
		return;
	// Move view by given number of screen points
	SetView(ScreenToGeo(m_spWindowCenter - d), true);
}

bool CGDIPainter::WillPaint(const ScreenRect & rect)
{
	return m_srWindow.Intersect(rect);
}

bool CGDIPainter::WillPaint(const ScreenPoint & pt)
{
	return m_srWindow.Side(pt) == 0;
}

Int CGDIPainter::WillPaintEx(const ScreenPoint & pt)
{
	// Check if given rect intersects scree rect
	return m_srWindow.Side(pt);
}

void CGDIPainter::Init(HWND hWnd, HKEY hRegKey) 
{
	AutoLock l;
	m_hWnd = hWnd;
	m_hResourceInst = g_hInst;

	m_gpCenter.Init(hRegKey, L"Center", GeoPoint(0, 0));
	m_fViewSet = false;
	m_ruiScale10.Init(hRegKey, L"ScaleD", 500);
	m_ruiScale10.Set(max((int)(ciMinZoom), m_ruiScale10()));
	m_ruiScale10.Set(min((int)(ciMaxZoom), m_ruiScale10()));

	m_lXScale100 = cos100(m_gpCenter().lat);
	m_fBottomBar = false;
	m_fFullScreen = false;
	m_srWindow.Init(ScreenPoint(0,0));
	m_srWindow.Append(ScreenPoint(1,1));
	m_iManualTimer = 0;

	m_PolygonTools[0x100].m_hPen = CreatePen(PS_SOLID, 1, RGB(0x00, 0x00, 0x00));
	m_PolygonTools[0x100].m_hBrush = CreateSolidBrush(RGB(0x00, 0x00, 0xff));
	m_PolygonTools[0x101].m_hPen = CreatePen(PS_SOLID, 1, RGB(0x00, 0x00, 0x00));
	m_PolygonTools[0x101].m_hBrush = CreateSolidBrush(RGB(0xff, 0x00, 0x00));
}
void CGDIPainter::Redraw()
{
	RedrawWindow(m_hWnd, 0, 0, RDW_INVALIDATE);
}
void CGDIPainter::RedrawMonitors()
{
	ScreenRect rc = GetMonitorsBar();
	RedrawWindow(m_hWnd, &rc, 0, RDW_INVALIDATE);
}

void CGDIPainter::PaintPoint(UInt uiType, const GeoPoint & gp, const wchar_t * wcName)
{
	ScreenPoint sp = GeoToScreen(gp);
	if (false == WillPaint(sp))
		return;

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
		if (uiType == 0xfffe) 
		{
			const int r = 16;
			m_hdc.SelectObject(m_hDefaultPen);
			m_hdc.SelectObject(m_hDefaultBrush);
			m_hdc.RoundRect(sp.x - r, sp.y - r, sp.x + r, sp.y + r, 7, 7);
		}
		if (it->second.m_hIcon)
			m_hdc.drawIcon(sp.x - 16, sp.y - 16, it->second.m_hIcon);
		else if (it->second.m_hBmp)
		{
			VP::DC dc(m_hdc, 0, 0);
			dc.SelectObject(it->second.m_hBmp);
			m_hdc.BitBlt(sp.x - 16, sp.y - 16, 32, 32, dc, 0, 0, SRCCOPY);
		}
	}
	else
	{
		if (uiType != 0x10000)
		{
			m_setUnknownTypes.insert(uiType);
			if (m_fShowUnknownTypes)
				it = m_PointTools.find(0);
			else
				return;
		}
		else
		{
		}
		if (it != m_PointTools.end())
		{
			m_hdc.drawIcon(sp.x - 16, sp.y - 16, it->second.m_hIcon);
		}
		else
		{
			m_hdc.SelectObject(m_hDefaultPen);
			m_hdc.SelectObject(m_hDefaultBrush);
			int r = 1;
			if (uiType > 0x7fff)
				r = 2;
			if (uiType == 0x10000)
			{
				r = 5;
				m_hdc.SelectObject(m_hCursorBrush);
				m_hdc.SelectObject(m_hCursorPen);
			}
			m_hdc.Ellipse(sp.x - r, sp.y - r, sp.x + r, sp.y + r);
		}
	}
		
	if (wcName != NULL && *wcName != 0)
	{
		// Elevation can be specified for POI objects like summit (Type 0x6616) and depth / height
		// points (Types 0x6200 & 0x6300) as well as for polyline objects like land / depth contours
		// (Types=0x20 to 0x25).
		wstring wstrNameReplace;
		if (uiType == 0x6616 || uiType == 0x6200 || uiType == 0x6300)
		{
			wstrNameReplace = app.HeightFromFeet(wcName);
			wcName = wstrNameReplace.c_str();
		}
		ScreenSize ssLabel;
		m_hdc.getTextExtentPoint(wcName, &ssLabel);
		// sp.y -= ssLabel.cy / 2;
		sp.x += 2;
		ScreenRect srLabel(sp, ssLabel);
		if (true == m_srsLabels.Fit(srLabel))
		{
			if (uiType > 0x7fff)
				m_hdc.SelectObject(m_FontCache.GetFont(3, 0));
			else
				m_hdc.SelectObject(m_FontCache.GetFont(2, 0));
			m_hdc.SetTextColor(m_crBg);
			m_hdc.ExtTextOut(sp.x - 1, sp.y, 0, 0, wcName, 0);
			m_hdc.ExtTextOut(sp.x + 1, sp.y, 0, 0, wcName, 0);
			m_hdc.ExtTextOut(sp.x, sp.y - 1, 0, 0, wcName, 0);
			m_hdc.ExtTextOut(sp.x, sp.y + 1, 0, 0, wcName, 0);
			m_hdc.SetTextColor(m_crText);
			m_hdc.ExtTextOut(sp.x, sp.y, 0, 0, wcName, 0);
		}
	}
}

void CGDIPainter::PaintStatusIcon(int iIcon)
{
	if (m_mapIcons.find(iIcon) != m_mapIcons.end())
	{
		m_hdc.drawIcon(m_srWindow.right - 32, m_iCurrentStatusIcon, m_mapIcons[iIcon]);
		m_iCurrentStatusIcon += 32;
	}
}

void CGDIPainter::ParseString(const char * buff, const std::wstring & wstrBase)
{
	std::vector<long> vRecord;
	wstring wstrRecord;
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
			wchar_t buffer[1000];
			buffer[MultiByteToWideChar(CP_ACP, 0, pos, newpos - pos, buffer, 1000)] = 0;
			wstrRecord = buffer;
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
					pt.m_hPen = CreatePen(PS_SOLID, 1, RGB(vRecord[2], vRecord[3], vRecord[4]));
					pt.m_hBrush = CreateSolidBrush(RGB(vRecord[2], vRecord[3], vRecord[4]));
				}
				if (vRecord.size() == 10)
				{
					PolygonTools & pt = m_PolygonTools[vRecord[1]];
					pt.m_hPen = CreatePen(vRecord[8] * PS_DOT, vRecord[9], RGB(vRecord[5], vRecord[6], vRecord[7]));
					pt.m_hBrush = CreateSolidBrush(RGB(vRecord[2], vRecord[3], vRecord[4]));
				}
				break;
			}
		case maskPoints:
			{
				if (vRecord.size() == 4 && wstrRecord != L"")
				{
					PointTools & pt = m_PointTools[vRecord[1]];
					pt.m_hIcon = (HICON)LoadImage(m_hResourceInst, wstrRecord.c_str(), IMAGE_ICON, 32, 32, 0);
					if (!pt.m_hIcon)
					{
						int i = wcstol(wstrRecord.c_str(), 0, 10);
						if (i)
						pt.m_hIcon = (HICON)LoadImage(m_hResourceInst, MAKEINTRESOURCE(i), IMAGE_ICON, 32, 32, 0);
					}
					pt.m_iDiffX = vRecord[2];
					pt.m_iDiffY = vRecord[3];
				}
				if (vRecord.size() == 2 && wstrRecord != L"")
				{
					PointTools & pt = m_PointTools[vRecord[1]];
					pt.m_hIcon = (HICON)LoadImage(m_hResourceInst, wstrRecord.c_str(), IMAGE_ICON, 32, 32, 0);
					if (!pt.m_hIcon)
					{
						int i = wcstol(wstrRecord.c_str(), 0, 10);
						if (i)
						pt.m_hIcon = (HICON)LoadImage(m_hResourceInst, MAKEINTRESOURCE(i), IMAGE_ICON, 32, 32, 0);
					}
					pt.m_iDiffX = 0;
					pt.m_iDiffY = 0;
				}
				break;
			}
		case maskPolylines:
			{
				if (vRecord.size() == 7)
				{
					m_PolylinePens[vRecord[1]].first = CreatePen(vRecord[5] * PS_DOT, vRecord[6], RGB(vRecord[2], vRecord[3], vRecord[4]));
				}
				if (vRecord.size() == 12)
				{
					m_PolylinePens[vRecord[1]].first = CreatePen(vRecord[5] * PS_DOT, vRecord[6], RGB(vRecord[2], vRecord[3], vRecord[4]));
					m_PolylinePens[vRecord[1]].second = CreatePen(vRecord[10] * PS_DOT, vRecord[11], RGB(vRecord[7], vRecord[8], vRecord[9]));
				}
				break;
			}
		case 0:
			{
				if (vRecord.size() == 4)
				{
					m_crBg = RGB(vRecord[1], vRecord[2], vRecord[3]);
					m_hBgBrush = CreateSolidBrush(m_crBg);	
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
					m_hCursorPen = CreatePen(PS_SOLID, 1, RGB(vRecord[1], vRecord[2], vRecord[3]));
					m_hCursorBrush = CreateSolidBrush(RGB(vRecord[1], vRecord[2], vRecord[3]));
				}
				break;
			}
		case 3:
			{
				if (vRecord.size() == 4)
					m_FontCache.SetTemplate(vRecord[1], vRecord[2], vRecord[3] != 0);
				break;
			}
		}
	}
	else if (!wstrRecord.empty())
	{
		m_hResourceInst = LoadLibraryEx(MakeFilename(wstrRecord, wstrBase).c_str(), 0, LOAD_LIBRARY_AS_DATAFILE);
	}
}

void CGDIPainter::InitToolsCommon()
{
	// Create default tools
	m_hDefaultPen = CreatePen(PS_DOT, 1, RGB(0xff, 0x0, 0x0));
	m_hDefaultBrush = CreateSolidBrush(RGB(0xff, 0x0, 0x0));
	// Prepare font structures

	m_mapIcons[1] = (HICON)LoadImage(m_hResourceInst, L"satelliteno", IMAGE_ICON, 32, 32, 0);
	m_mapIcons[2] = (HICON)LoadImage(m_hResourceInst, L"satellitenofix", IMAGE_ICON, 32, 32, 0);
	m_mapIcons[3] = (HICON)LoadImage(m_hResourceInst, L"satelliteyes", IMAGE_ICON, 32, 32, 0);
	m_mapIcons[4] = (HICON)LoadImage(m_hResourceInst, L"satellitewait", IMAGE_ICON, 32, 32, 0);
	m_mapIcons[5] = (HICON)LoadImage(m_hResourceInst, L"satellitedisabled", IMAGE_ICON, 32, 32, 0);
}

GeoRect CGDIPainter::GetRect() 
{
	return ScreenToGeo(m_srWindow);
}

void CGDIPainter::DrawTextMonitor(const wchar_t * wcLabel, const wchar_t * wcText)
{
	ScreenRect &sr = m_srCurrentMonitor;
	int iHeight = m_srCurrentMonitor.bottom - m_srCurrentMonitor.top;
	int iTitle = iHeight / 3;
	int iValue = iHeight - iTitle;

	SIZE sizeCurr;

	HFONT hMonitorTitleFont = m_FontCache.GetFont(iTitle, true, 0);
	HFONT hMonitorValueFont = m_FontCache.GetFont(iValue, true, 0);

	m_hdc.SetTextColor(m_crText);

	m_hdc.SelectObject(hMonitorTitleFont);
	m_hdc.getTextExtentPoint(wcLabel, &sizeCurr);
	m_hdc.ExtTextOut(sr.Center().x - sizeCurr.cx / 2, sr.top, ETO_CLIPPED, &sr, wcLabel, 0);
	sr.top += sizeCurr.cy;

	m_hdc.SelectObject(hMonitorValueFont);
	m_hdc.getTextExtentPoint(wcText, &sizeCurr);
	m_hdc.ExtTextOut(sr.Center().x - sizeCurr.cx / 2, sr.top, ETO_CLIPPED, &sr, wcText, 0);
	sr.top += sizeCurr.cy;

}
void CGDIPainter::DrawMonitorLabel(const wchar_t * wcLabel)
{
	ScreenRect &sr = m_srCurrentMonitor;
	int iHeight = m_srCurrentMonitor.bottom - m_srCurrentMonitor.top;
	int iTitle = iHeight / 3;

	SIZE sizeCurr;

	HFONT hMonitorTitleFont = m_FontCache.GetFont(iTitle, true, 0);

	m_hdc.SetTextColor(m_crText);
	m_hdc.SelectObject(hMonitorTitleFont);
	m_hdc.getTextExtentPoint(wcLabel, &sizeCurr);
	m_hdc.ExtTextOut(sr.Center().x - sizeCurr.cx / 2, sr.top, ETO_CLIPPED, &sr, wcLabel, 0);
	sr.top += sizeCurr.cy;

}
ScreenPoint CGDIPainter::GetMonitorSize()
{
	ScreenRect &sr = m_srCurrentMonitor;
	return ScreenPoint(sr.right - sr.left, sr.bottom - sr.top);
}
void CGDIPainter::DrawBar(const ScreenRect & srBar)
{
	m_hdc.SelectObject(m_hDefaultBrush);
	m_hdc.SelectObject(m_hDefaultPen);
	ScreenRect &sr = m_srCurrentMonitor;
	m_hdc.Rectangle(srBar.left + sr.left, srBar.top + sr.top, srBar.right + sr.left, srBar.bottom + sr.top);
}

ScreenRect CGDIPainter::GetMonitorsBar()
{
	ScreenRect res;
	if (m_fVertical)
	{
		res.Init(ScreenPoint(m_srWindow.left, m_srWindow.bottom));
		res.Append(ScreenPoint(m_srWindow.right, m_srWindow.bottom + m_iBottomBar));
	}
	else
	{
		res.Init(ScreenPoint(m_srWindow.right, m_srWindow.top));
		res.Append(ScreenPoint(m_srWindow.right + m_iBottomBar, m_srWindow.bottom));
	}
	return res;
}
void CGDIPainter::SetCurrentMonitor(const ScreenRect & srRect, bool fActive)
{
	m_srCurrentMonitor = srRect;
	m_hdc.SelectObject(m_hBgBrush);
	m_hdc.SelectObject(m_hCursorPen);
	m_hdc.Rectangle(srRect.left, srRect.top, srRect.right + 1, srRect.bottom + 1);
	if (true == fActive)
		m_hdc.Rectangle(srRect.left + 1, srRect.top + 1, srRect.right, srRect.bottom);
	if (true == fActive)
		m_srActiveMonitor = srRect;
}

CGDIPainter::~CGDIPainter()
{
}

void CGDIPainter::PaintLowMemory(const wchar_t * wcString1, const wchar_t * wcString2)
{
	HFONT hMonitorValueFont = m_FontCache.GetFont(40, true, 0);

	m_hdc.SelectObject(hMonitorValueFont);
	m_hdc.SetTextColor(RGB(0x7f, 0x3f, 0x3f));

	SIZE labelSize;
	
	m_hdc.getTextExtentPoint(wcString1, &labelSize);
	m_hdc.ExtTextOut(m_spWindowCenter.x - labelSize.cx / 2, m_spWindowCenter.y - labelSize.cy, 0, 0, wcString1, 0);

	m_hdc.getTextExtentPoint(wcString2, &labelSize);
	m_hdc.ExtTextOut(m_spWindowCenter.x - labelSize.cx / 2, m_spWindowCenter.y, 0, 0, wcString2, 0);

}

void CGDIPainter::PaintStatusLine(const wchar_t * wcName)
{
	if (!wcName || !*wcName)
		return;
	HFONT hFont = m_FontCache.GetFont(4, 0);
	m_hdc.SelectObject(hFont);
	m_hdc.SetTextColor(m_crBg);
	m_hdc.ExtTextOut(4 + 1, m_iStatusLineOffset + 4, 0, 0, wcName, 0);
	m_hdc.ExtTextOut(4 - 1, m_iStatusLineOffset + 4, 0, 0, wcName, 0);
	m_hdc.ExtTextOut(4, m_iStatusLineOffset + 4 + 1, 0, 0, wcName, 0);
	m_hdc.ExtTextOut(4, m_iStatusLineOffset + 4 - 1, 0, 0, wcName, 0);
	m_hdc.SetTextColor(m_crText);
	m_hdc.ExtTextOut(4, m_iStatusLineOffset + 4, 0, 0, wcName, 0);
	SIZE size;
	m_hdc.getTextExtentPoint(wcName, &size);
	m_iStatusLineOffset += size.cy + 4;
}

void CGDIPainter::SetFullScreen(bool fFull)
{
	m_fFullScreen = fFull;
#if defined(UNDER_CE)
	RECT rectWin;
	RECT rectCB;
	if (fFull)
	{
		GetWindowRect(g_hwndCB, &rectCB);
		GetWindowRect(m_hWnd, &rectWin);
		m_rectLastWinSize = rectWin;
		m_rectLastCBSize = rectCB;
		rectWin.top = 0;
		rectWin.bottom = rectCB.bottom;
		rectCB.top = rectCB.bottom;
		SetForegroundWindow(m_hWnd);
		SHFullScreen(m_hWnd, SHFS_HIDETASKBAR | SHFS_HIDESIPBUTTON | SHFS_HIDESTARTICON);
#ifdef SMARTPHONE
		HWND hwndTray = FindWindow(L"Tray", 0);
		if (hwndTray)
			ShowWindow(hwndTray, SW_HIDE);
#endif // SMARTPHONE
	}
	else
	{
		SHFullScreen(m_hWnd, SHFS_SHOWTASKBAR | SHFS_SHOWSIPBUTTON | SHFS_SHOWSTARTICON);
		rectWin = m_rectLastWinSize;
		rectCB = m_rectLastCBSize;
#ifdef SMARTPHONE
		HWND hwndTray = FindWindow(L"Tray", 0);
		if (hwndTray)
			ShowWindow(hwndTray, SW_SHOW);
#endif // SMARTPHONE
	}
	MoveWindow(m_hWnd, rectWin.left, rectWin.top, rectWin.right - rectWin.left, rectWin.bottom - rectWin.top, TRUE);
	MoveWindow(g_hwndCB, rectCB.left, rectCB.top, rectCB.right - rectCB.left, rectCB.bottom - rectCB.top, TRUE);
#else
	::ShowWindow(m_hWnd, m_fFullScreen ? SW_MAXIMIZE : SW_RESTORE);
#endif
}

GeoPoint CGDIPainter::ScreenToGeo(const ScreenPoint & pt)
{
	AutoLock l;
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

	res.lon = dx2 * m_ruiScale10() / 10 * 100 / m_lXScale100 + m_gpCenter().lon;
	res.lat = - dy2 * m_ruiScale10() / 10 + m_gpCenter().lat;
	return res;
}
ScreenPoint CGDIPainter::GeoToScreen(const GeoPoint & pt)
{
	AutoLock l;
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
	return res;
}
ScreenRect CGDIPainter::GeoToScreen(const GeoRect & rect)
{
	ScreenRect res;
	res.Init(GeoToScreen(GeoPoint(rect.minLon, rect.minLat)));
	res.Append(GeoToScreen(GeoPoint(rect.maxLon, rect.maxLat)));
	res.Append(GeoToScreen(GeoPoint(rect.minLon, rect.maxLat)));
	res.Append(GeoToScreen(GeoPoint(rect.maxLon, rect.minLat)));
	return res;
}
GeoRect CGDIPainter::ScreenToGeo(const ScreenRect & rect)
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
const GeoPoint CGDIPainter::GetCenter() 
{
	return m_gpCenter();
}
void CGDIPainter::PaintScale()
{
	static double lengths0[] = {10, 20, 50, 100, 200, 500, 1000, 2000, 5000, 10000, 20000, 50000, 100000, 200000, 500000, 1000000, 2000000, 5000000, 10000000, 0};
	static double lengths1[] = {10 * cdYard, 20 * cdYard, 50 * cdYard, 100 * cdYard, 200 * cdYard, 500 * cdYard, 500 * cdNauticalMile, 1000 * cdNauticalMile, 2000 * cdNauticalMile, 5000 * cdNauticalMile, 10000 * cdNauticalMile, 20000 * cdNauticalMile, 50000 * cdNauticalMile, 100000 * cdNauticalMile, 200000 * cdNauticalMile, 500000 * cdNauticalMile, 1000000 * cdNauticalMile, 2000000 * cdNauticalMile, 5000000 * cdNauticalMile, 10000000 * cdNauticalMile};
	static double lengths2[] = {10 * cdYard, 20 * cdYard, 50 * cdYard, 100 * cdYard, 200 * cdYard, 500 * cdYard, 500 * cdLandMile, 1000 * cdLandMile, 2000 * cdLandMile, 5000 * cdLandMile, 10000 * cdLandMile, 20000 * cdLandMile, 50000 * cdLandMile, 100000 * cdLandMile, 200000 * cdLandMile, 500000 * cdLandMile, 1000000 * cdLandMile, 2000000 * cdLandMile, 5000000 * cdLandMile, 10000000 * cdLandMile};
	static double *lengths[] = {lengths0, lengths1, lengths2};
	ScreenRect sr = GetScreenRect();
	ScreenPoint start = ScreenPoint(sr.left, sr.bottom - 1);
	double dScale = double(IntDistance(m_gpCenter(), ScreenToGeo(GeoToScreen(m_gpCenter()) + ScreenDiff(2000,0)))) / 2000;
	int type = 0x202;
	for (double * l = lengths[app.m_riMetrics()]; *l; ++l)
	{
		ScreenPoint finish(start);
		int length = int(start.x + *l / dScale);
		finish.x = length;
		if (finish.x > sr.right)
			break;
		const wstring & label = DistanceToText(*l);
		StartPolyline(type, label.c_str());
		AddPoint(start);
		AddPoint(finish);
		FinishObject();
		start = finish;
		type = 0x405 - type;
	}
	return;
	
	int iWidth = 40;
	if (app.m_Options[mcoLargeFonts])
		iWidth *= 2;
	double dDist = IntDistance(m_gpCenter(), ScreenToGeo(GeoToScreen(m_gpCenter()) + ScreenDiff(2000,0))) / 50;
	ScreenPoint pt1 = ScreenPoint(m_srWindow.left, m_srWindow.bottom) + ScreenDiff(10, -10);
	ScreenPoint pt2 = pt1 + ScreenDiff(iWidth,0);
	wstring wcScale = DistanceToText(dDist);
	StartPolyline(0xFD, wcScale.c_str());
	SetLabelMandatory();
	AddPoint(pt1 + ScreenDiff(0, -2));
	AddPoint(pt1);
	AddPoint(pt2);
	AddPoint(pt2 + ScreenDiff(0,-2));
	FinishObject();
}

void CGDIPainter::AddButton(const wchar_t * wcLabel, int iCommand, bool fSelected)
{
	HFONT f = m_FontCache.GetFont(5, 0);
	ScreenSize size;
	m_hdc.SelectObject(f);
	m_hdc.getTextExtentPoint(wcLabel, &size);
	m_iCurrentButton -= size.cy + 6;
	m_hdc.SelectObject(m_hBgBrush);
	m_hdc.SelectObject(m_hCursorPen);
	m_hdc.RoundRect(m_srWindow.right - size.cx - 5, m_iCurrentButton + 1, m_srWindow.right - 1, m_iCurrentButton + size.cy + 5, 8, 8);
	m_hdc.SetTextColor(fSelected ? RGB(127, 127, 127) : m_crText);
	m_hdc.SetBkMode(0);
	m_hdc.ExtTextOut(m_srWindow.right - size.cx - 3, m_iCurrentButton + 3, 0, 0, wcLabel, 0);

	m_buttons.push_back(
		make_pair(
			ScreenRect(ScreenPoint(m_srWindow.right - size.cx - 3, m_iCurrentButton + 3), size),
			iCommand
		));
}

void CGDIPainter::ClearButtons()
{
	m_buttons.clear();
}

int CGDIPainter::CheckButton(const ScreenPoint & sp)
{
	for (Buttons::iterator it = m_buttons.begin(); it != m_buttons.end(); ++it)
	{
		if (it->first.Side(sp) == 0)
			return it->second;
	}
	return -1;
}

void CGDIPainter::GetUnknownTypes(IListAcceptor * pAcceptor)
{
	for (set<int>::iterator it = m_setUnknownTypes.begin(); 
			it != m_setUnknownTypes.end(); ++it)
	{
		wchar_t wstrType[100];
		swprintf(wstrType, 100, L"%d, 0x%04x, %s", *it, *it, app.m_TypeInfo.PointType(*it).c_str());
		pAcceptor->AddItem(wstrType, *it);
	}
}

void CGDIPainter::PaintCompass()
{
	ScreenPoint spCenter(m_srWindow.right - 16, m_iCurrentStatusIcon + 16);
	ScreenPoint spNorth = spCenter + ScreenDiff(- m_sin100 / 7, - m_cos100 / 7);
	ScreenPoint spEast = spCenter + ScreenDiff(m_cos100 / 14, - m_sin100 / 14);
	ScreenPoint spSouth = spCenter + ScreenDiff(m_sin100 / 7, m_cos100 / 7);
	ScreenPoint spWest = spCenter + ScreenDiff(- m_cos100 / 14, m_sin100 / 14);
	m_pointList.resize(0);
	m_pointList.push_back(spEast);
	m_pointList.push_back(spWest);
	m_pointList.push_back(spNorth);
	m_hdc.SelectObject(m_PolygonTools[0x100].m_hPen);
	m_hdc.SelectObject(m_PolygonTools[0x100].m_hBrush);
	m_hdc.Polygon(&m_pointList[0], 3);
	m_pointList.back() = spSouth;
	m_hdc.SelectObject(m_PolygonTools[0x101].m_hPen);
	m_hdc.SelectObject(m_PolygonTools[0x101].m_hBrush);
	m_hdc.Polygon(&m_pointList[0], 3);
	m_iCurrentStatusIcon += 32;
}

double CGDIPainter::GetXScale() 
{ 
	return double(m_uiScale10Cache) / m_lXScale100 * 10;
}

void CGDIPainter::SetXScale(double scale)
{
	AutoLock l;
	unsigned int new_scale = (unsigned int)(scale / 10 * m_lXScale100);
	new_scale = max(ciMinZoom, min(ciMaxZoom, new_scale));
	if (m_ruiScale10() != new_scale)
		m_ruiScale10.Set(new_scale);
}
