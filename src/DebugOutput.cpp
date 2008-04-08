#ifndef NO_DEBUG_OUTPUT
	#include <iostream>
	using namespace std;
#endif

#include "DebugOutput.h"

DebugOutput dout;

DebugOutput & DebugOutput::operator << (char * a)
{
#ifndef NO_DEBUG_OUTPUT
	cout << a;
#endif
	return *this;
}

DebugOutput & DebugOutput::operator << (Int a)
{
#ifndef NO_DEBUG_OUTPUT
	cout << a;
#endif
	return *this;
}

DebugOutput & DebugOutput::operator << (UInt a)
{
#ifndef NO_DEBUG_OUTPUT
	cout << a;
#endif
	return *this;
}

DebugOutput & DebugOutput::operator << (string a)
{
#ifndef NO_DEBUG_OUTPUT
	cout << a;
#endif
	return *this;
}

DebugOutput & DebugOutput::operator << (double a)
{
#ifndef NO_DEBUG_OUTPUT
	cout.precision(7);
	cout.flags(cout.flags() & ~ ios_base::scientific);
	cout << a;
#endif
	return *this;
}

DebugOutput & DebugOutput::operator << (GeoPoint a)
{
#ifndef NO_DEBUG_OUTPUT
	cout.precision(7);
	cout.flags(cout.flags() & ~ ios_base::scientific);
	cout << a;
#endif
	return *this;
}

//void DebugOutput::Dump(Byte * data, UInt uiSize)
//{
//#ifndef NO_DEBUG_OUTPUT
//	for (UInt i = 0; i < uiSize; ++i)
//	{
//		cout << hex;
//		cout << UInt(data[i]) << " ";
//	}
//	cout << "\n";
//#endif
//}
//
