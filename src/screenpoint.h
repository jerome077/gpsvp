#ifndef SCREENPOINT_H
#define SCREENPOINT_H

#include <windows.h>
#include "PlatformDef.h"

//! Type for screen point position
struct ScreenPoint : public POINT
{
	//! Construct from coordinates
	ScreenPoint(Int iX, Int iY) {x = iX; y = iY;}
	ScreenPoint(){}
	bool operator == (const ScreenPoint & pt)
	{
		return (x == pt.x) && (y == pt.y);
	}
};

struct ScreenSize : public SIZE
{
	ScreenSize() {}
	ScreenSize(long x, long y) { cx = x; cy = y; }
};

struct ScreenRect : public RECT
{
	ScreenRect(){};
	ScreenRect(const RECT & rc){ *(RECT*)this = rc;};
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

class ScreenRectSet
{
private:
	struct Data;
	Data * m_data;
public:
	ScreenRectSet();
	~ScreenRectSet();
	bool Fit(const ScreenRect & sr);
	void Reset();
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

#endif // SCREENPOINT_H
