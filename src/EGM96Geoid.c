/*
WGS84 to EGM96 altitude separation approximation function

Copyright (c) 2009, Maurits van Dueren den Hollander

This source code is provided as is. No claims are made as to fitness
for any particular purpose. No warranties of any kind are expressed
or implied.

Limitations on Rights to Redistribute This Code:

The matrix was build from NASA-MIMA join venture EGM96 data as described
http://cddis.nasa.gov/926/egm96/egm96.html and calculated by 
http://earth-info.nima.mil/GandG/wgs84/gravitymod/egm96/intpt.html
or http://sps.unavco.org/geoid/
It is assumed these organisations provide this inforation free, but
any rights that may be held on said data may put additional restriction
on the use of this source code.

I hereby grant the right to freely use the information supplied in
this file in the creation of products, and to make copies of this file
in any form for internal or external distribution as long as this notice
remains attached to the source. Honerable mention of above copyright 
in any derived product description ("about box")is appriciated but not
mandetory. A free license for any derived product is also appriciated but
not mandetory.
 
May 2009, maurits@vandueren.nl

The function provides a simple but effective WGS84 to EGM96 altitude
correction. Such correction is needed as the GSP WGS84 elipsiod can vary from
true local geoid altitude by as much as 100meter. Proper & newer EMEA GPS receivers
provide both the WGS84 and geoid altitude, but many GPS receivers leave the latter
value blank, causing many GPS software to provide only rather incorrect information.

Implementing the full n-order function from NASA may provide more precision
then needed, at too high resource cost (for small handheld devices). This function
operates from a simple lookup matrix with 10 degree resolution and interpolates the
current position in the matrix. Only a few few integer and float calculations are used,
the size is less than 2kB and the altitude correction found seems to often be correct
within 1 meter of the full EGM96 or EGM2008 calculation.

This does not imply the altitude value is correct to within 1 meter, as normal GPS
inprecision due to satelite orbid, atmospheric interference etc. still applies. But
at least there is no more structural error (which as stated can be up to 100meter),
only incidental errors remain (which can be up to 15 meters) 

For example, in my home, in famously at sea-level Netherlands, GPS was structurally
reporting between 40 and 60 meters. The correction  turns out to be 45.1 meters, this
routine gives 44.9 meters, and now my altitude typically shows between -5 and 15 meters,
which is correctly centering around the true value of 5 meters. 

Additionally a function intelligence is provided, so that if the GPS already provides 
altitude separation data, this is detected and no second correction is applied. Also a 
fixed user provided separation can be used in stead of the position dependant matrix

*/

#include <math.h>
#include "EGM96Geoid.h"

// The following values are (to be) initialized from the m_rsEGM96Altitude registry entry
static char	iEGM96Mode;
	// Correction mode from registry or ini file
	// +127 = 0 = Auto: Perform correction if GPS does not already do this
	//                  So if NMEA field 11 = zero, calculate
	// +126 = 1 = Never: Do not perform any correction
	// +125 = 2 = Always: Always peform correction
	//                  Add existing field 11 to heigth, then subtract our calculation
	// -99..99  = User provided separation value
	// 4 = Sirf: GPS does provides info, but it still needs to be applied
static double dEGM96Separation;
	// Latest separation info from GPS or Array (depending on mode)
	// Advised write-back to registry or ini only at program close
	// For mode 3=User, the user provided separation value

// Array with WGS84 to EGM96 altitude separation
// Warning: array is tilted with north to the right.
// http://cddis.nasa.gov/926/egm96/geoid_050.gif
static const char geoid[19*37] = {
/*       -90,-80,-70,-60,-50,-40,-30,-20,-10,  0, 10, 20, 30, 40, 50, 60, 70, 80, 90 
  -180 */-30,-53,-62,-46,-17, 21, 48, 50, 35, 21, 13,  4, -8,-13, -8,  0,  3,  4, 14,
/*-170 */-29,-53,-63,-43,-19,  6, 23, 26, 21, 15, 12,  9, -5,-11,  5,  9,  1,  3, 14,
/*-160 */-29,-52,-61,-37,-18,  1,  6,  9, 11, 16, 10,  8, -9,-13,  7, 15, -1,  1, 14,
/*-150 */-29,-51,-57,-32,-16, -7, -1, -1,  5, 13,  1, -7,-16,-21,  0, 13, -1,  0, 14,
/*-140 */-29,-48,-52,-30,-17,-12, -9, -9, -3,  1,-11,-23,-28,-31,-12, 11, -3, -2, 14,
/*-130 */-29,-42,-44,-27,-16,-13,-13,-11, -9,-13,-29,-39,-40,-37,-18,  0, -8, -3, 14,
/*-120 */-29,-39,-37,-24,-10,-13,-10, -6,-11,-23,-39,-48,-42,-23,-16,-16,-15, -1, 14,
/*-110 */-29,-37,-33,-22,-11,-10, -8, -2, -8,-20,-29,-34,-30,-18,-17,-31,-25,  4, 14,
/* 100 */-29,-28,-26,-17, -9, -7, -4, -4,-12,-14,-12, -7,-23,-25,-24,-42,-28,  3, 14,
/* -90 */-29,-22,-21,-11, -3, -1,  1, -1, -9, -4,  1,-11,-27,-33,-35,-47,-26, 11, 14,
/* -80 */-29,-25,-10, -4,  5,  7,  8,  8, -1, 14,  1,-19,-32,-34,-40,-42,-18, 15, 14,
/* -70 */-29,-22,  4,  9, 11, 23, 37, 34, 27, 14,-11,-47,-52,-35,-27,-23,  3,  7, 14,
/* -60 */-29,-21,  5, 20, 12, 13, 18, 20,  3,-13,-42,-47,-41,-23,-13,  5, 23, 18, 14,
/* -50 */-29,-20,  0, 20,  2, -3,  3, -8,-19,-26,-43,-33,-18,  1, 24, 29, 30, 28, 14,
/* -40 */-29,-18,  3, 22,  3, -7, -9, -7,-12,-19,-16,-10, 16, 32, 45, 49, 46, 34, 14,
/* -30 */-29,-17,  1, 24, 11,  6,  3, -6,-10,  2,  3, 17, 30, 59, 62, 64, 57, 36, 14,
/* -20 */-29,-13,  2, 21, 20, 21, 10, -1,  4, 13, 17, 25, 34, 52, 62, 61, 60, 32, 14,
/* -10 */-29,-10,  9, 17, 27, 23, 15, 12, 13, 19, 33, 30, 43, 49, 58, 55, 57, 32, 14,
/*   0 */-29, -7, 14, 15, 25, 17, 21, 16, 11, 17, 23, 31, 35, 51, 45, 48, 49, 34, 14,
/*  10 */-29, -3, 14, 20, 25, 25, 26, 23, 13,  9, 22, 27, 28, 46, 48, 40, 42, 35, 14,
/*  20 */-29,  1, 16, 25, 33, 29, 34, 22,  6,-17,  2, 14, 26, 34, 40, 19, 30, 30, 14,
/*  30 */-29,  4, 21, 29, 37, 33, 30,  7,-13,-10, -5, 10, 16, 39, 27, 16, 20, 24, 14,
/*  40 */-29,  4, 21, 34, 45, 38, 14,-10,-27,-27,-10,  2, 10, 29, 11, 11, 11, 18, 14,
/*  50 */-29,  6, 27, 33, 45, 40, 14,-11,-32,-49,-32,-31,-18,-16,-12,  6,  3, 13, 14,
/*  60 */-29,  8, 26, 33, 38, 27, 15,-10,-38,-63,-58,-43,-15,-28,-19, -4, -3,  9, 14,
/*  70 */-29,  5, 17, 29, 40, 23,  6,-21,-61,-89,-91,-61,-39,-41,-33,-21, -9,  4, 14,
/*  80 */-29,  4, 16, 25, 27, 13,-9,-40,-76,-103,-96,-68,-35,-56,-45,-31,-13,  2, 14,
/*  90 */-29, -2,  8,  9, 13, -2,-25,-48,-65,-63,-63,-60,-34,-67,-42,-34,-13,  2, 14,
/* 100 */-29,-14, -6, -3, -2,-21,-38,-46,-27, -7,-25,-37,-32,-55,-42,-34,-15,  2, 14,
/* 110 */-29,-24,-18,-14,-15,-33,-40,-26, -2, 34, 11,-13,-24,-29,-30,-28, -9,  0, 14,
/* 120 */-29,-34,-29,-25,-23,-34,-23,  4, 36, 58, 52, 21,  7,  3, -5,-17, -9, -1, 14,
/* 130 */-29,-41,-37,-31,-22,-27,-14, 24, 52, 76, 59, 38, 30, 23, 15,  1, -6, -1, 14,
/* 140 */-29,-49,-44,-33,-19,-15, 13, 46, 69, 72, 61, 49, 42, 38, 22, 12, -1,  0, 14,
/* 150 */-29,-50,-55,-30,-17, -2, 32, 57, 78, 63, 45, 41, 19, 17, 20, 16,  2,  3, 14,
/* 160 */-29,-54,-56,-35,-14,  5, 33, 57, 63, 49, 35, 22,  3, -2,  6, 14,  3,  1, 14,
/* 170 */-29,-51,-59,-44,-10, 19, 44, 64, 51, 32, 25,  9, -7,-11,  3,  9,  4,  2, 14,
/* 180 */-30,-53,-62,-46,-17, 21, 48, 50, 35, 21, 13,  4, -8,-13, -8,  0,  3,  4, 14
};

void EGM96init(double sGeoidMode, double dGeoidSeparation)
{	
	// Convert and Store registry mode into static variable
	iEGM96Mode = 0;
	// Store registry value into static variable
	dEGM96Separation = dGeoidSeparation;
}

void EGM96Geoid(double dLat, double dLon, double *dAlt, double *dSep)
{
	double dLatDelta, dLonDelta, dIdx;
	int iLatIdx, iLonIdx, iIdx;

	switch (iEGM96Mode) {

	case 0: // "Auto" correct Altitudes if GPS does not provide separation
		if (*dSep != 0) return; // means GPS already provides separation
		// no return, continue with case 2: Always

	case 2: // "Always" correct Altitudes, do not check if GPS already does this
		// First undo chipset correction (if any)
		*dAlt += *dSep;	

		// Array index of position
		dLatDelta = modf (dLat/10, &dIdx);
		iLatIdx = dIdx;
		dLonDelta = modf (dLon/10, &dIdx);
		iLonIdx = dIdx;
		iIdx = (iLonIdx + 18) * 19 + iLatIdx + 9;

		// Calculate interpolated altitude
		*dSep =
			((geoid[iIdx] - geoid[iIdx+1] + geoid[iIdx+20] - geoid[iIdx+19])
			* (dLonDelta * dLatDelta / 100)) + geoid[iIdx];

		// Apply correction
 		*dAlt -= *dSep;
		return;
		
	case 3: // User determined fixed altitude correction
		// First undo chipset correction (if any)
		*dAlt += *dSep;	

		// Apply user correction
		*dAlt -= dEGM96Separation;
		return;

	case 4: // "Sirf" if GPS does profide separation, but it still needs to be added
 		*dAlt -= *dSep;
		return;

	case 1: // "Never" correct Altitudes
	default: // unknown mode, just return normal altitude
		return;
	}
}
