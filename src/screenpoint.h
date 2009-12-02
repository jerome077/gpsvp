/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef SCREENPOINT_H
#define SCREENPOINT_H

#ifndef LINUX
#	include <windows.h>
#endif
#include "PlatformDef.h"
#include <algorithm>

#ifndef LINUX
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
	void Trim(const ScreenRect & r)
	{
		if (right > r.right)
			right = (std::max)(left, r.right);
		if (left < r.left)
			left = (std::min)(right, r.left);
		if (bottom > r.bottom)
			bottom = (std::max)(top, r.bottom);
		if (top < r.top)
			top = (std::min)(bottom, r.top);
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

#else // LINUX
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

#endif // LINUX

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
