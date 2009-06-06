/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef REGVALUES_H
#define REGVALUES_H

#include <string>
#include <windows.h>

#include "Lock.h"

class CRegBase
{
public:
	virtual ~CRegBase();
protected:
	HKEY m_hKey;
	std::wstring m_wstrKey;
	void Init(HKEY hKey, const wchar_t * wcKey)
	{
		m_hKey = hKey;
		m_wstrKey = wcKey;
	}
private:
	void operator = (const CRegBase &);
};

template <class T, DWORD type>
class CRegScalar : private CRegBase
{
private:
	T m_Value;
	void operator = (const CRegScalar<T, type> &);
public:
	void Init(HKEY hKey, const wchar_t * wcKey, T wcDefault)
	{
		AutoLock l;
		CRegBase::Init(hKey, wcKey);
		T buff;
		unsigned int lLen = sizeof(buff);
		unsigned int lType;
		if (0 != RegQueryValueEx(m_hKey, m_wstrKey.c_str(), 0, &lType, (unsigned char *)&buff, &lLen))
		{
			if (0 != RegQueryValueEx(m_hKey, (m_wstrKey + L"Def").c_str(), 0, &lType, (unsigned char *)&buff, &lLen))
			{
				m_Value = wcDefault;
				return;
			}
		}
		{
			if (lLen == sizeof(buff))
			{
				m_Value = buff;
				return;
			}
		}
	}
	void Set(T value)
	{
		AutoLock l;
		m_Value = value;
		RegSetValueEx(m_hKey, m_wstrKey.c_str(), 0, type, (unsigned char *)&m_Value, sizeof(m_Value));
	}
	const T operator()()
	{
		AutoLock l;
		return m_Value;
	}
	bool operator !()
	{
		AutoLock l;
		return !m_Value;
	}
};

class CRegString : private CRegBase
{
	std::wstring m_wstrValue;
public:
	void Init(HKEY hKey, const wchar_t * wcKey);
	void operator =(const wchar_t * wcValue);
	const std::wstring operator()(void);
};

inline void MyRegSetValueString(HKEY hKey, const wchar_t * wcName, const wchar_t * wcValue)
{
	RegSetValueEx(hKey, wcName, 0, REG_SZ, (BYTE *)wcValue, wcslen(wcValue) * sizeof(*wcValue));
}

inline void RegisterFileType(const wchar_t * wcExtention, const wchar_t * wcDescription, 
	const wchar_t * wcFileType, const wchar_t * wcProgName)
{
	HKEY hKey;
	RegCreateKeyEx(HKEY_CLASSES_ROOT, wcExtention, 0, L"", 0, 0, 0, &hKey, 0);
	MyRegSetValueString(hKey, NULL, wcFileType);
	RegCreateKeyEx(HKEY_CLASSES_ROOT, wcFileType, 0, L"", 0, 0, 0, &hKey, 0);
	MyRegSetValueString(hKey, NULL, wcDescription);
	RegCreateKeyEx(HKEY_CLASSES_ROOT, (std::wstring(wcFileType) + L"\\shell\\open\\command").c_str(), 
		0, L"", 0, 0, 0, &hKey, 0);
	MyRegSetValueString(hKey, NULL, (std::wstring(L"\"") + wcProgName + L"\" \"%1\"").c_str());
	RegCreateKeyEx(HKEY_CLASSES_ROOT, (std::wstring(wcFileType) + L"\\DefaultIcon").c_str(), 
		0, L"", 0, 0, 0, &hKey,0);
	MyRegSetValueString(hKey, NULL, (std::wstring(L"") + wcProgName + L",0").c_str());
}

#endif // REGVALUES_H
