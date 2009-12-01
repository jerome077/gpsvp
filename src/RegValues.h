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
#ifndef LINUX
#	include <windows.h>
#endif

#include "Lock.h"

#ifndef LINUX

	class CRegBase
	{
	public:
		virtual ~CRegBase();
	protected:
		HKEY m_hKey;
		std::tstring m_wstrKey;
		void Init(HKEY hKey, const tchar_t * wcKey)
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
		void Init(HKEY hKey, const tchar_t * wcKey, T wcDefault)
		{
			AutoLock l;
			CRegBase::Init(hKey, wcKey);
			T buff;
			DWORD lLen = sizeof(buff);
			DWORD lType;
			if (0 != RegQueryValueEx(m_hKey, m_wstrKey.c_str(), 0, &lType, (unsigned char *)&buff, &lLen))
			{
				if (0 != RegQueryValueEx(m_hKey, (m_wstrKey + T("Def")).c_str(), 0, &lType, (unsigned char *)&buff, &lLen))
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
		std::tstring m_wstrValue;
	public:
		void Init(HKEY hKey, const tchar_t * wcKey);
		void operator =(const tchar_t * wcValue);
		const std::tstring operator()(void);
	};

	inline void MyRegSetValueString(HKEY hKey, const tchar_t * wcName, const tchar_t * wcValue)
	{
		RegSetValueEx(hKey, wcName, 0, REG_SZ, (BYTE *)wcValue, wcslen(wcValue) * sizeof(*wcValue));
	}

	inline void RegisterFileType(const tchar_t * wcExtention, const tchar_t * wcDescription, 
		const tchar_t * wcFileType, const tchar_t * wcProgName)
	{
		HKEY hKey;
		RegCreateKeyEx(HKEY_CLASSES_ROOT, wcExtention, 0, T(""), 0, 0, 0, &hKey, 0);
		MyRegSetValueString(hKey, NULL, wcFileType);
		RegCreateKeyEx(HKEY_CLASSES_ROOT, wcFileType, 0, T(""), 0, 0, 0, &hKey, 0);
		MyRegSetValueString(hKey, NULL, wcDescription);
		RegCreateKeyEx(HKEY_CLASSES_ROOT, (std::tstring(wcFileType) + T("\\shell\\open\\command")).c_str(), 
			0, T(""), 0, 0, 0, &hKey, 0);
		MyRegSetValueString(hKey, NULL, (std::tstring(T("\"")) + wcProgName + T("\" \"%1\"")).c_str());
		RegCreateKeyEx(HKEY_CLASSES_ROOT, (std::tstring(wcFileType) + T("\\DefaultIcon")).c_str(), 
			0, T(""), 0, 0, 0, &hKey,0);
		MyRegSetValueString(hKey, NULL, (std::tstring(T("")) + wcProgName + T(",0")).c_str());
	}
#else
	typedef void* HKEY;
	class CRegString
	{
	private:
		std::tstring Value;
	public:
		void Init(HKEY, const tchar_t* value) { Value = value;}
		const std::tstring& operator()() const {return Value;}
		CRegString& operator =(const std::tstring& value) {Value = value;}
	};

	enum RegType {
		REG_BINARY
	};
	template <class T, RegType type>
	class CRegScalar
	{
	private:
		T Value;
	public:
		void Set(const T& value) {Value = value;};
		const T& operator()() {return Value;}
		void Init(HKEY hKey, const tchar_t * wcKey, T wcDefault) {Value = wcDefault;}
	};
#endif

#endif // REGVALUES_H
