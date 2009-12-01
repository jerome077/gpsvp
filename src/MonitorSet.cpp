/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "MonitorSet.h"
#include "MapApp.h"

void CMonitorSet::ContextMenu(HWND hWnd, int iMonitor, ScreenPoint pt)
{
	std::map<int, std::tstring> mapMenu;
	CMenu mmPopupMenu;
	mmPopupMenu.Init();

	// Monitor specific submenu
	CMenu & mmMonitors = mmPopupMenu.CreateSubMenu(L("Change"));
	if (m_mapMonitors.find(m_vectMonitors[iMonitor]) != m_mapMonitors.end() && m_mapMonitors[m_vectMonitors[iMonitor]])
		m_mapMonitors[m_vectMonitors[iMonitor]]->PrepareContextMenu(mmPopupMenu.GetListAcceptor());
	int i = 1;
	mmMonitors.CreateItem(L("None"), 0xbadd);
	mmMonitors.CreateBreak();
	for (std::map<std::tstring, IMonitor *>::iterator it = m_mapMonitors.begin(); it != m_mapMonitors.end(); ++it)
	{
		mmMonitors.CreateItem(GetDict().Translate(it->first.c_str()), (it->first == m_vectMonitors[iMonitor]) ? -1 : i);
		mapMenu[i] = it->first;
		++i;
	}

	// Other non monitor menu items
	mmPopupMenu.CreateBreak();
	mmPopupMenu.CreateItem(L("Full screen"), mcoFullScreen);
	mmPopupMenu.CheckMenuItem(mcoFullScreen, app.m_Options[mcoFullScreen]);
	mmPopupMenu.CreateItem(L("Monitors Mode"), mcoMonitorsMode);
	mmPopupMenu.CheckMenuItem(mcoMonitorsMode, app.m_Options[mcoMonitorsMode]);
	// I think I should be using MF_CHECKED | MF_BYCOMMAND. but this seems to work 

	// Menu processing
	DWORD res = mmPopupMenu.Popup(pt.x, pt.y, hWnd);
	if (res == 0xbadd)
	{
		m_vectMonitors[iMonitor] = T("");
		Save();
	}
	else if (res == mcoFullScreen)
	{
		app.ProcessCommand(mcoFullScreen);
	}
	else if (res == mcoMonitorsMode)
	{
		app.ProcessCommand(mcoMonitorsMode);
	}
	else if (mapMenu.find(res) != mapMenu.end())
	{
		m_vectMonitors[iMonitor] = mapMenu[res];
		Save();
	}
	else
	{
		if (m_mapMonitors.find(m_vectMonitors[iMonitor]) != m_mapMonitors.end() && m_mapMonitors[m_vectMonitors[iMonitor]])
		{
			m_mapMonitors[m_vectMonitors[iMonitor]]->ProcessMenuCommand(res);
			app.CheckOptions();
			app.m_painter.Redraw();
		}
	}
}

void CMonitorSet::PaintMonitors(IMonitorPainter * pPainter, ScreenRect sr, bool fMonitorsMode, bool fVertical, bool fLarge)
{
	int iMonitorRows = fLarge ? 6 : 10;
	m_listMonitorRects.clear();
	if (m_vectMonitors.size() < 1)
		return;
	if (fLarge)
		m_iInRow = ((fVertical || fMonitorsMode) ? (fVertical ? 2 : 3) : 1);
	else
		m_iInRow = ((fVertical || fMonitorsMode) ? (fVertical ? 3 : 4) : 1);
	unsigned int i = 0;
	if (!fMonitorsMode)
		i = m_nRow * (fLarge ? 2 : 3);
	int iRowsLeft = (fMonitorsMode || !fVertical) ? 
		(fVertical ? iMonitorRows : iMonitorRows * 3 / 4) : 1;
	ScreenRect ssr;
	ssr.Init(ScreenPoint(sr.left, sr.top));
	ssr.Append(ScreenPoint(sr.right, sr.top + sr.Height() / iRowsLeft));
	int colleft = m_iInRow;
	for (;;)
	{
		if (fMonitorsMode)
		{
			if (i >= m_vectMonitors.size())
				break;
		}
		else
			i %= m_vectMonitors.size();
		ScreenRect srCurrentMonitor;
		srCurrentMonitor.Init(ScreenPoint(ssr.left, ssr.top));
		srCurrentMonitor.Append(ScreenPoint(srCurrentMonitor.right + ssr.Width() / colleft, ssr.bottom));
		ssr.left = srCurrentMonitor.right;
		pPainter->SetCurrentMonitor(srCurrentMonitor, i == m_iActiveMonitor && fMonitorsMode);
		m_listMonitorRects.push_back(std::make_pair(srCurrentMonitor, i));
		if (m_mapMonitors.find(m_vectMonitors[i]) != m_mapMonitors.end() && m_mapMonitors[m_vectMonitors[i]])
			m_mapMonitors[m_vectMonitors[i]]->Paint(pPainter);
		if (!--colleft)
		{
			sr.top = ssr.bottom;
			if (sr.top >= sr.bottom)
				break;
			-- iRowsLeft;
			ssr.Init(ScreenPoint(sr.left, sr.top));
			ssr.Append(ScreenPoint(sr.right, sr.top + sr.Height() / iRowsLeft));
			colleft = m_iInRow;
		}
		++i;
	}
}

void CMonitorSet::ContextMenu(HWND hWnd, int iMonitor, const ScreenRect& rt)
{
	ContextMenu(hWnd, iMonitor, ScreenPoint(
		rt.left + (rt.right - rt.left) * iMonitor / m_vectMonitors.size(), 
		rt.bottom));
}

void CMonitorSet::ContextMenu(HWND hWnd, const ScreenPoint& pt)
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
