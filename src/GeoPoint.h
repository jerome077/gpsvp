/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef GEOPOINT_H
#define GEOPOINT_H

#include "Common.h"
#include "PlatformDef.h"
#include <math.h>

// "Bit width" of fixed-point latitude in GeoPoint. 360 degrees corresponds to 2^GPWIDTH.
// 24 <= GPWIDTH <= 30
#define GPWIDTH (30)

struct GeoPoint
{
	GeoPoint() {};
	GeoPoint(int igLongitude, int igLatitude) : lon(igLongitude), lat(igLatitude){}
	GeoPoint(double dLongitude, double dLatitude) : lon (FromDegree(dLongitude)), lat(FromDegree(dLatitude)) {}
	int lon;
	int lat;
	inline int lat24() const  // 24-bit values for interfaces
	{
		return lat >> (GPWIDTH - 24);
	}
	inline int lon24() const
	{
		return lon >> (GPWIDTH - 24);
	}
	bool operator == (const GeoPoint & pt) const
	{
		return (lon == pt.lon) && (lat == pt.lat);
	}
	bool operator != (const GeoPoint & pt) const
	{
		return !operator ==(pt);
	}
};

#define GeoPoint24(iLongitude24,iLatitude24) GeoPoint((iLongitude24) << (GPWIDTH - 24), (iLatitude24) << (GPWIDTH - 24))

struct GeoRect
{
	int minLon;
	int minLat;
	int maxLon;
	int maxLat;
	GeoRect(){}
	GeoRect(Int a_minLon, Int a_maxLon, Int a_minLat, Int a_maxLat) : 
		minLon(a_minLon), minLat(a_minLat), maxLon(a_maxLon), maxLat(a_maxLat)
	{}
	GeoPoint Center() const
	{
		return GeoPoint((minLon + maxLon) / 2, (minLat + maxLat) / 2);
	}
	bool Contain(const GeoPoint & gp)
	{
		if (gp.lat < minLat) return false;
		if (gp.lat > maxLat) return false;
		if (gp.lon < minLon) return false;
		if (gp.lon > maxLon) return false;
		return true;
	}
	bool Contain(const GeoRect & rect)
	{
		if (rect.minLat < minLat) return false;
		if (rect.maxLat > maxLat) return false;
		if (rect.minLon < minLon) return false;
		if (rect.maxLon > maxLon) return false;
		return true;
	}
	bool Intersect(const GeoRect & rect) const
	{
		if (rect.maxLat < minLat) return false;
		if (rect.minLat > maxLat) return false;
		if (rect.maxLon < minLon) return false;
		if (rect.minLon > maxLon) return false;
		return true;
	}

	void Init(const GeoPoint & gp)
	{
		minLon = gp.lon;
		maxLon = gp.lon;
		minLat = gp.lat;
		maxLat = gp.lat;
	}
	void Append(const GeoPoint & gp)
	{
		if (gp.lat < minLat)
			minLat = gp.lat;
		else if (gp.lat > maxLat)
			maxLat = gp.lat;
		if (gp.lon < minLon)
			minLon = gp.lon;
		else if (gp.lon > maxLon)
			maxLon = gp.lon;
	}
	void Append(const GeoRect & r)
	{
		if (r.minLat < minLat)
			minLat = r.minLat;
		if (r.maxLat > maxLat)
			maxLat = r.maxLat;
		if (r.minLon < minLon)
			minLon = r.minLon;
		if (r.maxLon > maxLon)
			maxLon = r.maxLon;
	}
	void Expand(int r)
	{
		minLat -= r;
		maxLat += r;
		r *= 2;
		minLon -= r;
		maxLon += r;
	}
};

// Distance in meters between gp1 and gp2
inline int IntDistance(const GeoPoint & gp1, const GeoPoint & gp2)
{
	int iLonScale100 = cos100((gp1.lat + gp2.lat) / 2);
	int x = abs(int(iLonScale100 * (int64_t)(gp1.lon - gp2.lon) / 100));
	int y = abs(gp1.lat - gp2.lat);
	int c = 1;
	while (x > (1 << 15) || y > (1 << 15))
	{
		x /= 4;
		y /= 4;
		c *= 4;
	}
	int res = int_sqrt(sqr(x) + sqr(y));  // res < 1.42*2^15
	res *= 40000000 / (1 << 10);  // res < 0.85*2^31
	// assert(GPWIDTH >= 10);
	res /= 1 << (GPWIDTH - 10);
	res *= c;
	return res;
}

double DoubleDistance(const GeoPoint & llPoint1, const GeoPoint & llPoint2);
// Return azimuth in degrees (to the right from the North)
int IntAzimuth(const GeoPoint & llPointFrom, const GeoPoint & llPointTo);

#endif // GEOPOINT_H
