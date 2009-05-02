/*	
Copyright (c) 2005-2008, Jerome077 and others
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef VARIABLEINTERPRETER_H
#define VARIABLEINTERPRETER_H

#include <string>
#include <vector>
#include "../PlatformDef.h"
#include "GMCommon.h"

void Test_CStringSchema();

// ---------------------------------------------------------------

class CSchema
{
public:
	virtual std::wstring interpret(unsigned char l, unsigned long x, unsigned long y) = 0;
};

// ---------------------------------------------------------------

// Interpreter for the variables in user defined maps:
// The URL, filename and subpath in the configuration file can make
// use of Variables like %LONG1, %LAT1, %X, %ZOOM_00 which will be interperted here.
class CStringSchema: public CSchema
{
public:
	~CStringSchema() { clear(); };
	void clear();
	void assign(const std::wstring& strSchema);
	virtual std::wstring interpret(unsigned char l, unsigned long x, unsigned long y);
	bool empty() { return m_SchemaParts.empty(); };
protected:
	typedef std::vector<CSchema*> TPSchemaParts;
	TPSchemaParts m_SchemaParts;
};

// ---------------------------------------------------------------

// CSimpleStringSchema: for a part of the string without variable
class CSimpleStringSchema: public CSchema
{
public:
	CSimpleStringSchema(const std::wstring& strSimpleString) : m_SimpleString(strSimpleString) {};
	virtual std::wstring interpret(unsigned char l, unsigned long x, unsigned long y)
	{
		return m_SimpleString;
	};

protected:
	std::wstring m_SimpleString;
};

// ---------------------------------------------------------------

// CLongitudeSchema: for %LONG1, %LONG2
class CLongitudeSchema: public CSchema
{
public:
	CLongitudeSchema(int delta)	: m_delta(delta) {};
	virtual std::wstring interpret(unsigned char l, unsigned long x, unsigned long y);

protected:
	int m_delta;
};

// ---------------------------------------------------------------

// CLatitudeSchema: for %LAT1, %LAT2
class CLatitudeSchema: public CSchema
{
public:
	CLatitudeSchema(int delta) : m_delta(delta)	{};
	virtual std::wstring interpret(unsigned char l, unsigned long x, unsigned long y);

protected:
	int m_delta;
};

// ---------------------------------------------------------------

// CXSchema: for %X
class CXSchema: public CSchema
{
public:
	virtual std::wstring interpret(unsigned char l, unsigned long x, unsigned long y)
	{
		wchar_t buffer[32];
		swprintf(buffer, 32, L"%d", x);
		return buffer;
	};
};

// ---------------------------------------------------------------

// CYSchema: for %Y
class CYSchema: public CSchema
{
public:
	virtual std::wstring interpret(unsigned char l, unsigned long x, unsigned long y)
	{
		wchar_t buffer[32];
		swprintf(buffer, 32, L"%d", y);
		return buffer;
	};
};

// ---------------------------------------------------------------

// CZoomSchema: for %ZOOM_17, %ZOOM_00, %ZOOM_01
class CZoomSchema: public CSchema
{
public:
	CZoomSchema(int factor, int delta) : m_factor(factor), m_delta(delta) {};
	virtual std::wstring interpret(unsigned char l, unsigned long x, unsigned long y)
	{
		wchar_t buffer[32];
		swprintf(buffer, 32, L"%d", m_factor * l + m_delta);
		return buffer;
	};
protected:
	int m_factor, m_delta;
};

// ---------------------------------------------------------------

// %TMSY: Origin in the bottom-left corner (where %Y has its origin in the top left corner)
class CTMSYSchema: public CSchema
{
public:
	virtual std::wstring interpret(unsigned char l, unsigned long x, unsigned long y)
	{
		wchar_t buffer[32];
		swprintf(buffer, 32, L"%d", (1<<(17-l))-1-y);
		return buffer;
	};
};

// ---------------------------------------------------------------

// For %QRST
class CQRSTSchema: public CSchema
{
public:
	CQRSTSchema(int maxChar, int firstChar) : m_MaxChar(maxChar), m_firstChar(firstChar)	{};
	virtual std::wstring interpret(unsigned char l, unsigned long x, unsigned long y)
	{
		return GoogleXYZ17toQRSTW(x, y, l, m_MaxChar, m_firstChar);
	};

protected:
	// Maximal length of the interpreted value (0 for the whole value):
	int m_MaxChar;
	// First char to use (0 to start at the beginning):
	int m_firstChar;
};

// ---------------------------------------------------------------

// For %QKEY
class CQKeySchema: public CSchema
{
public:
	CQKeySchema(int maxChar, int firstChar) : m_MaxChar(maxChar), m_firstChar(firstChar)	{};
	virtual std::wstring interpret(unsigned char l, unsigned long x, unsigned long y)
	{
		return GoogleXYZ17toQKeyW(x, y, l, m_MaxChar, m_firstChar);
	};

protected:
	// Maximal length of the interpreted value (0 for the whole value):
	int m_MaxChar;
	// First char to use (0 to start at the beginning):
	int m_firstChar;
};

// ---------------------------------------------------------------

#endif // VARIABLEINTERPRETER_H
