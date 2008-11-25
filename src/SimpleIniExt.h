/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// Extension of SimpleIni to convert value to int or double...

#ifndef SIMPLEINIEXT_H
#define SIMPLEINIEXT_H

#include "SimpleIni.h"

class CSimpleIniExtW : public CSimpleIniW
{
public:
	int GetIntValue(const wchar_t * a_pSection,
                    const wchar_t * a_pKey,
                    const wchar_t * a_pDefault = NULL)
	{
		return _wtoi(GetValue(a_pSection, a_pKey, a_pDefault));
	};

	double GetDoubleValue(const wchar_t * a_pSection,
                          const wchar_t * a_pKey,
                          const wchar_t * a_pDefault = NULL)
	{
		// _wtof is not available on Windows Mobile, that why I must first convert the wstring.
		std::wstring wsVal = GetValue(a_pSection, a_pKey, a_pDefault);
		std::string sVal;
		sVal.assign(wsVal.begin(), wsVal.end());
		return atof(sVal.c_str());
	};
};

#endif // SIMPLEINIEXT_H
