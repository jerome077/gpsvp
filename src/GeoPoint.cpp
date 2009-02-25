/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


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
	// llPoint1, llPoint2 - points to find the distance between
	// pi = 3.1415...,  rad - sphere (Earth) radius
	double rad = 6372795;

	// Get coordinates in radians
	double lat1 = Degree(llPoint1.lat)*pi/180;
	double lat2 = Degree(llPoint2.lat)*pi/180;
	double lng1 = Degree(llPoint1.lon)*pi/180;
	double lng2 = Degree(llPoint2.lon)*pi/180;

	// Cosines and sines of latitudes and longitude differences
	double cl1 = cos(lat1);
	double cl2 = cos(lat2);
	double sl1 = sin(lat1);
	double sl2 = sin(lat2);
	double delta = lng2 - lng1;
	double cdelta = cos(delta);
	double sdelta = sin(delta);

	// Calculate the length of the great circle arc
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

	// Return the length of the great circle arc
	return dist;
}
