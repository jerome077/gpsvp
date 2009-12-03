/*
Copyright (c) 2005-2008, Jerome077 and others
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "VariableInterpreter.h"
#include <cassert>

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
	//assert(S1.interpret(3, 8734, 5710) == L"11.90917968750000,47.76886840424206,11.93115234375000,47.78363463526377");	
	assert(S1.interpret(3, 8734, 5710) == L"11.90917968750000,47.76886840424218,11.93115234375000,47.78363463526387");	
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
	S1.assign(L"http://a.X.b/%QRST/%5QRST/%4,6QRST/%5,13QRST/%2,17QRST/");
	assert(!S1.empty());
	assert(S1.interpret(3, 8734, 5710) == L"http://a.X.b/trtqtsqqtqrsssq/trtqt/sqqt/ssq//");	
	S1.assign(L"http://a.X.b/%5QKEY/%QKEY/%4,7QKEY");
	assert(!S1.empty());
	assert(S1.interpret(3, 8734, 5710) == L"http://a.X.b/12023/12023002013330/0201");	
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

int GetIntegerAndIncPos(const std::wstring& strSchema, size_t& pos)
{
	// Works only between 1 and 99...
	int result = _wtoi(strSchema.c_str()+pos);
	if (result >= 1) ++pos;
	if (result >= 10) ++pos;
	return result;
}

bool GetSeparatorAndIncPos(const std::wstring& strSchema, size_t& pos)
{
	if (L',' == strSchema[pos])
	{
		++pos;
		return true;
	}
	else
		return false;
}

bool GetVarNameAndIncPos(const std::wstring& strSchema, size_t& pos, const std::wstring& varName)
{
	size_t varLen = varName.length();
	if (0 == strSchema.compare(pos, varLen, varName))
	{
		pos += varLen;
		return true;
	}
	else
		return false;
}

// Like 'CheckVariableAndGetLength' but for variables where you can limit the char count: %5QRST
// an also give a start position like %5,7QRST
size_t CheckExtendedVariable(const std::wstring& strSchema, size_t pos, const std::wstring& varName,
							 int& maxCharCount, int& firstChar)
{
	size_t currentPos = pos;

	// Max character count:
	maxCharCount = GetIntegerAndIncPos(strSchema, currentPos);

	// First position:
	if (GetSeparatorAndIncPos(strSchema, currentPos))
	{
		firstChar = GetIntegerAndIncPos(strSchema, currentPos);
		if (firstChar > 0) --firstChar;  // C++ begins to count with 0
	}
	else
		firstChar = 0;

	// Variable present:
	if (GetVarNameAndIncPos(strSchema, currentPos, varName))
		return (currentPos-pos);
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
		int maxCharCount, firstChar;
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
			m_SchemaParts.push_back( new CLatitudeSchema(1) ); // south
		}
		else if (0 != (varlen = CheckVariableAndGetLength(strSchema, found+1, L"LAT2")))
		{
			m_SchemaParts.push_back( new CSimpleStringSchema(strSchema.substr(pos0, found-pos0)) );
			m_SchemaParts.push_back( new CLatitudeSchema(0) ); // north
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
		else if (0 != (varlen = CheckExtendedVariable(strSchema, found+1, L"QRST", maxCharCount, firstChar)))
		{
			m_SchemaParts.push_back( new CSimpleStringSchema(strSchema.substr(pos0, found-pos0)) );
			m_SchemaParts.push_back( new CQRSTSchema(maxCharCount, firstChar) );
		}
		else if (0 != (varlen = CheckExtendedVariable(strSchema, found+1, L"QKEY", maxCharCount, firstChar)))
		{
			m_SchemaParts.push_back( new CSimpleStringSchema(strSchema.substr(pos0, found-pos0)) );
			m_SchemaParts.push_back( new CQKeySchema(maxCharCount, firstChar) );
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


std::wstring CLongitudeSchema::interpret(unsigned char l, unsigned long x, unsigned long y)
{
	wchar_t buffer[32];
	double Long1 = GoogleXZ17toLong(x + m_delta, l);
	swprintf(buffer, 32, L"%.14f", Long1);
	return buffer;
};

// ---------------------------------------------------------------

std::wstring CLatitudeSchema::interpret(unsigned char l, unsigned long x, unsigned long y)
{
	wchar_t buffer[32];
	double Lat1 = GoogleYZ17toLat(y + m_delta, l);
	swprintf(buffer, 32, L"%.14f", Lat1);
	return buffer;
};

// ---------------------------------------------------------------