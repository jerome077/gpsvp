﻿/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "RegValues.h"

CRegBase::~CRegBase()
{
}

void CRegString::Init(HKEY hKey, const wchar_t * wcKey)
{
	AutoLock l;
	CRegBase::Init(hKey, wcKey);
	wchar_t buff[1000] = {0};
	DWORD lLen = sizeof(buff);
	DWORD lType;
	if (ERROR_SUCCESS == RegQueryValueEx(m_hKey, m_wstrKey.c_str(), 0, &lType, (unsigned char *)&buff, &lLen))
	{
		buff[lLen] = 0;
		m_wstrValue = buff;
	} 
	else if (ERROR_SUCCESS == RegQueryValueEx(m_hKey, (m_wstrKey + L"Def").c_str(), 0, &lType, (unsigned char *)&buff, &lLen))
	{
		buff[lLen] = 0;
		m_wstrValue = buff;
	}
}

void CRegString::operator =(const wchar_t * wcValue)
{
	AutoLock l;
	m_wstrValue = wcValue;
	RegSetValueEx(m_hKey, m_wstrKey.c_str(), 0, REG_SZ, (unsigned char *)wcValue, sizeof(wchar_t) * (wcslen(wcValue) + 1));
}

const std::wstring CRegString::operator()(void) 
{
	AutoLock l;
	return m_wstrValue;
}
