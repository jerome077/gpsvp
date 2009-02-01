/*
Copyright (c) 2005-2008, Vsevolod E. Shorin and others
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef GMCOMMON_H
#define GMCOMMON_H

#include "../PlatformDef.h"
#include <string>

// ---------------------------------------------------------------

// To convert between Zoom17 and Zoom1 or Zoom0:
const long LEVEL_REVERSE_OFFSET = 18;

// ---------------------------------------------------------------

// To convert a "google" tile reference 'x', 'y' at a given zoom level 'zoom17'
// to longitude or latitude:
double	GoogleXZ17toLong	(unsigned long x, unsigned char zoom17);
double	GoogleYZ17toLat		(unsigned long y, unsigned char zoom17);

// ---------------------------------------------------------------

// To convert a "google" tile reference 'x', 'y' at a given zoom level
// to a quadtree reference like for Google satellite:
std::string		GoogleXYZ1toQRST	(unsigned long x, unsigned long y, unsigned char zoom1,
									 int maxCharCount = 0, int firstChar = 0);
std::wstring	GoogleXYZ1toQRSTW	(unsigned long x, unsigned long y, unsigned char zoom1,
									 int maxCharCount = 0, int firstChar = 0);

inline std::string	GoogleXYZ17toQRST	(unsigned long x, unsigned long y, unsigned char zoom17,
										 int maxCharCount = 0, int firstChar = 0)
{ return GoogleXYZ1toQRST(x, y, LEVEL_REVERSE_OFFSET-zoom17, maxCharCount, firstChar); }

inline std::wstring	GoogleXYZ17toQRSTW	(unsigned long x, unsigned long y, unsigned char zoom17,
										 int maxCharCount = 0, int firstChar = 0)
{ return GoogleXYZ1toQRSTW(x, y, LEVEL_REVERSE_OFFSET-zoom17, maxCharCount, firstChar); }

// ---------------------------------------------------------------

// To convert a "google" tile reference 'x', 'y' at a given zoom level
// to a quadtree reference like for Live maps:
std::string		GoogleXYZ1toQKey	(unsigned long x, unsigned long y, unsigned char zoom1,
									 int maxCharCount = 0, int firstChar = 0);
std::wstring	GoogleXYZ1toQKeyW	(unsigned long x, unsigned long y, unsigned char zoom1,
									 int maxCharCount = 0, int firstChar = 0);

inline std::string	GoogleXYZ17toQKey	(unsigned long x, unsigned long y, unsigned char zoom17,
										 int maxCharCount = 0, int firstChar = 0)
{ return GoogleXYZ1toQKey(x, y, LEVEL_REVERSE_OFFSET-zoom17, maxCharCount, firstChar); }

inline std::wstring	GoogleXYZ17toQKeyW	(unsigned long x, unsigned long y, unsigned char zoom17,
										 int maxCharCount = 0, int firstChar = 0)
{ return GoogleXYZ1toQKeyW(x, y, LEVEL_REVERSE_OFFSET-zoom17, maxCharCount, firstChar); }

// ---------------------------------------------------------------

// test function, see comment with the implementation
void LongLatToUTM(double lon360, double lat360, double& utmX, double& utmY, int& utmZone);

// ---------------------------------------------------------------


#endif // GMCOMMON_H
