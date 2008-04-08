#include "MRUPoints.h"
#include "MapApp.h"

void CMRUPoints::AddPoint(GeoPoint gp, const wchar_t * wcName)
{
	wstring wstrName = wcName;

	Points::iterator it;
	for (it = m_points.begin(); it != m_points.end(); ++it)
	{
		if (it->gp == gp)
			break;
	}
	if (it != m_points.end())
		m_points.erase(it);
	else
	{
		if (m_points.size() >= cnMaxPoints)
		{
			m_points.pop_back();
		}
	}

	m_points.push_front(Point());
	m_points.front().gp = gp;
	m_points.front().wstrName = wstrName;
	Save();
}

void CMRUPoints::Navigate(ScreenPoint pt, HWND hWnd)
{
	CMenu mMenu;
	mMenu.Init();
	Points::iterator it;
	int iCount;
	for (it = m_points.begin(), iCount = 1; it != m_points.end() && iCount <= cnMaxPoints; ++it, ++iCount)
		mMenu.CreateItem(it->wstrName.c_str(), iCount);
	mMenu.CreateItem(L("Full list ..."), iCount);
	DWORD res = mMenu.Popup(pt.x, pt.y, hWnd);
	if (res == iCount)
	{
		app.ToolsWaypoints();
		return;
	}
	for (it = m_points.begin(), iCount = 1; it != m_points.end(); ++it, ++iCount)
	{
		if (iCount == res)
		{
			app.Navigate(it->gp, it->wstrName.c_str());
			break;
		}
	}
}