/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef MRUPOINTS_H
#define MRUPOINTS_H

#include <list>
#include <windows.h>
#include "GeoPoint.h"
#include "GDIPainter.h"

class CMRUPoints
{
private:
	struct Point
	{
		GeoPoint gp;
		std::wstring wstrName;
	};
	typedef std::list<Point> Points;
	Points m_points;
	HKEY m_hRegKey;
	enum {cnMaxPoints = 9};
public:
	CMRUPoints() {};
	void AddPoint(GeoPoint gp, const wchar_t * wcName);
	void Navigate(ScreenPoint pt, HWND hWnd);
	void Init(HKEY hRegKey)
	{
		m_hRegKey = hRegKey;
		Load();
	}
	void Save()
	{
		std::vector<Byte> data;
		for (Points::iterator it = m_points.begin(); it != m_points.end(); ++it)
		{
			int len = it->wstrName.length();
			data.insert(data.end(), (const Byte*)&it->gp, (const Byte*)&it->gp + sizeof(it->gp));
			data.insert(data.end(), (const Byte*)&len, (const Byte*)&len + sizeof(len));
			data.insert(data.end(), (const Byte*)&it->wstrName.c_str()[0], (const Byte*)&it->wstrName.c_str()[0] + sizeof(wchar_t) * len); 
		}
		wchar_t buf[1000];
		wsprintf(buf, L"%d", data.size());
		RegSetValueEx(m_hRegKey, L"MRUPoints", 0, REG_BINARY, &data[0], data.size());
	}
	void Load()
	{
		std::vector<Byte> data;
		DWORD ulTotalLen = 0;
		DWORD dwType = REG_BINARY;
		RegQueryValueEx(m_hRegKey, L"MRUPoints", 0, &dwType, 0, &ulTotalLen);
		if (ulTotalLen > 0)
		{
			if (dwType != REG_BINARY)
				return;
			data.resize(ulTotalLen);
			if (RegQueryValueEx(m_hRegKey, L"MRUPoints", 0, &dwType, &data[0], &ulTotalLen) != ERROR_SUCCESS)
				return;
			m_points.clear();
			unsigned int uiPos = 0;
			while (uiPos < ulTotalLen)
			{
				GeoPoint gp;
				int iLen;
				std::wstring wstr;
				memcpy(&gp, &data[uiPos], sizeof(gp));
				uiPos += sizeof(gp);
				memcpy(&iLen, &data[uiPos], sizeof(iLen));
				uiPos += sizeof(iLen);
				wstr.assign((wchar_t *)&data[uiPos], iLen);
				uiPos += sizeof(wchar_t) * iLen;
				m_points.push_back(Point());
				m_points.back().gp = gp;
				m_points.back().wstrName = wstr;
			}
		}
	}
};

#endif // MRUPOINTS_H
