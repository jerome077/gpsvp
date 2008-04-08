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
		wstring wstrName;
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
		vector<Byte> data;
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
		vector<Byte> data;
		unsigned long ulTotalLen = 0;
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
				wstring wstr;
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