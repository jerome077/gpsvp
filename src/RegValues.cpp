#include "RegValues.h"

CRegBase::~CRegBase()
{
}

void CRegString::Init(HKEY hKey, const wchar_t * wcKey)
{
	AutoLock l;
	CRegBase::Init(hKey, wcKey);
	wchar_t buff[1000] = {0};
	unsigned long lLen = sizeof(buff);
	unsigned long lType;
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

const wstring CRegString::operator()(void) 
{
	AutoLock l;
	return m_wstrValue;
}
