/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "MRUPoints.h"
#include "MapApp.h"

void CMRUPoints::AddPoint(GeoPoint gp, const tchar_t * wcName)
{
	std::tstring wstrName = wcName;

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
	mMenu.CreateItem(I("Full list ..."), iCount);
	unsigned int res = mMenu.Popup(pt.x, pt.y, hWnd);
	if (res == iCount)
	{
		app->ToolsWaypoints();
		return;
	}
	for (it = m_points.begin(), iCount = 1; it != m_points.end(); ++it, ++iCount)
	{
		if (iCount == res)
		{
			app->Navigate(it->gp, it->wstrName.c_str());
			break;
		}
	}
}
