#ifndef REGVALUES_H
#define REGVALUES_H

#include <string>
#include <windows.h>

#include "Lock.h"

using namespace std;

class CRegBase
{
public:
	virtual ~CRegBase();
protected:
	HKEY m_hKey;
	wstring m_wstrKey;
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
		unsigned long lLen = sizeof(buff);
		unsigned long lType;
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
	wstring m_wstrValue;
public:
	void Init(HKEY hKey, const wchar_t * wcKey);
	void operator =(const wchar_t * wcValue);
	const wstring operator()(void);
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
	RegCreateKeyEx(HKEY_CLASSES_ROOT, (wstring(wcFileType) + L"\\shell\\open\\command").c_str(), 
		0, L"", 0, 0, 0, &hKey, 0);
	MyRegSetValueString(hKey, NULL, (wstring(L"\"") + wcProgName + L"\" \"%1\"").c_str());
	RegCreateKeyEx(HKEY_CLASSES_ROOT, (wstring(wcFileType) + L"\\DefaultIcon").c_str(), 
		0, L"", 0, 0, 0, &hKey,0);
	MyRegSetValueString(hKey, NULL, (wstring(L"") + wcProgName + L",0").c_str());
}

#endif // REGVALUES_H
