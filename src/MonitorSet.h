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
#include "GDIPainter.h"

class CMonitorSet
{
private:
	typedef std::list<std::pair<ScreenRect, int> > MonitorRects;
	MonitorRects m_listMonitorRects;
	std::map<std::wstring, IMonitor *> m_mapMonitors;
	std::vector<std::wstring> m_vectMonitors;
	HKEY m_hRegKey;
	UInt m_iActiveMonitor;
	int m_nRow;
	UInt m_iInRow;
public:
	void Init(HKEY hRegKey)
	{
		m_hRegKey = hRegKey;
		m_nRow = 0;
		Load();
		m_iActiveMonitor = 0;
	}
	void AddMonitor(IMonitor * pMonitor);
	void PaintMonitors(IMonitorPainter * pPainter, ScreenRect sr, bool fMonitorsMode, bool fVertical, bool fLarge);
	void ContextMenu(HWND hWnd, int iMonitor, ScreenRect rt);
	void ContextMenu(HWND hWnd, int iMonitor, ScreenPoint pt);
	void ContextMenu(HWND hWnd, ScreenPoint pt);
	int GetMonitorUnder(ScreenPoint pt);
	void SwapMonitors(int m1, int m2);
	void Load();
	void Save();
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
	void SetActiveMonitor(UInt uiMonitor)
	{
		m_iActiveMonitor = uiMonitor;
	}
	void NextRow()
	{
		++ m_nRow;
		if (m_nRow * m_iInRow > m_vectMonitors.size() - 1)
			m_nRow = 0;
		Save();
	}
	void PrevRow()
	{
		-- m_nRow;
		if (m_nRow < 0)
			m_nRow = (m_vectMonitors.size() - 1) / m_iInRow;
		Save();
	}
};

#endif // MONITORSET_H

