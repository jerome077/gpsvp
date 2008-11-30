/*
Copyright (c) 2005-2008
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "VariableInterpreter.h"
#include <cassert>
#include <math.h>

const long LEVEL_REVERSE_OFFSET = 18;

// ---------------------------------------------------------------

// Test function for the class CStringSchema
void Test_CStringSchema()
{
	CStringSchema S1;
	assert(S1.empty());
	S1.assign(L"");
	assert(S1.empty());
	S1.assign(L"%X");
	assert(!S1.empty());
	assert(S1.interpret(3, 8734, 5710) == L"8734");
	S1.assign(L"abc%Xdef%Yabc%X%Ydef");
	assert(!S1.empty());
	assert(S1.interpret(3, 8734, 5710) == L"abc8734def5710abc87345710def");
	S1.assign(L"%LONG1,%LAT1,%LONG2,%LAT2");
	assert(!S1.empty());
	assert(S1.interpret(3, 8734, 5710) == L"11.90917968750000,47.76886840424206,11.93115234375000,47.78363463526377");	
	S1.assign(L"http://a.X.b/%ZOOM_17/%ZOOM_00/%ZOOM_01");
	assert(!S1.empty());
	assert(S1.interpret(3, 8734, 5710) == L"http://a.X.b/3/14/15");	
	S1.assign(L"http://a.X.b/%NOT_EXISTING_VAR/123%");
	assert(!S1.empty());
	assert(S1.interpret(3, 8734, 5710) == L"http://a.X.b/%NOT_EXISTING_VAR/123%");	
	S1.assign(L"%5");
	assert(!S1.empty());
	assert(S1.interpret(3, 8734, 5710) == L"%5");	
	S1.assign(L"http://a.X.b/%TMSX/%TMSY");
	assert(!S1.empty());
	assert(S1.interpret(3, 8734, 5710) == L"http://a.X.b/8734/10673");	
	S1.assign(L"http://a.X.b/%QRST/%5QRST");
	assert(!S1.empty());
	assert(S1.interpret(3, 8734, 5710) == L"http://a.X.b/trtqtsqqtqrsssq/trtqt");	
	S1.assign(L"http://a.X.b/%5QKEY/%QKEY/");
	assert(!S1.empty());
	assert(S1.interpret(3, 8734, 5710) == L"http://a.X.b/12023/12023002013330/");	
}

// ---------------------------------------------------------------

// Returns the variable length if the variable is at the given position, else returns 0.
size_t CheckVariableAndGetLength(const std::wstring& strSchema, size_t pos, const std::wstring& varName)
{
	size_t varLen = varName.length();
	if (0 == strSchema.compare(pos, varLen, varName))
		return varLen;
	else
		return 0;
}

// ---------------------------------------------------------------

// Like 'CheckVariableAndGetLength' but for variables where you can limit the char count: %5QRST
size_t CheckExtendedVariable(const std::wstring& strSchema, size_t pos, const std::wstring& varName, int& maxCharCount)
{
	size_t varLen = varName.length();
	if (0 == strSchema.compare(pos, varLen, varName))
	{
		maxCharCount = 0; // 0 means no length limit
		return varLen;
	}
	else if ( (pos+1<strSchema.length()) && (0 == strSchema.compare(pos+1, varLen, varName)) )
	{
		maxCharCount = _wtoi(strSchema.substr(pos, 1).c_str());
		if (maxCharCount > 0)
			return varLen+1;
		else
			return 0;
	}
	else if ( (pos+2<strSchema.length()) && (0 == strSchema.compare(pos+2, varLen, varName)) )
	{
		maxCharCount = _wtoi(strSchema.substr(pos, 2).c_str());
		if (maxCharCount > 0)
			return varLen+2;
		else
			return 0;
	}
	else
		return 0;
}

// ---------------------------------------------------------------

void CStringSchema::clear()
{
	TPSchemaParts::iterator it;
	for (it = m_SchemaParts.begin(); it != m_SchemaParts.end(); ++it)
	{
		delete *it;
	}
	m_SchemaParts.clear();
}

// ---------------------------------------------------------------

void CStringSchema::assign(const std::wstring& strSchema)
{
	clear();
	size_t pos0 = 0;
	size_t found = strSchema.find(L"%");
	while (std::wstring::npos != found)
	{
		size_t varlen;
		int maxCharCount;
		if (0 != (varlen = CheckVariableAndGetLength(strSchema, found+1, L"LONG1")))
		{
			m_SchemaParts.push_back( new CSimpleStringSchema(strSchema.substr(pos0, found-pos0)) );
			m_SchemaParts.push_back( new CLongitudeSchema(0) ); // west
		}
		else if (0 != (varlen = CheckVariableAndGetLength(strSchema, found+1, L"LONG2"))) //east
		{
			m_SchemaParts.push_back( new CSimpleStringSchema(strSchema.substr(pos0, found-pos0)) );
			m_SchemaParts.push_back( new CLongitudeSchema(1) ); // east
		}
		else if (0 != (varlen = CheckVariableAndGetLength(strSchema, found+1, L"LAT1")))
		{
			m_SchemaParts.push_back( new CSimpleStringSchema(strSchema.substr(pos0, found-pos0)) );
			m_SchemaParts.push_back( new CLatitudeSchema(1) ); // north
		}
		else if (0 != (varlen = CheckVariableAndGetLength(strSchema, found+1, L"LAT2")))
		{
			m_SchemaParts.push_back( new CSimpleStringSchema(strSchema.substr(pos0, found-pos0)) );
			m_SchemaParts.push_back( new CLatitudeSchema(0) ); // south
		}
		else if (0 != (varlen = CheckVariableAndGetLength(strSchema, found+1, L"X")))
		{
			m_SchemaParts.push_back( new CSimpleStringSchema(strSchema.substr(pos0, found-pos0)) );
			m_SchemaParts.push_back( new CXSchema() );
		}
		else if (0 != (varlen = CheckVariableAndGetLength(strSchema, found+1, L"Y")))
		{
			m_SchemaParts.push_back( new CSimpleStringSchema(strSchema.substr(pos0, found-pos0)) );
			m_SchemaParts.push_back( new CYSchema() );
		}
		else if (0 != (varlen = CheckVariableAndGetLength(strSchema, found+1, L"ZOOM_17")))
		{
			m_SchemaParts.push_back( new CSimpleStringSchema(strSchema.substr(pos0, found-pos0)) );
			m_SchemaParts.push_back( new CZoomSchema(1, 0) ); // data.level
		}
		else if (0 != (varlen = CheckVariableAndGetLength(strSchema, found+1, L"ZOOM_00")))
		{
			m_SchemaParts.push_back( new CSimpleStringSchema(strSchema.substr(pos0, found-pos0)) );
			m_SchemaParts.push_back( new CZoomSchema(-1, LEVEL_REVERSE_OFFSET-1) ); // 17-data.level
		}
		else if (0 != (varlen = CheckVariableAndGetLength(strSchema, found+1, L"ZOOM_01")))
		{
			m_SchemaParts.push_back( new CSimpleStringSchema(strSchema.substr(pos0, found-pos0)) );
			m_SchemaParts.push_back( new CZoomSchema(-1, LEVEL_REVERSE_OFFSET) ); // 18-data.level
		}
		else if (0 != (varlen = CheckVariableAndGetLength(strSchema, found+1, L"TMSX")))
		{
			m_SchemaParts.push_back( new CSimpleStringSchema(strSchema.substr(pos0, found-pos0)) );
			m_SchemaParts.push_back( new CXSchema() );
		}
		else if (0 != (varlen = CheckVariableAndGetLength(strSchema, found+1, L"TMSY")))
		{
			m_SchemaParts.push_back( new CSimpleStringSchema(strSchema.substr(pos0, found-pos0)) );
			m_SchemaParts.push_back( new CTMSYSchema() );
		}
		else if (0 != (varlen = CheckExtendedVariable(strSchema, found+1, L"QRST", maxCharCount)))
		{
			m_SchemaParts.push_back( new CSimpleStringSchema(strSchema.substr(pos0, found-pos0)) );
			m_SchemaParts.push_back( new CQRSTSchema(maxCharCount) );
		}
		else if (0 != (varlen = CheckExtendedVariable(strSchema, found+1, L"QKEY", maxCharCount)))
		{
			m_SchemaParts.push_back( new CSimpleStringSchema(strSchema.substr(pos0, found-pos0)) );
			m_SchemaParts.push_back( new CQKeySchema(maxCharCount) );
		}
		else
		{
			m_SchemaParts.push_back( new CSimpleStringSchema(strSchema.substr(pos0, found-pos0+1)) );
		}
		pos0 = found + varlen + 1;
		found = strSchema.find(L"%", pos0);
	}
	if (pos0 < strSchema.length())
		m_SchemaParts.push_back( new CSimpleStringSchema(strSchema.substr(pos0, strSchema.length())) );
}

// ---------------------------------------------------------------

std::wstring CStringSchema::interpret(unsigned char l, unsigned long x, unsigned long y)
{
	TPSchemaParts::iterator it;
	std::wstring Result = L"";
	for (it = m_SchemaParts.begin(); it != m_SchemaParts.end(); ++it)
	{
		Result += (*it)->interpret(l, x, y);
	}
	return Result;
};

// ---------------------------------------------------------------

// Calculation derived from here: http://www.supware.net/GoogleMapping/ 
double XtoLong(unsigned long x, unsigned char level)
{
    return 360.0*x/(1<<(17-level))-180;
}

double YtoLat(unsigned long y, unsigned char level)
{
	return (atan(exp(3.14159265358979323846*(1.0-256.0*y/(1<<(24-level)))))/3.14159265358979323846-0.25)*360;
}

// ---------------------------------------------------------------

std::wstring CLongitudeSchema::interpret(unsigned char l, unsigned long x, unsigned long y)
{
	wchar_t buffer[32];
	double Long1 = XtoLong(x + m_delta, l);
	swprintf(buffer, 32, L"%.14f", Long1);
	return buffer;
};

// ---------------------------------------------------------------

std::wstring CLatitudeSchema::interpret(unsigned char l, unsigned long x, unsigned long y)
{
	wchar_t buffer[32];
	double Lat1 = YtoLat(y + m_delta, l);
	swprintf(buffer, 32, L"%.14f", Lat1);
	return buffer;
};

// ---------------------------------------------------------------

std::wstring CQRSTSchema::interpret(unsigned char l, unsigned long x, unsigned long y)
{
	long NumX = x;
	long NumY = y;
	long level = LEVEL_REVERSE_OFFSET - l;
	long d = 1 << (level - 1);

	if ((NumX < 0) || (NumX > (d-1))) {
		NumX = NumX % d;
		if (NumX < 0) {
			NumX += d;
		}
	}

	wchar_t buf[24];
	buf[0] = L't';

	for (long nPos = 1; nPos < level; nPos++) {
	    d >>= 1;
		if (NumY < d) {
			if (NumX < d) {
				buf[nPos] = L'q';
			} else {
				buf[nPos] = L'r';
				NumX -= d;
			}
		} else {
			if (NumX < d) {
				buf[nPos] = L't';
			} else { 
				buf[nPos] = L's';
				NumX -= d;
			}
			NumY -= d;
		}
	}
	buf[level] = L'\0';
	if ((m_MaxChar > 0) && (m_MaxChar < 24))
		buf[m_MaxChar] = L'\0';
	return buf;
}

// ---------------------------------------------------------------

std::wstring CQKeySchema::interpret(unsigned char l, unsigned long x, unsigned long y)
{
	long NumX = x;
	long NumY = y;
	long level = LEVEL_REVERSE_OFFSET - l;
	long d = 1 << (level - 1);

	if ((NumX < 0) || (NumX > (d-1))) {
		NumX = NumX % d;
		if (NumX < 0) {
			NumX += d;
		}
	}

	wchar_t buf[24];

	for (long nPos = 0; nPos < (level-1); nPos++) {
	    d >>= 1;
		if (NumY < d) {
			if (NumX < d) {
				buf[nPos] = L'0';
			} else {
				buf[nPos] = L'1';
				NumX -= d;
			}
		} else {
			if (NumX < d) {
				buf[nPos] = L'2';
			} else { 
				buf[nPos] = L'3';
				NumX -= d;
			}
			NumY -= d;
		}
	}
	buf[level-1] = L'\0';
	if ((m_MaxChar > 0) && (m_MaxChar < 24))
		buf[m_MaxChar] = L'\0';
	return buf;
}

// ---------------------------------------------------------------
