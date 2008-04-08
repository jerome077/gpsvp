/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "WayPoints.h"
#include <windows.h>
#include <vector>

CWaypoints::CPoint::CPoint(double dLon, double dLat, int iAltitude, const wchar_t * wcName)
{
	static int iNextId = 0;
	m_iId = iNextId++;
	m_dLongitude = dLon;
	m_dLatitude = dLat;
	Cache();
	m_iAltitude = iAltitude;
	SetLabel(wcName);
	m_iRadius = 0;
	SYSTEMTIME st;
	FILETIME ft;
	GetSystemTime(&st);
	SystemTimeToFileTime(&st, &ft);
	m_dLastUsed = double(ft.dwHighDateTime) * double(1 << 16) * double(1 << 16) 
		+ double(ft.dwLowDateTime);
	m_fInProximity = false;
}
void CWaypoints::CPoint::Paint(IPainter * pPainter, const GeoPoint * pgp)
{
	int iType = 0xffff;
	if (pgp && m_iRadius && (IntDistance(*pgp, m_poscache) < m_iRadius))
		iType = 0xfffe;
	pPainter->PaintPoint(iType, m_poscache, m_wstrName.c_str());
}
void CWaypoints::CPoint::Write(FILE * pFile, int nNum)
{
	fprintf(pFile, "%d,%S,%2.8f,%2.8f, ,0,1,3,0,65535,,0,0,%d,%d,,,,,,,,,,%f\n", nNum, m_wstrName.c_str(), m_dLatitude, m_dLongitude, m_iRadius, m_iAltitude, m_dLastUsed);
}
void CWaypoints::CPoint::AddToList(IListAcceptor * pAcceptor)
{
	pAcceptor->AddItem(m_wstrName.c_str(), m_iId);
}
void CWaypoints::CPoint::AddToList(IListAcceptor2 * pAcceptor)
{
	wchar_t buff[100];
	int i = pAcceptor->AddItem(m_wstrName.c_str(), m_iId, 0, 0);
	swprintf(buff, 100, L"%f", m_dLatitude);
	pAcceptor->AddItem(buff, i, 2, 0);
	swprintf(buff, 100, L"%f", m_dLongitude);
	pAcceptor->AddItem(buff, i, 3, 0);
}

bool CWaypoints::CPoint::CheckProximity(GeoPoint gp)
{
	if (!m_iRadius)
		return false;
	if (!m_fInProximity && (IntDistance(m_poscache, gp) < m_iRadius))
	{
		m_fInProximity = true;
		return true;
	}
	if (m_fInProximity && (IntDistance(m_poscache, gp) > m_iRadius * 2))
		m_fInProximity = false;
	return false;
}
bool CWaypoints::CPoint::operator == (const CPoint & to) const
{
	return m_dLatitude == to.m_dLatitude && m_dLongitude == to.m_dLongitude;
}
bool CWaypoints::CPoint::operator == (int to) const
{
	return m_iId == to;
}

Int CWaypoints::AddPoint(GeoPoint gp, int iAltitude, const wchar_t * wcName, int iRadius)
{
	m_Points.push_back(CPoint(Degree(gp.lon), Degree(gp.lat), iAltitude, wcName));
	m_Points.back().SetRadius(iRadius);
	Write();
	return m_Points.back().GetID();
}
Int CWaypoints::AddPoint(const CPoint & pt)
{
	m_Points.push_back(pt);
	Write();
	return m_Points.back().GetID();
}
int CWaypoints::GetNearestPoint(GeoPoint gp, double dRadius)
{
	double dMin = dRadius;
	int nNearest = -1;
	for (list<CPoint>::iterator it = m_Points.begin(); it != m_Points.end(); ++it)
	{
		double dDistance = IntDistance(it->GetApproximatePos(), gp);
		if (dDistance < dMin)
		{
			dMin = dDistance;
			nNearest = it->GetID();
		}
	}
	return nNearest;
}
void CWaypoints::Write()
{
	if (m_wstrFilename.empty())
		return;
	FILE * pFile = wfopen(m_wstrFilename.c_str(), L"wt");
	if (!pFile)
		return;
	fputs(
		"OziExplorer Waypoint File Version 1.0\n"
		"WGS 84\n"
		"Reserved 2\n"
		"Reserved 3\n", pFile);
	int nCount = 0;
	for (list<CPoint>::iterator it = m_Points.begin(); it != m_Points.end(); ++it)
	{
		it->Write(pFile, ++nCount);
	}
	fclose(pFile);
}
void CWaypoints::Read(const wchar_t * wcFilename)
{
	m_wstrFilename = L"";
	m_Points.erase(m_Points.begin(), m_Points.end());
	char buff[1000];
	FILE * pFile = wfopen(wcFilename, L"rt");
	if (pFile)
	{
		std::vector<long> vRecord;
		for (int i = 0; i < 4; ++i)
		{
			if (!fgets(buff, sizeof(buff), pFile))
				break;
		}
		vector<string> listParts;
		while(fgets(buff, sizeof(buff), pFile))
		{
			string strCommand = buff;
			listParts.resize(0);
			string::size_type pos = 0;
			string::size_type nextpos = 0;
			while ((nextpos = strCommand.find(',', pos)) != string::npos)
			{
				listParts.push_back(strCommand.substr(pos, nextpos - pos));
				pos = nextpos + 1;
			}
			listParts.push_back(strCommand.substr(pos));
			if (listParts.size() >= 15)
			{
				double dLatitude = myatof(listParts[2].c_str());
				double dLongitude = myatof(listParts[3].c_str());
				wchar_t buff[1000] = {0};
				MultiByteToWideChar(CP_ACP, 0, listParts[1].c_str(), -1, buff, 1000);
				int iRadius = atoi(listParts[13].c_str());
				int iAltitude = atoi(listParts[14].c_str());
				AddPoint(GeoPoint(dLongitude, dLatitude), iAltitude, buff, iRadius);
			}
		}
	}
	m_wstrFilename = wcFilename;
}
void CWaypoints::Paint(IPainter * pPainter, const GeoPoint * pgp)
{
	for (list<CPoint>::iterator it = m_Points.begin(); it != m_Points.end(); ++it)
		it->Paint(pPainter, pgp);
}
void CWaypoints::GetList(IListAcceptor * pAcceptor, GeoPoint gpCenter, int iRadius)
{
	for (list<CPoint>::iterator it = m_Points.begin(); it != m_Points.end(); ++it)
	{
		if (IntDistance(it->GetApproximatePos(), gpCenter) < iRadius)
			it->AddToList(pAcceptor);
	}
}
void CWaypoints::GetList(IListAcceptor2 * pAcceptor)
{
	for (list<CPoint>::iterator it = m_Points.begin(); it != m_Points.end(); ++it)
		it->AddToList(pAcceptor);
}
wstring CWaypoints::GetLabelByID(int iId)
{
	for (list<CPoint>::iterator it = m_Points.begin(); it != m_Points.end(); ++it)
	{
		if (it->GetID() == iId)
			return it->GetLabel();
	}
	return L"";
}
int CWaypoints::GetRadiusByID(int iId)
{
	for (list<CPoint>::iterator it = m_Points.begin(); it != m_Points.end(); ++it)
	{
		if (it->GetID() == iId)
			return it->GetRadius();
	}
	return 0;
}
int CWaypoints::GetAltitudeByID(int iId)
{
	for (list<CPoint>::iterator it = m_Points.begin(); it != m_Points.end(); ++it)
	{
		if (it->GetID() == iId)
			return it->GetAltitude();
	}
	return 0;
}
void CWaypoints::SetLabelByID(int iId, const wchar_t * wcLabel)
{
	for (list<CPoint>::iterator it = m_Points.begin(); it != m_Points.end(); ++it)
	{
		if (it->GetID() == iId)
		{
			it->SetLabel(wcLabel);
			Write();
			return;
		}
	}
}
void CWaypoints::SetRadiusByID(int iId, int iRadius)
{
	for (list<CPoint>::iterator it = m_Points.begin(); it != m_Points.end(); ++it)
	{
		if (it->GetID() == iId)
		{
			it->SetRadius(iRadius);
			Write();
			return;
		}
	}
}
GeoPoint CWaypoints::GetPointByIDApprox(int iId)
{
	for (list<CPoint>::iterator it = m_Points.begin(); it != m_Points.end(); ++it)
	{
		if (it->GetID() == iId)
			return it->GetApproximatePos();
	}
	return GeoPoint(0,0);
}
double CWaypoints::GetUsedByID(int iId)
{
	for (list<CPoint>::iterator it = m_Points.begin(); it != m_Points.end(); ++it)
	{
		if (it->GetID() == iId)
			return it->GetUsed();
	}
	return 0;
}
void CWaypoints::DeleteByID(int iId)
{
	for (list<CPoint>::iterator it = m_Points.begin(); it != m_Points.end(); ++it)
	{
		if (it->GetID() == iId)
		{
			m_Points.erase(it);
			Write();
			return;
		}
	}
}	
void CWaypoints::MovePoint(int iId, GeoPoint gp)
{
	for (list<CPoint>::iterator it = m_Points.begin(); it != m_Points.end(); ++it)
	{
		if (it->GetID() == iId)
		{
			it->SetApproximatePos(gp);
			Write();
			return;
		}
	}
}	
bool CWaypoints::CheckProximity(GeoPoint gp)
{
	bool fRes = false;
	for (list<CPoint>::iterator it = m_Points.begin(); it != m_Points.end(); ++it)
		fRes |= it->CheckProximity(gp);
	return fRes;
}	
void CWaypoints::Import(const CWaypoints & from)
{
	for (list<CPoint>::const_iterator it = from.m_Points.begin(); it != from.m_Points.end(); ++it)
	{
		if (std::find(m_Points.begin(), m_Points.end(), *it) == m_Points.end())
		{
			m_Points.push_back(*it);
		}
	}
	Write();
}
CWaypoints::CPoint & CWaypoints::ById(int id)
{
	list<CPoint>::iterator it = std::find(m_Points.begin(), m_Points.end(), id);
	if (it != m_Points.end())
		return *it;
	static CPoint stub(0, 0, 0, L"");
	return stub;
}
