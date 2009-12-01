#ifndef gtkpainter_h
#define gtkpainter_h

#include "GeoPoint.h"
#include "RegValues.h"
#include "Monitors.h"

typedef void* HWND;


struct ScreenPoint
{
	ScreenPoint(int x_, int y_) : x(x_), y(y_) {}
	ScreenPoint() : x(0), y(0) {}
	int x, y;
};

struct ScreenSize
{
	int cx,cy;
	ScreenSize() {}
	ScreenSize(long x, long y) { cx = x; cy = y; }
};

struct ScreenRect
{
	int left, right, top, bottom;
	ScreenRect(){};
	// ScreenRect(const RECT & rc){ *(RECT*)this = rc;};
	ScreenRect(const ScreenPoint & sp, const ScreenSize & ss)
	{
		left = sp.x;
		top = sp.y;
		right = sp.x + ss.cx;
		bottom = sp.y + ss.cy;
	}
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

struct ScreenDiff
{
	ScreenDiff(const ScreenDiff & d) { dx = d.dx; dy = d.dy; }
	ScreenDiff(int x, int y) : dx(x), dy(y) {}
	bool Null() {return dx == 0.0 && dy == 0.0;}
	int dx;
	int dy;
	void operator *=(int i) {dx*=i; dy*=i;}
	void operator /=(int i) {dx/=i; dy/=i;}
};

inline ScreenPoint & operator -= (ScreenPoint & pt, const ScreenDiff & d)
{
	pt.x -= d.dx;
	pt.y -= d.dy;
	return pt;
}

inline ScreenPoint operator - (ScreenPoint pt, const ScreenDiff & d)
{
	return pt -= d;
}

inline ScreenPoint & operator += (ScreenPoint & pt, const ScreenDiff & d)
{
	pt.x += d.dx;
	pt.y += d.dy;
	return pt;
}

inline ScreenPoint operator + (ScreenPoint pt, const ScreenDiff & d)
{
	return pt += d;
}

inline ScreenDiff operator - (ScreenPoint a, ScreenPoint b)
{
	return ScreenDiff(a.x - b.x, a.y - b.y);
}

class CGTKPainter : public IMonitorPainter
{
public:
	void Init(HKEY, HWND);
    void Redraw();
    const GeoPoint GetCenter();
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
	void OnTimer();
	void RedrawMonitors();
	void SetShowMonitors(bool fShow);
	bool IsFullScreen();
	void SetFullScreen(bool fFull);
	void ResetManualMode();
	bool ManualMode();
	int GetScale();
	void PrepareScales();
	double GetXScale();
	bool IsVertical();
	void BeginPaint();

	//IMonitorPainter
	void DrawTextMonitor(const tchar_t * wcLabel, const tchar_t * wcText);
	void DrawMonitorLabel(const tchar_t * wcLabel);
	const ScreenPoint& GetMonitorSize();
	void DrawBar(const ScreenRect & srBar);
	void SetCurrentMonitor(const ScreenRect & srRect, bool fActive);
};

#endif // gtkpainter_h
