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