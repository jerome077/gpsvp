#include "GeoPoint.h"

static double Sqr(double dValue)
{
	return dValue * dValue;
}

int IntAzimuth(const GeoPoint & llPointFrom, const GeoPoint & llPointTo)
{
	int iLonScale100 = cos100((llPointFrom.lat + llPointTo.lat) / 2);

	double dLat = Degree(llPointTo.lat) - Degree(llPointFrom.lat);
	double dLon = Degree(llPointTo.lon) - Degree(llPointFrom.lon);
	dLon = dLon * iLonScale100 / 100;
	double res = 0;
	if (dLat == 0)
	{
		if (dLon > 0)
			res = 90;
		else if (dLon < 0)
			res = -90;
	} 
	else
	{
		res = atan(dLon / dLat) / pi * 180.0;
		if (dLat < 0)
			res += 180.0;
	}
	if (res < 0)
		res += 360;
	return int(res);
}

double DoubleDistance(const GeoPoint & llPoint1, const GeoPoint & llPoint2)
{
	// p1, pnt2 - точки между которыми вычисляются расстояния
	// pi - число pi, rad - радиус сферы (Земли), num - количество
	// знаков после запятой
	double rad = 6372795;

	// получение координат точек в радианах
	double lat1 = Degree(llPoint1.lat)*pi/180;
	double lat2 = Degree(llPoint2.lat)*pi/180;
	double lng1 = Degree(llPoint1.lon)*pi/180;
	double lng2 = Degree(llPoint2.lon)*pi/180;

	// косинусы и синусы широт и разниц долгот
	double cl1 = cos(lat1);
	double cl2 = cos(lat2);
	double sl1 = sin(lat1);
	double sl2 = sin(lat2);
	double delta = lng2 - lng1;
	double cdelta = cos(delta);
	double sdelta = sin(delta);

	// вычисления длины большого круга
	double p1 = Sqr((cl2*sdelta));
	double p2 = Sqr((cl1*sl2) - (sl1*cl2*cdelta));
	double p3 = sqrt(p1 + p2);
	double p4 = sl1*sl2;
	double p5 = cl1*cl2*cdelta;
	double p6 = p4 + p5;
	double p7 = p3/p6;
	double anglerad = atan(p7);
	if (anglerad < 0)
		anglerad += pi;
	double dist = anglerad*rad;

	// возврат значения длины большого круга
	return dist;
}
