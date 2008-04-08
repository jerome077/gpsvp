#ifndef MONITORSET_H
#define MONITORSET_H

#include "Menu.h"
#include "GDIPainter.h"

class CMonitorSet
{
private:
	typedef list< pair<ScreenRect, int> > MonitorRects;
	MonitorRects m_listMonitorRects;
	map<wstring, IMonitor *> m_mapMonitors;
	vector<wstring> m_vectMonitors;
	HKEY m_hRegKey;
	UInt m_iActiveMonitor;
	int m_nRow;
	UInt m_iInRow;
public:
	void Init(HKEY hRegKey)
	{
		m_hRegKey = hRegKey;
		Load();
		m_iActiveMonitor = 0;
		m_nRow = 0;
	}
	void AddMonitor(IMonitor * pMonitor)
	{
		wstring wstrName = pMonitor->GetId();
		m_mapMonitors[wstrName] = pMonitor;
		if (m_vectMonitors.size() < 30)
		{
			vector<wstring>::iterator it;
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
		vector<Byte> data;
		unsigned long ulTotalLen = 0;
		DWORD dwType = REG_BINARY;
		RegQueryValueEx(m_hRegKey, L"Monitors", 0, &dwType, 0, &ulTotalLen);
		if (ulTotalLen > 0)
		{
			if (dwType != REG_BINARY)
				return;
			data.resize(ulTotalLen);
			if (RegQueryValueEx(m_hRegKey, L"Monitors", 0, &dwType, &data[0], &ulTotalLen) != ERROR_SUCCESS)
				return;
			m_vectMonitors.clear();
			unsigned int uiPos = 0;
			while (uiPos < ulTotalLen)
			{
				int iLen;
				wstring wstr;
				memcpy(&iLen, &data[uiPos], sizeof(iLen));
				uiPos += sizeof(iLen);
				wstr.assign((wchar_t *)&data[uiPos], iLen);
				uiPos += sizeof(wchar_t) * iLen;
				m_vectMonitors.push_back(wstr);
			}
		}
	}
	void Save()
	{
		vector<Byte> data;
		for (vector<wstring>::iterator it = m_vectMonitors.begin(); it != m_vectMonitors.end(); ++it)
		{
			wstring wstrFilename = *it;
			int len = wstrFilename.length();
			data.insert(data.end(), (const Byte*)&len, (const Byte*)&len + sizeof(len));
			data.insert(data.end(), (const Byte*)&wstrFilename.c_str()[0], (const Byte*)&wstrFilename.c_str()[0] + sizeof(wchar_t) * len); 
		}
		wchar_t buf[1000];
		wsprintf(buf, L"%d", data.size());
		RegSetValueEx(m_hRegKey, L"Monitors", 0, REG_BINARY, &data[0], data.size());
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

