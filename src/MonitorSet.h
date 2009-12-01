/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef MONITORSET_H
#define MONITORSET_H

#include "Menu.h"
#include "Monitors.h"

#ifdef LINUX
	typedef void* HWND;
#endif

class CMonitorSet
{
private:
	typedef std::list<std::pair<ScreenRect, int> > MonitorRects;
	MonitorRects m_listMonitorRects;
	std::map<std::tstring, IMonitor *> m_mapMonitors;
	std::vector<std::tstring> m_vectMonitors;
#ifndef LINUX
	HKEY m_hRegKey;
#endif
	UInt m_iActiveMonitor;
	int m_nRow;
	UInt m_iInRow;
public:
#ifndef LINUX
	void Init(HKEY hRegKey)
	{
		m_hRegKey = hRegKey;
		Load();
		m_iActiveMonitor = 0;
		m_nRow = 0;
	}
#endif
	void AddMonitor(IMonitor * pMonitor)
	{
		std::tstring wstrName = pMonitor->GetId();
		m_mapMonitors[wstrName] = pMonitor;
		if (m_vectMonitors.size() < 30)
		{
			std::vector<std::tstring>::iterator it;
			for (it = m_vectMonitors.begin(); it != m_vectMonitors.end(); ++it)
				if (*it == wstrName) break;
			if (it == m_vectMonitors.end())
				m_vectMonitors.push_back(wstrName);
		}
		Save();
	}
	void PaintMonitors(IMonitorPainter * pPainter, ScreenRect sr, bool fMonitorsMode, bool fVertical, bool fLarge);
	void ContextMenu(HWND hWnd, int iMonitor, ScreenRect rt)
	{
		ContextMenu(hWnd, iMonitor, ScreenPoint(
			rt.left + (rt.right - rt.left) * iMonitor / m_vectMonitors.size(), 
			rt.bottom));
	}
	void ContextMenu(HWND hWnd, int iMonitor, ScreenPoint pt);
	void ContextMenu(HWND hWnd, ScreenPoint pt)
	{
		for (MonitorRects::iterator it1 = m_listMonitorRects.begin(); 
			it1 != m_listMonitorRects.end(); ++it1)
		{
			if (it1->first.Side(pt) == 0)
			{
				int iMonitor = it1->second;
				ContextMenu(hWnd, iMonitor, pt);
				return;
			}
		}
	}
	void Load()
	{
#ifndef LINUX
		std::vector<Byte> data;
		DWORD ulTotalLen = 0;
		DWORD dwType = REG_BINARY;
		RegQueryValueEx(m_hRegKey, T("Monitors"), 0, &dwType, 0, &ulTotalLen);
		if (ulTotalLen > 0)
		{
			if (dwType != REG_BINARY)
				return;
			data.resize(ulTotalLen);
			if (RegQueryValueEx(m_hRegKey, T("Monitors"), 0, &dwType, &data[0], &ulTotalLen) != ERROR_SUCCESS)
				return;
			m_vectMonitors.clear();
			unsigned int uiPos = 0;
			while (uiPos < ulTotalLen)
			{
				int iLen;
				std::tstring wstr;
				memcpy(&iLen, &data[uiPos], sizeof(iLen));
				uiPos += sizeof(iLen);
				wstr.assign((tchar_t *)&data[uiPos], iLen);
				uiPos += sizeof(tchar_t) * iLen;
				m_vectMonitors.push_back(wstr);
			}
		}
#endif
	}
	void Save()
	{
#ifndef LINUX
		std::vector<Byte> data;
		for (std::vector<std::tstring>::iterator it = m_vectMonitors.begin(); it != m_vectMonitors.end(); ++it)
		{
			std::tstring wstrFilename = *it;
			int len = wstrFilename.length();
			data.insert(data.end(), (const Byte*)&len, (const Byte*)&len + sizeof(len));
			data.insert(data.end(), (const Byte*)&wstrFilename.c_str()[0], (const Byte*)&wstrFilename.c_str()[0] + sizeof(tchar_t) * len); 
		}
		tchar_t buf[1000];
		stprintf(buf, 1000, T("%d"), data.size());
		RegSetValueEx(m_hRegKey, T("Monitors"), 0, REG_BINARY, &data[0], data.size());
#endif
	}
	void Decrease(UInt uiDiff)
	{
		while (m_iActiveMonitor < uiDiff)
			m_iActiveMonitor += m_vectMonitors.size();
		m_iActiveMonitor -= uiDiff;
	}
	void Increase(UInt uiDiff)
	{
		m_iActiveMonitor += uiDiff;
		if (m_iActiveMonitor >= m_vectMonitors.size())
			m_iActiveMonitor %= m_vectMonitors.size();
	}
	void Left()
	{
		Decrease(1);
	}
	void Right()
	{
		Increase(1);
	}
	void Up()
	{
		Decrease(m_iInRow);
	}
	void Down()
	{
		Increase(m_iInRow);
	}
	int GetActiveMonitor()
	{
		return m_iActiveMonitor;
	}
	void NextRow()
	{
		++ m_nRow;
		if (m_nRow * m_iInRow > m_vectMonitors.size() - 1)
			m_nRow = 0;
	}
	void PrevRow()
	{
		-- m_nRow;
		if (m_nRow < 0)
			m_nRow = (m_vectMonitors.size() - 1) / m_iInRow;
	}
};

#endif // MONITORSET_H

