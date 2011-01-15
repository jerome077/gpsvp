/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef COMMON_H
#define COMMON_H

#include "PlatformDef.h"
#include <ctype.h>
#include <string>

//! Translate to double degrees for output
double Degree(Int iDegree);
//! Translate to Int degrees for internal usage
Int FromDegree(double dDegree);
//! Check condition and throw exception if false
void Check(bool fCondition);
//! Make double from std::string
double myatof(const char * str);
double myatof(const wchar_t * str);

/*
	//! Template implementation for swapping values
	template<class T>
	void swap(T &a, T &b)
	{
		T c = a;
		a = b;
		b = c;
	}
*/

//! Template function for square
template<class T>
T sqr(T a)
{
	return a * a;
}

std::wstring DoubleToText(double dDouble, int iDigits = 1);
std::string DoubleToStr(double dDouble, int iDigits = 1);

std::wstring DegreeToText(double dDegree, bool fLat, int iCoordFormat);
double TextToDegree(const wchar_t * wcText);
// Some coordinates must know WGS84-longitude and latitude at the same time.
void CoordToText(double dLon, double dLat, std::wstring& wstrLon, std::wstring& wstrLat);
void TextToCoord(const std::wstring& wstrLon, const std::wstring& wstrLat, double& dLon, double& dLat);
std::wstring CoordLabelLon();
std::wstring CoordLabelLat();

std::wstring IntToText(int iInt);
std::wstring IntToHex(int iInt);
std::wstring DistanceToText(double dDistance);
std::wstring HeightToText(double dDistance);
std::wstring SpeedToText(double dDistance);
std::wstring MemoryToText(unsigned long ulMemory);
inline std::wstring a2w(const char * s)
{
	wchar_t buff[1000];
	swprintf(buff, 1000, L"%S", s);
	return std::wstring(buff);
}

struct IListAcceptor
{
	virtual void AddItem(const wchar_t * wcLabel, int iId) = 0;
	virtual void UpdateSelected(const wchar_t * wcLabel) = 0;
};

struct IListAcceptor2
{
	virtual int AddItem(const wchar_t * wcLabel, int iId, int iSubItem, int lParam) = 0;
	virtual void UpdateCurrent(const wchar_t * wcLabel, int iSubItem) = 0;
	virtual void UpdateSelected(const wchar_t * wcLabel, int iSubItem) = 0;
};

struct IListAcceptor2Acceptor
{
	virtual void UpdateCurrent(int iId, IListAcceptor2 * pAcceptor) = 0;
};

enum enumObjTypes
{
	maskPoints = 0x10,
	maskIndexedPoints = 0x20,
	maskPolylines = 0x40,
	maskPolygons = 0x80
};

template <class T>
T abs(T val)
{
	return (val > T(0)) ? val : -val;
}

inline int int_sqrt(int input)
{
	if (input < 0)
		return 0;
	int nv, v = input>>1, c = 0;
	if (!v)
		return input;
	do
	{
		nv = (v + input/v)>>1;
		if (abs(v - nv) <= 1)
			return nv;
		v = nv;
	}
	while (c++ < 25);
	return nv;
}

int cos100(int degree);  // degree value must be in [-pi..pi]
int sin100(int degree);  // degree value must be in [-pi/2..3pi/2]
static const double cdNauticalMile = 1.852;
static const double cdLandMile = 1.609344;
static const double cdFoot = 0.3048;
static const double cdYard = 0.9144;
static const double pi = 3.1415926535897932;
std::wstring MakeFilename(const std::wstring & name, const std::wstring & basename);

#define L(x) (GetDict().Translate(L##x))

class Dict
{
public:
	Dict();
	~Dict();
	void Read(const wchar_t * wcFilename);
	wchar_t * Translate(const wchar_t * wcOriginal);
private:
	struct Data;
	Data * m_data;
};

extern Dict & GetDict();

bool UTCVariantTimeToLocalVariantTime(double dUTCTime, double &dLocalTime);
bool LocalVariantTimeToUTCVariantTime(double dLocalTime, double &dUTCTime);
std::wstring UTCTimeToLocalTimeText(unsigned long ulTimeUTC);
std::wstring DurationToText(long iDuration);

// ---------------------------------------------------------------

// Functions to work with UTM coordinates:
// - utmZone between 1 and 60 for the north hemisphere and between -1 and -60 for the south hemisphere.
// - Some functions accept utmZone = 0 for "automatic" but don't care of exceptions
//   (like in Norway where a zone is locally bigger).

void TestUTM(); // Test cases, only used for debug

// LongLatToUTM
// input:
//  - longitude and latitude in degrees
//  - utmZone = wanted zone (or 0 for automatic)
// ouput:
//  - utmZone = used zone (if it was 0 in input, don't take care of exceptions)
//  - utmX = UTM easting in meters
//  - utmY = UTM northing in meters
void LongLatToUTM(double lon360, double lat360, int& utmZone, double& utmX, double& utmY);
void LongLatToUTM(double lon360, double lat360, int& utmZone, int& utmX, int& utmY);

// UTMToLongLat
// input:
//  - UTM easting and northing in meters
//  - utmZone = used zone (should NOT be 0!)
// ouput:
//  - longitude and latitude in degrees
void UTMToLongLat(double utmX, double utmY, int utmZone, double& lon360, double& lat360);

// UTMToLongLat
// input:
//  - utmZone = used zone (should NOT be 0!)
// ouput:
//  - a text describing the zone
std::wstring UTMZoneToLongText(int utmZone);

// GetFraction
// input:
//  - d = double value we need to estimate
//  - maxDen = maximum denominator we expect
// ouput:
//  - a pair of nominator and denominator
std::pair<long, long> GetFraction(double d, long maxDen);

// ---------------------------------------------------------------

#ifndef UNDER_CE
#	define WMKDIR(x) _wmkdir(x)
#else // UNDER_CE
#	define WMKDIR(x) CreateDirectory(x, NULL)
#endif // UNDER_CE

// ---------------------------------------------------------------

#endif // COMMON_H
