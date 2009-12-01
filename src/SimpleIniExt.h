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
	// -----------------------------------------------------------------------
	int GetIntValue(const tchar_t * a_pSection,
                    const tchar_t * a_pKey,
                    const tchar_t * a_pDefault = NULL)
	{
		return _wtoi(GetValue(a_pSection, a_pKey, a_pDefault));
	};

	// -----------------------------------------------------------------------
	double GetDoubleValue(const tchar_t * a_pSection,
                          const tchar_t * a_pKey,
                          const tchar_t * a_pDefault = NULL)
	{
		// _wtof is not available on Windows Mobile, that why I must first convert the std::tstring.
		std::tstring wsVal = GetValue(a_pSection, a_pKey, a_pDefault);
		std::string sVal;
		sVal.assign(wsVal.begin(), wsVal.end());
		return atof(sVal.c_str());
	};

	// -----------------------------------------------------------------------
    // Extended Version of LoadFile that automatically recognize if the file is ANSI or UTF-8.
	// (Using the BOM, see http://en.wikipedia.org/wiki/Byte-order_mark)
	SI_Error LoadAnsiOrUtf8File(const SI_WCHAR_T * a_pwszFile)
	{
	#ifdef _WIN32
	    FILE * fp = NULL;
	#if __STDC_WANT_SECURE_LIB__ && !_WIN32_WCE
	    _wfopen_s(&fp, a_pwszFile, T("rb"));
	#else // !__STDC_WANT_SECURE_LIB__
	    fp = _wfopen(a_pwszFile, T("rb"));
	#endif // __STDC_WANT_SECURE_LIB__
	    if (!fp) return SI_FILE;
	    SI_Error rc = LoadAnsiOrUtf8File(fp);
	    fclose(fp);
	    return rc;
	#else // !_WIN32 (therefore SI_CONVERT_ICU)
	    char szFile[256];
	    u_austrncpy(szFile, a_pwszFile, sizeof(szFile));
	    return LoadAnsiOrUtf8File(szFile);
	#endif // _WIN32
	};

	SI_Error LoadAnsiOrUtf8File(FILE * a_fpFile)
	{
	    // load the raw file data
	    int retval = fseek(a_fpFile, 0, SEEK_END);
	    if (retval != 0) {
			return SI_FILE;
		}
		long lSize = ftell(a_fpFile);
		if (lSize < 0) {
	        return SI_FILE;
	    }
	    if (lSize == 0) {
			return SI_OK;
		}
		char * pData = new char[lSize];
		if (!pData) {
	        return SI_NOMEM;
	    }
	    fseek(a_fpFile, 0, SEEK_SET);
	    size_t uRead = fread(pData, sizeof(char), lSize, a_fpFile);
	    if (uRead != (size_t) lSize) {
			delete[] pData;
			return SI_FILE;
		}
	
	    // convert the raw data to unicode
        if (uRead >= 3) {
			if (memcmp(pData, SI_UTF8_SIGNATURE, 3) == 0) {
				SetUnicode(true);
			}
		}		
		SI_Error rc = Load(pData, uRead);
	    delete[] pData;
	    return rc;
	};
	// -----------------------------------------------------------------------
};

#endif // SIMPLEINIEXT_H
