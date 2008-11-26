/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef PLATFORMDEF_H
#define PLATFORMDEF_H

#include <stdlib.h>
#include <stdio.h>

// This file contains platform-dependent functions
// and macro definitions

#ifndef LINUX
#include <windows.h>
#endif

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
#elif defined(LINUX)
// LINUXTODO:
inline FILE * wfopen(const wchar_t * name, const wchar_t * mode)
{
	return 0;
}
#else
#	define wfopen _wfopen
#endif

#ifdef LINUX
#	define fnchar_t char
#	define FN(x) (x)
#else
#	define fnchar_t wchar_t
#	define FN(x) (L##x)
#endif

#define fnstring basic_string<fnchar_t>

#ifdef LINUX
#	define tchar_t char
#else
#	define tchar_t wchar_t
#endif

#define tstring basic_string<tchar_t>

#endif // PLATFORMDEF_H
