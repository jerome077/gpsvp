/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef DEBUGOUTPUT_H
#define DEBUGOUTPUT_H

#include <string>

using namespace std;

#include "PlatformDef.h"
#include "GeoPoint.h"

class DebugOutput
{
public:
	DebugOutput & operator << (char * a);
	DebugOutput & operator << (Int a);
	DebugOutput & operator << (UInt a);
	DebugOutput & operator << (string a);
	DebugOutput & operator << (double a);
	DebugOutput & operator << (GeoPoint gp);
	// void Dump(Byte * data, UInt uiSize);
};

#ifndef NO_DEBUG_OUTPUT

inline ostream & operator << (ostream & ostr, GeoPoint gp)
{
	return ostr << gp.lon << ", " << gp.lat;
}

#endif // NO_DEBUG_OUTPUT

extern DebugOutput dout;

#endif // DEBUGOUTPUT_H