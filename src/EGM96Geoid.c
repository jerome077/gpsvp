/*
WGS84 to EGM2008 altitude separation approximation function

Copyright (c) 2009, Maurits van Dueren den Hollander
Updated April 2010, together with Mach2003 of XDA
http://forum.xda-developers.com/showthread.php?t=571266

This source code is provided as is. No claims are made as to fitness
for any particular purpose. No warranties of any kind are expressed
or implied. The source was updated to use EGM2008 rather than EGM96, variable names
still refer to 96

Limitations on Rights to Redistribute This Code:

The matrix was build from NGA information as described
http://earth-info.nga.mil/GandG/wgs84/gravitymod/egm2008/egm08_wgs84.html
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
reporting between 40 and 60 meters. The correction  turns out to be 44.25 meters, this
routine gives 46 meters, and now my altitude typically shows between -5 and 15 meters,
which is correctly centering around the true value of 5 meters.
(Note: A recent SirfIII chipset gave 47 meters, so was less correct, though that may
be an annecdotal occurance: In general, if a shipset provides separation, we assume the
chipset has the better value)

Additionally a function intelligence is provided, so that if the GPS already provides 
altitude separation data, this is detected and no second correction is applied. Also a 
fixed user provided separation can be used in stead of the position dependant matrix

It should be noted that the GPS Mod driver of XDA author Mach2003 uses a 1 degree lookup table
rather than the 10 degree here, so may be assumed more correct. For Windows Mobile devices, 
said driver is advised, and will automatically disable this routine.
http://forum.xda-developers.com/showthread.php?t=571266


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
static double dEGM96Separation;
	// Latest separation info from GPS or Array (depending on mode)
	// Advised write-back to registry or ini only at program close
	// For mode 3=User, the user provided separation value

// Array with WGS84 to EGM2008 altitude separation
// Warning: array is tilted with north to the right.
// http://cddis.nasa.gov/926/egm96/geoid_050.gif

static const signed char geoid[19*37] = { 
/*       -90,-80,-70,-60,-50,-40,-30,-20,-10,  0, 10, 20, 30, 40, 50, 60, 70, 80, 90 
  -180 */-30,-52,-62,-46,-16, 20, 48, 50, 35, 21, 13,  4, -7,-12, -6,  1,  2,  3, 15,
/*-170 */-30,-53,-63,-43,-19,  6, 23, 26, 21, 15, 12, 10, -5,-11,  6, 10,  1,  2, 15,
/*-160 */-30,-53,-60,-37,-18,  1,  6,  9, 11, 16, 11,  8, -9,-13,  7, 15, -1,  0, 15,
/*-150 */-30,-52,-57,-32,-16, -7, -1, -1,  5, 13,  1, -7,-16,-21,  0, 14, -1,  0, 15,
/*-140 */-30,-48,-52,-30,-17,-12, -9, -8, -3,  1,-11,-23,-28,-31,-11, 11, -3, -1, 15,
/*-130 */-30,-43,-44,-27,-16,-13,-13,-11, -9,-13,-29,-39,-40,-37,-18,  0, -8, -2, 15,
/*-120 */-30,-39,-38,-24,-10,-13,-10, -5,-11,-23,-39,-47,-42,-23,-16,-16,-15,  0, 15,
/*-110 */-30,-35,-32,-22,-11,-10, -8, -2, -8,-20,-29,-33,-30,-18,-18,-31,-25,  6, 15,
/*-100 */-30,-29,-27,-17, -8, -7, -4, -4,-12,-14,-11, -7,-23,-25,-24,-42,-27,  4, 15,
/* -90 */-30,-23,-21,-11, -3, -1,  1, -1, -9, -4,  2,-11,-27,-33,-35,-47,-26,  9, 15,
/* -80 */-30,-24, -8, -4,  5,  7,  8,  8, -1, 15,  1,-20,-32,-34,-39,-42,-18, 12, 15,
/* -70 */-30,-25,  2,  9, 12, 22, 36, 35, 28, 14,-10,-48,-52,-34,-26,-23,  3,  7, 15,
/* -60 */-30,-22,  5, 20, 13, 13, 18, 20,  4,-13,-41,-47,-41,-27,-13,  5, 24, 19, 15,
/* -50 */-30,-19,  0, 20,  2, -3,  3, -8,-19,-26,-43,-33,-18,  1, 24, 29, 29, 28, 15,
/* -40 */-30,-17,  2, 21,  4, -7, -8, -7,-12,-19,-17,-10, 17, 32, 44, 49, 45, 34, 15,
/* -30 */-30,-15,  1, 24, 11,  6,  3, -6,-10,  2,  3, 17, 30, 59, 63, 64, 56, 37, 15,
/* -20 */-30,-10,  3, 21, 21, 21, 10,  0,  4, 14, 17, 25, 34, 52, 62, 61, 60, 32, 15,
/* -10 */-30, -6, 10, 17, 27, 23, 15, 12, 13, 19, 33, 31, 44, 49, 59, 56, 56, 34, 15,
/*   0 */-30, -5, 14, 15, 25, 18, 22, 17, 11, 17, 24, 31, 34, 51, 45, 48, 49, 36, 15,
/*  10 */-30, -2, 17, 20, 26, 25, 26, 23, 13,  9, 22, 27, 28, 45, 48, 41, 42, 35, 15,
/*  20 */-30,  1, 18, 25, 33, 29, 33, 22,  5,-17,  2, 14, 26, 34, 40, 19, 30, 29, 15,
/*  30 */-30,  4, 20, 29, 38, 33, 31,  7,-12,-11, -5,  9, 16, 39, 27, 16, 20, 24, 15,
/*  40 */-30,  5, 22, 34, 45, 39, 14,-10,-27,-28,-10,  2, 11, 31, 11, 11, 11, 18, 15,
/*  50 */-30,  5, 29, 33, 45, 41, 15,-11,-32,-49,-32,-31,-18,-18,-12,  6,  3, 13, 15,
/*  60 */-30,  5, 30, 33, 38, 28, 15,-10,-37,-63,-58,-42,-15,-29,-19, -4, -3,  8, 15,
/*  70 */-30,  6, 17, 29, 40, 23,  6,-21,-61,-89,-91,-61,-38,-41,-33,-21,-10,  2, 15,
/*  80 */-30,  6, 18, 25, 27, 13,-9,-40,-76,-103,-97,-68,-37,-55,-44,-30,-13,  2, 15,
/*  90 */-30, -3, 11,  9, 13, -2,-25,-48,-64,-63,-63,-59,-34,-66,-42,-34,-12,  1, 15,
/* 100 */-30,-13, -8, -3, -2,-21,-38,-46,-26, -7,-25,-38,-34,-55,-42,-34,-14,  3, 15,
/* 110 */-30,-25,-21,-14,-15,-33,-39,-26, -1, 34, 11,-13,-24,-28,-30,-28, -9,  2, 15,
/* 120 */-30,-34,-27,-25,-22,-34,-22,  4, 38, 60, 52, 22,  8,  2, -4,-17, -8,  2, 15,
/* 130 */-30,-42,-39,-31,-22,-27,-14, 24, 52, 76, 60, 38, 30, 23, 15,  0, -5,  1, 15,
/* 140 */-30,-50,-43,-33,-18,-15, 13, 46, 69, 72, 60, 49, 42, 38, 23, 12,  0,  0, 15,
/* 150 */-30,-51,-55,-30,-17, -2, 32, 57, 77, 62, 45, 41, 19, 17, 20, 16,  3,  2, 15,
/* 160 */-30,-50,-56,-35,-14,  5, 34, 57, 62, 49, 35, 21,  3, -2,  6, 14,  3,  2, 15,
/* 170 */-30,-51,-58,-45,-10, 19, 44, 64, 51, 32, 25,  9, -7,-11,  3,  9,  3,  1, 15,
/* 180 */-30,-52,-62,-46,-16, 20, 48, 50, 35, 21, 13,  4, -7,-12, -6,  1,  2,  3, 15
};

// Older EGM96 table
//static const char geoid[19*37] = {
//*       -90,-80,-70,-60,-50,-40,-30,-20,-10,  0, 10, 20, 30, 40, 50, 60, 70, 80, 90 
// -180 */-30,-53,-62,-46,-17, 21, 48, 50, 35, 21, 13,  4, -8,-13, -8,  0,  3,  4, 14,
//*-170 */-29,-53,-63,-43,-19,  6, 23, 26, 21, 15, 12,  9, -5,-11,  5,  9,  1,  3, 14,
//*-160 */-29,-52,-61,-37,-18,  1,  6,  9, 11, 16, 10,  8, -9,-13,  7, 15, -1,  1, 14,
//*-150 */-29,-51,-57,-32,-16, -7, -1, -1,  5, 13,  1, -7,-16,-21,  0, 13, -1,  0, 14,
//*-140 */-29,-48,-52,-30,-17,-12, -9, -9, -3,  1,-11,-23,-28,-31,-12, 11, -3, -2, 14,
//*-130 */-29,-42,-44,-27,-16,-13,-13,-11, -9,-13,-29,-39,-40,-37,-18,  0, -8, -3, 14,
//*-120 */-29,-39,-37,-24,-10,-13,-10, -6,-11,-23,-39,-48,-42,-23,-16,-16,-15, -1, 14,
//*-110 */-29,-37,-33,-22,-11,-10, -8, -2, -8,-20,-29,-34,-30,-18,-17,-31,-25,  4, 14,
//* 100 */-29,-28,-26,-17, -9, -7, -4, -4,-12,-14,-12, -7,-23,-25,-24,-42,-28,  3, 14,
//* -90 */-29,-22,-21,-11, -3, -1,  1, -1, -9, -4,  1,-11,-27,-33,-35,-47,-26, 11, 14,
//* -80 */-29,-25,-10, -4,  5,  7,  8,  8, -1, 14,  1,-19,-32,-34,-40,-42,-18, 15, 14,
//* -70 */-29,-22,  4,  9, 11, 23, 37, 34, 27, 14,-11,-47,-52,-35,-27,-23,  3,  7, 14,
//* -60 */-29,-21,  5, 20, 12, 13, 18, 20,  3,-13,-42,-47,-41,-23,-13,  5, 23, 18, 14,
//* -50 */-29,-20,  0, 20,  2, -3,  3, -8,-19,-26,-43,-33,-18,  1, 24, 29, 30, 28, 14,
//* -40 */-29,-18,  3, 22,  3, -7, -9, -7,-12,-19,-16,-10, 16, 32, 45, 49, 46, 34, 14,
//* -30 */-29,-17,  1, 24, 11,  6,  3, -6,-10,  2,  3, 17, 30, 59, 62, 64, 57, 36, 14,
//* -20 */-29,-13,  2, 21, 20, 21, 10, -1,  4, 13, 17, 25, 34, 52, 62, 61, 60, 32, 14,
//* -10 */-29,-10,  9, 17, 27, 23, 15, 12, 13, 19, 33, 30, 43, 49, 58, 55, 57, 32, 14,
//*   0 */-29, -7, 14, 15, 25, 17, 21, 16, 11, 17, 23, 31, 35, 51, 45, 48, 49, 34, 14,
//*  10 */-29, -3, 14, 20, 25, 25, 26, 23, 13,  9, 22, 27, 28, 46, 48, 40, 42, 35, 14,
//*  20 */-29,  1, 16, 25, 33, 29, 34, 22,  6,-17,  2, 14, 26, 34, 40, 19, 30, 30, 14,
//*  30 */-29,  4, 21, 29, 37, 33, 30,  7,-13,-10, -5, 10, 16, 39, 27, 16, 20, 24, 14,
//*  40 */-29,  4, 21, 34, 45, 38, 14,-10,-27,-27,-10,  2, 10, 29, 11, 11, 11, 18, 14,
//*  50 */-29,  6, 27, 33, 45, 40, 14,-11,-32,-49,-32,-31,-18,-16,-12,  6,  3, 13, 14,
//*  60 */-29,  8, 26, 33, 38, 27, 15,-10,-38,-63,-58,-43,-15,-28,-19, -4, -3,  9, 14,
//*  70 */-29,  5, 17, 29, 40, 23,  6,-21,-61,-89,-91,-61,-39,-41,-33,-21, -9,  4, 14,
//*  80 */-29,  4, 16, 25, 27, 13,-9,-40,-76,-103,-96,-68,-35,-56,-45,-31,-13,  2, 14,
//*  90 */-29, -2,  8,  9, 13, -2,-25,-48,-65,-63,-63,-60,-34,-67,-42,-34,-13,  2, 14,
//* 100 */-29,-14, -6, -3, -2,-21,-38,-46,-27, -7,-25,-37,-32,-55,-42,-34,-15,  2, 14,
//* 110 */-29,-24,-18,-14,-15,-33,-40,-26, -2, 34, 11,-13,-24,-29,-30,-28, -9,  0, 14,
//* 120 */-29,-34,-29,-25,-23,-34,-23,  4, 36, 58, 52, 21,  7,  3, -5,-17, -9, -1, 14,
//* 130 */-29,-41,-37,-31,-22,-27,-14, 24, 52, 76, 59, 38, 30, 23, 15,  1, -6, -1, 14,
//* 140 */-29,-49,-44,-33,-19,-15, 13, 46, 69, 72, 61, 49, 42, 38, 22, 12, -1,  0, 14,
//* 150 */-29,-50,-55,-30,-17, -2, 32, 57, 78, 63, 45, 41, 19, 17, 20, 16,  2,  3, 14,
//* 160 */-29,-54,-56,-35,-14,  5, 33, 57, 63, 49, 35, 22,  3, -2,  6, 14,  3,  1, 14,
//* 170 */-29,-51,-59,-44,-10, 19, 44, 64, 51, 32, 25,  9, -7,-11,  3,  9,  4,  2, 14,
//* 180 */-30,-53,-62,-46,-17, 21, 48, 50, 35, 21, 13,  4, -8,-13, -8,  0,  3,  4, 14
//};

void EGM96init(wchar_t *sGeoidMode)
{	
	// Convert and Store registry mode into static variable
	if (wcscmp(sGeoidMode,L"Auto")==0) { iEGM96Mode = -127; return; }
	if (wcscmp(sGeoidMode,L"Never")==0) { iEGM96Mode = -126; return; }
	if (wcscmp(sGeoidMode,L"Always")==0) { iEGM96Mode = -125; return; }
	iEGM96Mode = (char)wcstol(sGeoidMode, 0, 10);
	if (iEGM96Mode==0 && wcscmp(sGeoidMode,L"0")!=0) { iEGM96Mode = -127; return; }
}

void EGM96Geoid(double dLat, double dLon, double *dAlt, double *dSep)
{
	double dLatDelta, dLonDelta, dIdx;
	double dMid1, dMid2;
	int iLatIdx, iLonIdx, iIdx;

	switch (iEGM96Mode) {

	case -127: // "Auto" correct Altitudes if GPS does not provide separation
		if (*dSep != 0) return; // means GPS already provides separation
		// no return, continue with case 2: Always

	case -125: // "Always" correct Altitudes, do not check if GPS already does this
		// First undo chipset correction (if any)
		*dAlt += *dSep;	

		// Array index of position
		dLatDelta = modf (dLat/10, &dIdx);
		iLatIdx = (int)dIdx;
		if (dLat<0) {
		   iLatIdx--;
		   dLatDelta++;
		}
		dLonDelta = modf (dLon/10, &dIdx);
		iLonIdx = (int)dIdx;
		if (dLon<0) {
		   iLonIdx--;
		   dLonDelta++;
		}
		iIdx = (iLonIdx + 18) * 19 + iLatIdx + 9;

		// Calculate interpolated altitude
		dMid1 = dLatDelta * (geoid[iIdx+1 ] - geoid[iIdx   ]) + geoid[iIdx   ];
		dMid2 = dLatDelta * (geoid[iIdx+20] - geoid[iIdx+19]) + geoid[iIdx+19];
		*dSep = dLonDelta * (dMid2          - dMid1         ) + dMid1;

		// Apply correction
 		*dAlt -= *dSep;
		return;

	case -126: // "Never" correct Altitudes
		return;
		
	default: // Apply user determined fixed altitude correction
		// First undo chipset correction (if any)
		*dAlt += *dSep;	
		// Then apply user correction
 		*dSep = (double)iEGM96Mode;
 		*dAlt -= *dSep;
		return;

	}
}
