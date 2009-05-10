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
#include "FileFormats\OSM.h"
#include "MapApp.h"

bool isWptModel(const std::wstring& WaypointName)
{
	return (WaypointName.substr(0, WPT_MODEL_PREFIX.length()) == WPT_MODEL_PREFIX);
}


CWaypoints::CPoint::CPoint(double dLon, double dLat, int iAltitude, const wchar_t * wcName)
: m_OSMPropList()
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
//CWaypoints::CPoint::CPoint(const CWaypoints::CPoint& source)
//{
//	m_iId = source.m_iId; // Same ID because not in the list
//	m_dLongitude = source.m_dLongitude;
//	m_dLatitude = source.m_dLatitude;
//	m_poscache = source.m_poscache;
//	m_iAltitude = source.m_iAltitude;
//	m_wstrName = source.m_wstrName;
//	m_iRadius = source.m_iRadius;
//	m_dLastUsed = source.m_dLastUsed;
//	m_fInProximity = source.m_fInProximity;
//}
CWaypoints::CPoint::CPoint()
{
	m_iId = 0;
	m_dLongitude = 0;
	m_dLatitude = 0;
	m_iAltitude = -777;
	m_iRadius = 0;
	m_fInProximity = false;
}
void CWaypoints::CPoint::Assign(const CWaypoints::CPoint& source)
{
	// m_iId don't need to be changed
	m_dLongitude = source.m_dLongitude;
	m_dLatitude = source.m_dLatitude;
	m_poscache = source.m_poscache;
	m_iAltitude = source.m_iAltitude;
	m_wstrName = source.m_wstrName;
	m_iRadius = source.m_iRadius;
	m_dLastUsed = source.m_dLastUsed;
	m_fInProximity = source.m_fInProximity;
	m_OSMPropList.assign(source.m_OSMPropList.begin(), source.m_OSMPropList.end());
}
void CWaypoints::CPoint::AssignOSM(const CWaypoints::CPoint& source)
{
	// here just copy the complementary porperties (to use the point as model)
	m_iRadius = source.m_iRadius;
	m_fInProximity = source.m_fInProximity;
	m_OSMPropList.assign(source.m_OSMPropList.begin(), source.m_OSMPropList.end());
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
Int CWaypoints::AddPoint(const CPoint & pt, int iRadius)
{
	m_Points.push_back(pt);
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

// ---------------------------------------------------------------

void CWaypoints::Write()
{
	if (0 < m_BeginUpdateCount)
	{
		m_bWriteRequested = true;
		return;
	}
	if (m_bCanWrite)
		Write(m_wstrFilename);
};

void CWaypoints::Write(const std::wstring& wstrFilename)
{
	if (wstrFilename.empty())
		return;
	std::wstring wstrExt = wstrFilename.substr(wstrFilename.length()-4, 4);
	if (0 == _wcsnicmp(wstrExt.c_str(), L".gpx", 4))
		WriteGPX(wstrFilename);
	else
		WriteWPT(wstrFilename);
}

// ---------------------------------------------------------------

void CWaypoints::WriteGPX(const std::wstring& wstrFilename)
{
	try
	{
		CGPXFileWriter GpxWriter(wstrFilename, GetCreator());
		for (list<CPoint>::iterator it = m_Points.begin(); it != m_Points.end(); ++it)
		{
			GpxWriter.AddNextWaypoint(it->m_dLatitude, it->m_dLongitude);
			// Caution: field order is important, see http://www.topografix.com/GPX/1/1/
			GpxWriter.CurrentWpt()->addAltitude(it->m_iAltitude);
			GpxWriter.CurrentWpt()->addName(it->m_wstrName);

			// <extensions> after all standard fields:
			GpxWriter.CurrentWpt()->addRadiusEx(it->m_iRadius);
			for(std::vector<CStringProp>::iterator iter = it->m_OSMPropList.begin();
				iter != it->m_OSMPropList.end();
				iter++)
			{
				GpxWriter.CurrentWpt()->addOSMTag(iter->Name(), iter->Value());
			}
		}
	}
	catch (CGPXFileWriter::Error e)
	{
		MessageBox(NULL, e.c_str(), L("GPX write error"), MB_ICONEXCLAMATION);
	}
}

// ---------------------------------------------------------------

void CWaypoints::WriteWPT(const std::wstring& wstrFilename)
{
	FILE * pFile = wfopen(wstrFilename.c_str(), L"wt");
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

// ---------------------------------------------------------------

// Default Name for waypoint is like "2001.01.01 12:00:00"
bool IsDefaultWaypointName(const std::wstring& wstrWptName)
{
	return (19 == wstrWptName.length())
	    && (L'.' == wstrWptName[4])
	    && (L'.' == wstrWptName[7])
	    && (L' ' == wstrWptName[10])
	    && (L':' == wstrWptName[13])
	    && (L':' == wstrWptName[16])
	    && (isdigit(wstrWptName[0]))
	    && (isdigit(wstrWptName[1]))
	    && (isdigit(wstrWptName[2]))
	    && (isdigit(wstrWptName[3]))
	    && (isdigit(wstrWptName[5]))
	    && (isdigit(wstrWptName[6]))
	    && (isdigit(wstrWptName[8]))
	    && (isdigit(wstrWptName[9]))
	    && (isdigit(wstrWptName[11]))
	    && (isdigit(wstrWptName[12]))
	    && (isdigit(wstrWptName[14]))
	    && (isdigit(wstrWptName[15]))
	    && (isdigit(wstrWptName[17]))
	    && (isdigit(wstrWptName[18]));
}

// ---------------------------------------------------------------

void CWaypoints::WriteOSM(const std::wstring& wstrFilename)
{
	try
	{
		COSMFileWriter OsmWriter(wstrFilename, GetCreator());
		int iOsmWptCount = 0;
		for (list<CPoint>::iterator it = m_Points.begin(); it != m_Points.end(); ++it)
		{
			if (isWptModel(it->m_wstrName)) continue;    // ignore models
			if (0 == it->m_OSMPropList.size()) continue; // ignore waypoints without osm tags
			bool bNameWritten = false;
			OsmWriter.AddNextWaypoint(it->m_dLatitude, it->m_dLongitude);
			iOsmWptCount++;
			for(std::vector<CStringProp>::iterator iter = it->m_OSMPropList.begin();
				iter != it->m_OSMPropList.end();
				iter++)
			{
				if (L"name" == iter->Name()) bNameWritten = true;
				if (0 == iter->Value().length()) continue; // ignore empty values
				OsmWriter.CurrentWpt()->addTag(iter->Name(), iter->Value());
			}
			if (!bNameWritten)
			{
				if (!IsDefaultWaypointName(it->m_wstrName))
					OsmWriter.CurrentWpt()->addTag(L"name", it->m_wstrName);
			}
		}
		wchar_t wcMsg[100];
		swprintf(wcMsg, 100, L("%d waypoints with OSM tags"), iOsmWptCount);
		MessageBox(NULL, wcMsg, L("File written"), MB_ICONEXCLAMATION);
	}
	catch (CGPXFileWriter::Error e)
	{
		MessageBox(NULL, e.c_str(), L("OSM write error"), MB_ICONEXCLAMATION);
	}
}

// ---------------------------------------------------------------

void CWaypoints::Read(const wchar_t * wcFilename)
{
	m_wstrFilename = L"";
	m_Points.erase(m_Points.begin(), m_Points.end());

	std::wstring wstrName = wcFilename;
	std::wstring wstrExt = wstrName.substr(wstrName.length()-4, 4);
	if (0 == _wcsnicmp(wstrExt.c_str(), L".gpx", 4))
		ReadGPX(wstrName);
	else
		ReadWPT(wcFilename);

	m_wstrFilename = wcFilename;
}

// ---------------------------------------------------------------

void CWaypoints::ReadGPX(const std::wstring& wstrFilename)
{
	try
	{
		ComInit MyObjectToInitCOM;
		{
			CGPXFileReader GpxReader(wstrFilename);
			CGPXFileReader::WaypointIterator iterWpt = GpxReader.WaypointBegin();
			while (iterWpt != GpxReader.WaypointEnd())
			{
				double dLatitude = iterWpt->getLatitude();
				double dLongitude = iterWpt->getLongitude();
				int iRadius = iterWpt->getRadius();
				int iAltitude = int(iterWpt->getAltitude());

				CWaypoints::CPoint& wpt = ById(AddPoint(CPoint(dLongitude, dLatitude, iAltitude, L""), iRadius));
				CWaypoints::CPointEditor wptEditor = wpt.GetEditor();
				std::auto_ptr<CGPXField> apField = iterWpt->firstField();
				while (!apField->eof())
				{
					std::wstring fieldName = apField->getName();
					if (L"name" == fieldName)
					{
						wpt.SetLabel(apField->getValue().c_str());
					}
					else if (L"gpsVP:osm" == fieldName)
					{
						CWaypoints::CPointProp& prop = wptEditor.AddOSMProp();
						prop.SetName(apField->getOSMKey());
						prop.SetValue(apField->getOSMValue());
					}
					apField = iterWpt->nextField();
				}
				wptEditor.Commit();

				++iterWpt;
			}
			m_bCanWrite = GpxReader.IsGpsVPWaypointFile();
		}
	}
	catch (CGPXFileReader::Error e)
	{
		MessageBox(NULL, (L("Error while reading waypoints: ")+e()).c_str(), L("GPX read error"), MB_ICONEXCLAMATION);
		m_Points.erase(m_Points.begin(), m_Points.end());
	}
	catch (_com_error e)
	{
		MessageBox(NULL, (std::wstring(L("Error while reading waypoints: "))+e.ErrorMessage()).c_str(),
			       L("GPX read error"), MB_ICONEXCLAMATION);
		m_Points.erase(m_Points.begin(), m_Points.end());
	}
}

// ---------------------------------------------------------------

void CWaypoints::ReadWPT(const wchar_t * wcFilename)
{
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
				AddPoint(CPoint(dLongitude, dLatitude, iAltitude, buff), iRadius);
			}
		}
	}
	m_bCanWrite = true;
}

// ---------------------------------------------------------------

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
// ---------------------------------------------------------------
std::wstring CWaypoints::GetCreator()
{
	return app.GetGpsVPVersion().AsWStringWithName();
}
// ---------------------------------------------------------------
void CWaypoints::BeginUpdate()
{
	if (0 == m_BeginUpdateCount)
		m_bWriteRequested = false;
	m_BeginUpdateCount++;
}
void CWaypoints::EndUpdate()
{
	m_BeginUpdateCount--;
	if ((0 == m_BeginUpdateCount) && m_bWriteRequested)
	{
		Write();
	}
}
// ---------------------------------------------------------------
int CWaypoints::GetWaypointModelCount()
{
	m_ModelsIdList.clear();
	for(list<CPoint>::iterator it = m_Points.begin();
		it != m_Points.end();
		it++)
	{
		// Waypoint with a name beginning with "~" are considered as models for new waypoints
		if (isWptModel(it->m_wstrName))
			m_ModelsIdList.push_back(it->GetID());
	}
	return m_ModelsIdList.size();
}
// ---------------------------------------------------------------
CWaypoints::CPoint & CWaypoints::GetWaypointModel(int modelIndex)
{
	int modelId = -1;
	if (modelIndex < (int)m_ModelsIdList.size())
		modelId = m_ModelsIdList.at(modelIndex);
	return ById(modelId);
}
// ---------------------------------------------------------------
bool CWaypoints::IsGPX()
{
	std::wstring wstrExt = m_wstrFilename.substr(m_wstrFilename.length()-4, 4);
	return (0 == _wcsnicmp(wstrExt.c_str(), L".gpx", 4));
}
// ---------------------------------------------------------------

CWaypoints::CPointEditor CWaypoints::CPoint::GetEditor()
{
	return CWaypoints::CPointEditor(*this);
}

// ---------------------------------------------------------------
int CWaypoints::CPointEditor::GetPropertyCount() const
{
	return 5+m_pt.m_OSMPropList.size();
}

// ---------------------------------------------------------------

std::auto_ptr<CWaypoints::CPointProp> CWaypoints::CPointEditor::GetPropertyByIndex(int iId)
{
	switch (iId) 
	{
	case 0:
		return std::auto_ptr<CWaypoints::CPointProp>(new CNameProp(m_pt));
	case 1:
		return std::auto_ptr<CWaypoints::CPointProp>(new CLatitudeProp(m_pt, *this));
	case 2:
		return std::auto_ptr<CWaypoints::CPointProp>(new CLongitudeProp(m_pt, *this));
	case 3:
		return std::auto_ptr<CWaypoints::CPointProp>(new CRadiusProp(m_pt));
	case 4:
		return std::auto_ptr<CWaypoints::CPointProp>(new CAltitudeProp(m_pt));
	default:
		if (iId-5 < (int)m_pt.m_OSMPropList.size())
			return std::auto_ptr<CWaypoints::CPointProp>(new CPropProxy(&m_pt.m_OSMPropList[iId-5]));
		else
			return std::auto_ptr<CWaypoints::CPointProp>(NULL);
	}
}

// ---------------------------------------------------------------

// Returns false if the property can't be removed
bool CWaypoints::CPointEditor::RemovePropertyByIndex(int iId)
{	
	if (iId <5) return false;
	int iOsmId = iId-5;
	if ((iOsmId < (int)m_pt.m_OSMPropList.size()) && m_pt.m_OSMPropList[iOsmId].DeleteAllowed())
	{
		 m_pt.m_OSMPropList.erase(m_pt.m_OSMPropList.begin()+iOsmId);
		 return true;
	}
	return false;
}

// ---------------------------------------------------------------

CWaypoints::CPointProp& CWaypoints::CPointEditor::AddOSMProp()
{
	m_pt.m_OSMPropList.push_back(CStringProp(nsOSM, L"", L""));
	return m_pt.m_OSMPropList.back();
}

// ---------------------------------------------------------------

void CWaypoints::CPointEditor::Commit()
{
	if (!m_wstrLon.empty() && !m_wstrLat.empty())
	{
		double dLon, dLat;
		TextToCoord(m_wstrLon, m_wstrLat, dLon, dLat);
		m_pt.Longitude(dLon);
		m_pt.Latitude(dLat);
	}
}

// ---------------------------------------------------------------
