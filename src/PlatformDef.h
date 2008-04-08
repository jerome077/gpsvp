#ifndef PLATFORMDEF_H
#define PLATFORMDEF_H

#include <stdlib.h>
#include <stdio.h>

// This file contains platform-dependent functions
// and macro definitions

#ifndef DONT_DEFINE_MIN
#	define DEFINE_MIN
#endif

#ifndef USE_STDIO_H
#	define USE_IO_H
#endif

#ifndef NO_EXCEPTION
#	define USE_EXCEPTION
#endif

//! One-byte unsigned integer
typedef unsigned char Byte;
//! At least 4-byte unsigned integer
typedef unsigned int UInt;
//! At least 4-byte signed integer
typedef int Int;

//! Get 32-bit unsigned number from byte buffer
UInt GetUInt32(Byte * pbStart);
//! Get 24-bit unsigned number from byte buffer
UInt GetUInt24(Byte * pbStart);
//! Get 16-bit unsigned number from byte buffer
UInt GetUInt16(Byte * pbStart);
//! Get 24-bit signed number from byte buffer
Int GetInt24(Byte * pbStart);
//! Get 16-bit signed number from byte buffer
UInt GetInt16(Byte * pbStart);

#ifdef DEFINE_MIN
	//! Template min function. Returns minimum of two values
	template < class T > 
	T mymin(T a, T b) 
	{
		if (a<b) 
			return a; 
		else 
			return b;
	}
	template < class T > 
	T mymax(T a, T b) 
	{
		if (a<b) 
			return b; 
		else 
			return a;
	}
#endif // DEFINE_MIN

#ifdef DEFINE_SIZE_T
#	ifndef _SIZE_T_DEFINED
		typedef long unsigned int size_t;
#		define _SIZE_T_DEFINED
#	endif
#endif

#ifdef UNDER_CE
	inline int swprintf(wchar_t * buffer, int size, const wchar_t * format, ...)
	{
		va_list arg;
		va_start(arg, format);
		return vswprintf(buffer, format, arg);
	}
#endif

#ifdef _WINDOWS
inline FILE * wfopen(const wchar_t * name, const wchar_t * mode)
{
	FILE * res = NULL;
	_wfopen_s(&res, name, mode);
	return res;
}
#else
#	define wfopen _wfopen
#endif

#endif // PLATFORMDEF_H
