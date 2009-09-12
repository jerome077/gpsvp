/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef MONITORS_H
#define MONITORS_H

#include "GDIPainter.h"
#include "RegValues.h"

class CNMEAParser;

class CMonitorBase : public IMonitor
{
protected:
	std::wstring m_wstrLabel;
	std::wstring m_wstrName;
	bool m_fSet;
	enum {cnResetCmd = 1000, cnToggleCmd, cnSetTimeCmd, cnShowDateCmd, cnCopyText};
public:
	CMonitorBase() : m_fSet(false) {};
	void SetIdL(wchar_t * wcName)
	{
		m_wstrLabel = GetDict().Translate(wcName);
		m_wstrName = wcName;
	}
	void SetIdL(const std::wstring& wstrName, const std::wstring& wstrLabel)
	{
		m_wstrLabel = wstrLabel;
		m_wstrName = wstrName;
	}
	virtual const wchar_t * GetId()
	{
		return m_wstrName.c_str();
	}
	void Reset()
	{
		m_fSet = false;
	}
	virtual void PrepareContextMenu(IListAcceptor * pMenu)
	{
	}
	virtual void ProcessMenuCommand(int i)
	{
	}
	bool IsSet()
	{
		return m_fSet;
	}
};

class CTimeMonitor : public CMonitorBase
{
private:
	int m_iYear, m_iMonth, m_iDay, m_iHour, m_iMinute, m_iSecond;
	bool m_fSet;
	bool m_fShowDate;
public:
	CTimeMonitor() : m_fShowDate(false) {}
	virtual void Paint(IMonitorPainter * pPainter)
	{
		wchar_t buff[20];
		if (m_fSet)
		{
			TIME_ZONE_INFORMATION tz;
			DWORD dwTz = GetTimeZoneInformation(&tz);
			int iBias = tz.Bias + ((dwTz == TIME_ZONE_ID_DAYLIGHT) ? tz.DaylightBias : tz.StandardBias);
			int iHour = m_iHour;
			int iMinute = m_iMinute;
			iMinute -= iBias;
			iHour += ((iMinute - ((iMinute + 60 * 24) % 60)) / 60);
			iHour = (iHour + 24) % 24;
			iMinute = (iMinute + 60 * 24) %60;

			if (m_fShowDate)
				swprintf(buff, 20, L"%04d.%02d.%02d", m_iYear, m_iMonth, m_iDay);
			else
				swprintf(buff, 20, L"%02d:%02d:%02d", iHour, iMinute, m_iSecond);
		}
		else
			swprintf(buff, 20, L"-");
		pPainter->DrawTextMonitor(m_wstrLabel.c_str(), buff);
	}
	void Set(double dHour)
	{
		int iHour = int(dHour);
		double dMinute = (dHour - iHour) * 60;
		int iMinute = int(dMinute);
		int iSecond = int((dMinute - iMinute) * 60);
		Set(0, 0, 0, iHour, iMinute, iSecond);
	}
	// UTC time
	void Set(int iYear, int iMonth, int iDay, int iHour, int iMinute, int iSecond)
	{
		m_iYear = iYear;
		m_iMonth = iMonth;
		m_iDay = iDay;
		m_iHour = iHour;
		m_iMinute = iMinute;
		m_iSecond = iSecond;
		m_fSet = true;
	}
	// UTC time
	bool Get(int &iYear, int &iMonth, int &iDay, int &iHour, int &iMinute, int &iSecond) const
	{
		if (!m_fSet)
			return false;
		iYear = m_iYear;
		iMonth = m_iMonth;
		iDay = m_iDay;
		iHour = m_iHour;
		iMinute = m_iMinute;
		iSecond = m_iSecond;
		return true;
	}
	void Reset()
	{
		m_fSet = false;
	}
	virtual void PrepareContextMenu(IListAcceptor * pMenu);
	virtual void ProcessMenuCommand(int i)
	{
		if (i == cnSetTimeCmd)
			SetTime();
		if (i == cnShowDateCmd)
		{
			m_fShowDate = !m_fShowDate;
		}
	}
	void SetTime()
	{
		if (m_fSet)
		{
			SYSTEMTIME st;
			ZeroMemory(&st, sizeof(st));
			GetSystemTime(&st);
			
			st.wYear = m_iYear;
			st.wMonth = m_iMonth;
			st.wDay = m_iDay;
			st.wHour = m_iHour;
			st.wMinute = m_iMinute;
			st.wSecond = m_iSecond;
			st.wMilliseconds = 0;
			int res = SetSystemTime(&st);
		}
	}
};

class CMemoryMonitor : public CMonitorBase
{
private:
	unsigned long m_ulValue;
public:
	virtual void Paint(IMonitorPainter * pPainter)
	{
		pPainter->DrawTextMonitor(m_wstrLabel.c_str(), m_fSet ? MemoryToText(m_ulValue).c_str() : L"-");
	}
	CMemoryMonitor & operator = (unsigned long ul)
	{
		m_ulValue = ul;
		m_fSet = true;
		return *this;
	}
	CMemoryMonitor & operator += (unsigned long ul)
	{
		if (m_fSet)
			m_ulValue += ul;
		return *this;
	}
};

class CPercentMonitor : public CMonitorBase
{
private:
	byte m_bValue;
public:
	virtual void Paint(IMonitorPainter * pPainter)
	{
		wchar_t buffer[10];
		swprintf(buffer, 10, L"%d%%", m_bValue);
		pPainter->DrawTextMonitor(m_wstrLabel.c_str(), m_fSet ? buffer : L"-");
	}
	CPercentMonitor & operator = (byte b)
	{
		m_bValue = b;
		m_fSet = true;
		return *this;
	}
};

class CTextMonitor : public CMonitorBase
{
public:
	enum TEXTCOPY_SOURCES {
		TEXTCOPY_SRC_NONE = 0,
		TEXTCOPY_SRCURL
	};

private:
	std::wstring m_wstrValue;
	TEXTCOPY_SOURCES m_enTCSrc;
	std::string m_strURL;
public:
	CTextMonitor() : m_enTCSrc(TEXTCOPY_SRC_NONE) {};

	virtual void Paint(IMonitorPainter * pPainter)
	{
		pPainter->DrawTextMonitor(m_wstrLabel.c_str(), m_wstrValue.c_str());
	}
	CTextMonitor & operator = (const wchar_t * wcLabel)
	{
		m_wstrValue = wcLabel;
		return *this;
	}
	void SetTextCopySrc(enum TEXTCOPY_SOURCES enSrc)
	{
		m_enTCSrc = enSrc;
	}
	virtual void PrepareContextMenu(IListAcceptor * pMenu);
	virtual void ProcessMenuCommand(int i);
};

class COptionMonitor : public CMonitorBase
{
private:
	CRegScalar<bool, REG_BINARY> m_Value;
public:
	virtual void Paint(IMonitorPainter * pPainter);
	COptionMonitor & operator = (bool f)
	{
		m_Value.Set(f);
		return *this;
	}
	operator bool () {return m_Value();}
	bool operator()(){return m_Value();}
	void SetRegistry(HKEY hRegKey, wchar_t * wstrName, bool fDefault)
	{
		m_Value.Init(hRegKey, wstrName, fDefault);
		m_fSet = true;
	}
	virtual void PrepareContextMenu(IListAcceptor * pMenu)
	{
		pMenu->AddItem(L("Toggle option"), cnToggleCmd);
	}
	virtual void ProcessMenuCommand(int i)
	{
		if (i == cnToggleCmd)
			(*this) = !m_Value;
	}
};

class CAzimuthMonitor : public CMonitorBase
{
private:
	int m_iValue;
public:
	void Set(int iValue)
	{
		m_fSet = true;
		m_iValue = iValue;
	}
	int Get()
	{
		return m_iValue;
	}
	virtual void Paint(IMonitorPainter * pPainter);
};

class CDoubleMonitor : public CMonitorBase
{
protected:
	double m_dValue;
	CRegScalar<double, REG_BINARY> m_reg;
	bool m_fInRegistry;
	bool m_fResetable;
	int m_iDigits;
public:
	CDoubleMonitor() : m_fInRegistry(false), m_fResetable(false), m_iDigits(1) {} 
	virtual void Paint(IMonitorPainter * pPainter)
	{
		AutoLock l;
		pPainter->DrawTextMonitor(m_wstrLabel.c_str(), m_fSet ? DoubleToText(m_dValue, m_iDigits).c_str() : L"-");
	}
	CDoubleMonitor & operator = (double d)
	{
		Set(d);
		return *this;
	}
	void Set(double d)
	{
		AutoLock l;
		m_dValue = d;
		m_fSet = true;
		if (m_fInRegistry)
			m_reg.Set(m_dValue);
	}
	void SetRegistry(HKEY hRegKey, wchar_t * wstrName)
	{
		AutoLock l;
		m_reg.Init(hRegKey, wstrName, 0);
		m_fInRegistry = true;
		m_dValue = m_reg();
		m_fSet = true;
	}
	void operator += (double d)
	{
		AutoLock l;
		(*this) = m_dValue + d;
	}
	void SetResetable()
	{
		m_fResetable = true;
	}
	virtual void PrepareContextMenu(IListAcceptor * pMenu)
	{
		if (m_fResetable)
			pMenu->AddItem(L("Reset"), cnResetCmd);
	}
	virtual void ProcessMenuCommand(int i)
	{
		if (i == cnResetCmd)
			(*this) = 0.0;
	}
	double Get()
	{
		AutoLock l;
		return m_dValue;
	}
	void SetPrecision(int iDigits)
	{
		AutoLock l;
		m_iDigits = iDigits;
	}
};

class CDegreeMonitorLat : public CDoubleMonitor
{
protected:
	CDoubleMonitor* m_monLinkedLongitude;
public:
	void SetLinkedLongitude(CDoubleMonitor* mon) { m_monLinkedLongitude = mon; };
	void Paint(IMonitorPainter * pPainter)
	{
		std::wstring wstrLon, wstrLat;
		CoordToText(m_monLinkedLongitude->Get(), m_dValue, wstrLon, wstrLat);
		pPainter->DrawTextMonitor(m_wstrLabel.c_str(), m_fSet ? wstrLat.c_str() : L"-");
	}
};

class CDegreeMonitorLon : public CDoubleMonitor
{
protected:
	CDoubleMonitor* m_monLinkedLatitude;
public:
	void SetLinkedLatitude(CDoubleMonitor* mon) { m_monLinkedLatitude = mon; };
	void Paint(IMonitorPainter * pPainter)
	{
		std::wstring wstrLon, wstrLat;
		CoordToText(m_dValue, m_monLinkedLatitude->Get(), wstrLon, wstrLat);
		pPainter->DrawTextMonitor(m_wstrLabel.c_str(), m_fSet ? wstrLon.c_str() : L"-");
	}
};

class CSatellitesMonitor : public CMonitorBase
{
protected:
	CNMEAParser * m_pNMEAParser;
public:
	virtual void Paint(IMonitorPainter * pPainter);
	void SetParser(CNMEAParser * pParser)
	{
		m_pNMEAParser = pParser;
	}
};

class CDistanceMonitor : public CDoubleMonitor
{
public:
	virtual void Paint(IMonitorPainter * pPainter)
	{
		AutoLock l;
		pPainter->DrawTextMonitor(m_wstrLabel.c_str(), m_fSet ? DistanceToText(m_dValue).c_str() : L"-");
	}
};

class CHeightMonitor : public CDoubleMonitor
{
public:
	virtual void Paint(IMonitorPainter * pPainter)
	{
		AutoLock l;
		pPainter->DrawTextMonitor(m_wstrLabel.c_str(), m_fSet ? HeightToText(m_dValue).c_str() : L"-");
	}
};

class CSpeedMonitor : public CDoubleMonitor
{
	virtual void Paint(IMonitorPainter * pPainter)
	{
		AutoLock l;
		pPainter->DrawTextMonitor(m_wstrLabel.c_str(), m_fSet ? SpeedToText(m_dValue).c_str() : L"-");
	}
};

#endif // MONITORS_H
