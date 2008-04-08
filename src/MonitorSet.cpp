#include "MonitorSet.h"
#include "MapApp.h"

void CMonitorSet::ContextMenu(HWND hWnd, int iMonitor, ScreenPoint pt)
{
	map<int, wstring> mapMenu;
	CMenu mmPopupMenu;
	mmPopupMenu.Init();
	CMenu & mmMonitors = mmPopupMenu.CreateSubMenu(L("Change"));
	if (m_mapMonitors.find(m_vectMonitors[iMonitor]) != m_mapMonitors.end() && m_mapMonitors[m_vectMonitors[iMonitor]])
		m_mapMonitors[m_vectMonitors[iMonitor]]->PrepareContextMenu(mmPopupMenu.GetListAcceptor());
	int i = 1;
	mmMonitors.CreateItem(L("None"), 0xbadd);
	mmMonitors.CreateBreak();
	for (map<wstring, IMonitor *>::iterator it = m_mapMonitors.begin(); it != m_mapMonitors.end(); ++it)
	{
		mmMonitors.CreateItem(GetDict().Translate(it->first.c_str()), (it->first == m_vectMonitors[iMonitor]) ? -1 : i);
		mapMenu[i] = it->first;
		++i;
	}
	DWORD res = mmPopupMenu.Popup(pt.x, pt.y, hWnd);
	if (res == 0xbadd)
	{
		m_vectMonitors[iMonitor] = L"";
		Save();
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
		m_listMonitorRects.push_back(make_pair(srCurrentMonitor, i));
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
