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
		if (firstChar >= strBuf.length())
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
		if (firstChar >= strBuf.length())
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
		if (firstChar >= strBuf.length())
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
		if (firstChar >= strBuf.length())
			return L"";
		else
			return strBuf.substr(firstChar, maxCharCount);
	}
	else
		return buf;
}

// ---------------------------------------------------------------

// Test-function (Jerome: currently not use, I might remove it later) to see if it 
// would be possible to display/enter UTM coordinates. The function works, but
// I'm thinking about using the gdal DLL, which supports a lot of coordanites systems.
// Calculation based on explaination from Steven Dutch on http://www.uwgb.edu/dutchs/UsefulData/UTMFormulas.HTM
void LongLatToUTM(double lon360, double lat360, double& utmX, double& utmY, int& utmZone)
{
    utmZone = floor(lon360/6.0) + 31;
	double lon0_360 = utmZone*6.0 - 183.0;
	
	double lon628 = lon360 * pi / 180.0;
    double lat628 = lat360 * pi / 180.0;
	double cosLat = cos(lat628);
	double sinLat = sin(lat628);
	double tanLat = tan(lat628);
	double cosLat_2 = cosLat*cosLat;
	double cosLat_3 = cosLat*cosLat_2;
	double cosLat_4 = cosLat_2*cosLat_2;
	double sinLat_2 = sinLat*sinLat;
	double tanLat_2 = tanLat*tanLat;

	double a = 6378137;
	double b = 6356752.3142;
    double SinOneSecond = 4.8481368e-6;
	double SinOneSecond_2 = SinOneSecond*SinOneSecond;
	double SinOneSecond_3 = SinOneSecond*SinOneSecond_2;
	double SinOneSecond_4 = SinOneSecond_2*SinOneSecond_2;
	double k0 = 0.9996;
	double e = sqrt(1-b*b/(a*a));
	double e_2 = e*e;
	double ei2 = e_2/(1-e_2);
	double n = (a-b)/(a+b);
	double n_2 = n*n;
	double n_3 = n*n_2;
	double n_4 = n_2*n_2;
	double n_5 = n*n_4;
	double nu = a / sqrt(1 - e_2*sinLat_2);
	double p = (lon360-lon0_360)*3600/10000; // it's not explained, why it should in seconds and divided by 10000
	double p_2 = p*p;

	double Ai = a * (1.0 - n + (5.0/4.0)*(n_2 - n_3) + (81.0/64.0)*(n_4 - n_5));
	double Bi = (1.5*a*n)*(1.0 - n + (7.0/8.0)*(n_2 - n_3) + (55.0/64.0)*(n_4 - n_5));
	double Ci = (15.0*a*n_2/16.0)*(1.0 - n + 0.75*(n_2 - n_3));
	double Di = (35.0*a*n_3/48.0)*(1.0 - n + (11.0/16.0)*(n_2 - n_3));
	double Ei = (315.0*a*n_4/51.0)*(1 - n);
	double S = Ai*lat628 - Bi*sin(2*lat628) + Ci*sin(4*lat628) - Di*sin(6*lat628) + Ei*sin(8*lat628);

	double K1 = S * k0;
	double K2 = 1e8* k0 * SinOneSecond_2 * nu * sinLat * cosLat * 0.5;
	double K3 = 1e16* (k0 * SinOneSecond_4 * nu * sinLat * cosLat_3 / 24.0)
               *(5 - tanLat_2 + 9.0 * ei2 * cosLat_2 + 4.0 * ei2*ei2 * cosLat_4);
	double K4 = 1e4* k0 * SinOneSecond * nu * cosLat;
	double K5 = 1e12* (k0 * SinOneSecond_3 * nu * cosLat_3 / 6.0) * (1 - tanLat_2 + ei2 * cosLat_2);

    // Northing
	utmY = K1 + K2*p_2 + K3*p_2*p_2;	
	// Conventional UTM easting
    utmX = K4*p + K5*p*p_2 + 500000;
}

// ---------------------------------------------------------------

