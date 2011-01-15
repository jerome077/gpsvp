/*
Copyright (c) 2005-2008, Vsevolod E. Shorin and others
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "GMCommon.h"
#include <math.h>
#include "../Common.h"

// ---------------------------------------------------------------

// Calculation derived from here: http://www.supware.net/GoogleMapping/ 
double GoogleXZ17toLong(unsigned long x, unsigned char zoom17)
{
    return 360.0*x/(1<<(17-zoom17))-180;
}

double GoogleYZ17toLat(unsigned long y, unsigned char zoom17)
{
	return (atan(exp(pi*(1.0-256.0*y/(1<<(24-zoom17)))))/pi-0.25)*360;
}

// ---------------------------------------------------------------

// Maps like Google Maps use the WGS84 coordinates and project it on the 
// map as it they where spherical coordinates. But WGS84 is based on a ellipsoid!
// The values for the latitude are not the same on a sphere or on an ellipsoid
// (latitude is the angle between the equatorial plane and the normal vector of
//  a plane tangent to the earth surface). This funtions convert a WGS84-latitude
// into a spherical latitude.
//
// Calculation derived from: http://www.geodyssey.com/papers/ncsphere.html
//
// esq = ((a * a) - (b * b)) / (a * a)
// b = a (1−f)
// a = 6378137 m
// f = 1/298.257223563
// => 1-esp = 0.99330562000985868300386276645996
const double OneMinusEsp = 0.99330562000985868300386276645996;

double WGS84LatToSphericalLat(double dWGS84Lat)
{
	return atan(OneMinusEsp*tan(dWGS84Lat*pi/180))*180/pi;
}

// ---------------------------------------------------------------

double LongToXSphericalMercator(double dLong)
{
	return 6378137.0 * pi / 180 * dLong;
}

double LatToYSphericalMercator (double dLat)
{
	return 6378137.0 * log(tan(pi/180*(45+dLat/2.0)));
}

// ---------------------------------------------------------------

// ... a quadtree reference like for Google satellite:
std::string GoogleXYZ1toQRST(unsigned long x, unsigned long y, unsigned char zoom1, int maxCharCount, int firstChar)
{
	long NumX = x;
	long NumY = y;
	long d = 1 << (zoom1 - 1);

	if ((NumX < 0) || (NumX > (d-1))) {
		NumX = NumX % d;
		if (NumX < 0) {
			NumX += d;
		}
	}

	char buf[24];
	buf[0] = 't';

	for (long nPos = 1; nPos < zoom1; nPos++) {
	    d >>= 1;
		if (NumY < d) {
			if (NumX < d) {
				buf[nPos] = 'q';
			} else {
				buf[nPos] = 'r';
				NumX -= d;
			}
		} else {
			if (NumX < d) {
				buf[nPos] = 't';
			} else { 
				buf[nPos] = 's';
				NumX -= d;
			}
			NumY -= d;
		}
	}
	buf[zoom1] = '\0';
	if ((firstChar > 0) || (maxCharCount > 0))
	{
		std::string strBuf = buf;
		if ((size_t) firstChar >= strBuf.length())
			return "";
		else
			return strBuf.substr(firstChar, maxCharCount);
	}
	else
		return buf;
}

// ---------------------------------------------------------------

std::wstring GoogleXYZ1toQRSTW(unsigned long x, unsigned long y, unsigned char zoom1, int maxCharCount, int firstChar)
{
	long NumX = x;
	long NumY = y;
	long d = 1 << (zoom1 - 1);

	if ((NumX < 0) || (NumX > (d-1))) {
		NumX = NumX % d;
		if (NumX < 0) {
			NumX += d;
		}
	}

	wchar_t buf[24];
	buf[0] = L't';

	for (long nPos = 1; nPos < zoom1; nPos++) {
	    d >>= 1;
		if (NumY < d) {
			if (NumX < d) {
				buf[nPos] = L'q';
			} else {
				buf[nPos] = L'r';
				NumX -= d;
			}
		} else {
			if (NumX < d) {
				buf[nPos] = L't';
			} else { 
				buf[nPos] = L's';
				NumX -= d;
			}
			NumY -= d;
		}
	}
	buf[zoom1] = L'\0';
	if ((firstChar > 0) || (maxCharCount > 0))
	{
		std::wstring strBuf = buf;
		if ((size_t) firstChar >= strBuf.length())
			return L"";
		else
			return strBuf.substr(firstChar, maxCharCount);
	}
	else
		return buf;
}

// ---------------------------------------------------------------

// ... a quadtree reference like for Live maps:
std::string GoogleXYZ1toQKey(unsigned long x, unsigned long y, unsigned char zoom1, int maxCharCount, int firstChar)
{
	long NumX = x;
	long NumY = y;
	long d = 1 << (zoom1 - 1);

	if ((NumX < 0) || (NumX > (d-1))) {
		NumX = NumX % d;
		if (NumX < 0) {
			NumX += d;
		}
	}

	char buf[24];

	for (long nPos = 0; nPos < (zoom1-1); nPos++) {
	    d >>= 1;
		if (NumY < d) {
			if (NumX < d) {
				buf[nPos] = '0';
			} else {
				buf[nPos] = '1';
				NumX -= d;
			}
		} else {
			if (NumX < d) {
				buf[nPos] = '2';
			} else { 
				buf[nPos] = '3';
				NumX -= d;
			}
			NumY -= d;
		}
	}
	buf[zoom1-1] = '\0';
	if ((firstChar > 0) || (maxCharCount > 0))
	{
		std::string strBuf = buf;
		if ((size_t) firstChar >= strBuf.length())
			return "";
		else
			return strBuf.substr(firstChar, maxCharCount);
	}
	else
		return buf;
}

// ---------------------------------------------------------------

std::wstring GoogleXYZ1toQKeyW(unsigned long x, unsigned long y, unsigned char zoom1, int maxCharCount, int firstChar)
{
	long NumX = x;
	long NumY = y;
	long d = 1 << (zoom1 - 1);

	if ((NumX < 0) || (NumX > (d-1))) {
		NumX = NumX % d;
		if (NumX < 0) {
			NumX += d;
		}
	}

	wchar_t buf[24];

	for (long nPos = 0; nPos < (zoom1-1); nPos++) {
	    d >>= 1;
		if (NumY < d) {
			if (NumX < d) {
				buf[nPos] = L'0';
			} else {
				buf[nPos] = L'1';
				NumX -= d;
			}
		} else {
			if (NumX < d) {
				buf[nPos] = L'2';
			} else { 
				buf[nPos] = L'3';
				NumX -= d;
			}
			NumY -= d;
		}
	}
	buf[zoom1-1] = L'\0';
	if ((firstChar > 0) || (maxCharCount > 0))
	{
		std::wstring strBuf = buf;
		if ((size_t) firstChar >= strBuf.length())
			return L"";
		else
			return strBuf.substr(firstChar, maxCharCount);
	}
	else
		return buf;
}

// ---------------------------------------------------------------
