#ifndef GEOPOINT_H
#define GEOPOINT_H

#include "Common.h"
#include <math.h>

struct GeoPoint
{
	GeoPoint() {};
	GeoPoint(int iLongitude, int iLatitude) : lon(iLongitude), lat(iLatitude){}
	GeoPoint(double dLongitude, double dLatitude) : lon (FromDegree(dLongitude)), lat(FromDegree(dLatitude)) {}
	int lon;
	int lat;
	bool operator == (const GeoPoint & pt) const
	{
		return (lon == pt.lon) && (lat == pt.lat);
	}
	bool operator != (const GeoPoint & pt) const
	{
		return !operator ==(pt);
	}
};

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

inline int IntDistance(const GeoPoint & gp1, const GeoPoint & gp2)
{
	int iLonScale100 = cos100((gp1.lat + gp2.lat) / 2);
	int x = abs(iLonScale100 * (gp1.lon - gp2.lon) / 100);
	int y = abs(gp1.lat - gp2.lat);
	int c = 1;
	while (x > (1 << 15) || y > (1 << 15))
	{
		x /= 2;
		y /= 2;
		c *= 2;
	}
	int res = int_sqrt(sqr(x) + sqr(y));
	res *= 4000;
	res /= (1 << 12);
	res *= 10000;
	res /= (1 << 12);
	res *= c;
	return res;
}

double DoubleDistance(const GeoPoint & llPoint1, const GeoPoint & llPoint2);
int IntAzimuth(const GeoPoint & llPointFrom, const GeoPoint & llPointTo);

#endif // GEOPOINT_H
