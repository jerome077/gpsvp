/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "Track.h"
#include "MapApp.h"

extern int GetTrackStep();

void CTrack::AddPoint(GeoPoint pt, double timeUTC, double dHDOP)
{
	AutoLock l;
	// Calculate time for file
	double dTimeUTC;
	if (timeUTC != 0)
	{
		dTimeUTC = timeUTC;
	}
	else
	{
		SYSTEMTIME stTime;
		GetSystemTime(&stTime);
		SystemTimeToVariantTime(&stTime, &dTimeUTC);
	}

	m_dLastTimeUTC = dTimeUTC;
	unsigned long ulTimeUTC = (unsigned long)(dTimeUTC * 24 * 60 * 60);
	// Only if point is different
	if (m_Track.empty())
		m_Track.push_back(Segment());
	if (m_fBeginTrack && !m_Track.back().empty())
		m_Track.push_back(Segment());
	if (!m_Track.back().empty() && pt == m_Track.back().back().gp)
		return;
	if (!m_Track.back().empty() && (IntDistance(pt, m_Track.back().back().gp) < (GetTrackStep() /* * dHDOP */) ))
		return;
	if (!m_Track.back().empty() && (IntDistance(pt, m_Track.back().back().gp) > 100) && abs(dTimeUTC - m_dLastTimeUTC) > (3.0 / 24 / 60 / 60))
		Break();
	m_gpLastpoint = pt;
	m_fTrackPresent = true;
	// Add it to list
	int iAltitude = (m_fAltitude)?int(m_dAltitude + 0.5):NO_iALTITUDE;
	m_Track.back().push_back(TrackPoint(pt, ulTimeUTC, iAltitude));
	++m_nPointCount;
	// If we are writing to a file
	if(m_fWriting)
	{
		switch (m_CurrentTrackFormat)
		{
		case tfPLT:
			WritePLT(pt, dTimeUTC);
			break;
		case tfGPX:
			WriteGPX(pt, dTimeUTC, dHDOP);
			break;
		}
		// Flush the file to save the point
		if (m_iBufferPos >= 4096)
		{
			Flush(m_iBufferPos);
			m_iBufferPos = 0;
			//m_iBufferPos -= 4096;
			//memcpy(m_writeBuffer, m_writeBuffer + 4096, m_iBufferPos);
		}
	}
	m_fBeginTrack = false;
	// If we can compress track
	if (m_fCompressable)
	{
		// While we need to compress some more
		while (m_nPointCount > cnMaxPoints)
		{
			// Remove any empty segments (though there souldn't be)
			while(m_Track.front().empty())
				m_Track.pop_front();
			// Transfer one point
			m_CompressedTrack.back().push_back(
				m_Track.front().front());
			// Whether we sould start a new semgent:
			bool fNextSegment = false;
			// Remove the required number of points
			for (int i = 0; i < cnCompressRatio; ++i)
			{
				// Removing one point
				m_Track.front().pop_front();
				--m_nPointCount;
				// Removing empty segments (one could appear)
				while(m_Track.front().empty())
				{
					m_Track.pop_front();
					// In that case we should start a new segment in compressed track too
					fNextSegment = true;
				}
			}
			// Start if we should. We know that the last segment was not empty.
			if (fNextSegment)
				m_CompressedTrack.push_back(Segment());
		}
	}
}

void CTrack::WritePLT(GeoPoint pt, double dTimeUTC)
{
	double dLocalTime;
	UTCVariantTimeToLocalVariantTime(dTimeUTC, dLocalTime);
	// Output std::string to file
	m_iBufferPos += _snprintf(m_writeBuffer + m_iBufferPos, 1024, "%2.8f,%2.8f,%d,%.1f,%6.8f,,\r\n",
		                      Degree(pt.lat), Degree(pt.lon), m_fBeginTrack ? 1 : 0,
							  (m_fAltitude ? m_dAltitude / cdFoot : -777), dLocalTime);
	// Since a point was added, the track should continue
	m_fBeginFile = false;
}

void CTrack::WriteGPX(GeoPoint pt, double dTimeUTC, double dHDOP)
{
	// Output std::string to file
	if (m_fBeginTrack && !m_fBeginFile)
		m_iBufferPos += _snprintf(m_writeBuffer + m_iBufferPos, 1024, "</trkseg><trkseg>\r\n");
	std::string strEle;
	if (m_fAltitude)
		strEle = "<ele>"+DoubleToStr(m_dAltitude)+"</ele>";
	double dTimeUTCint, dTimeUTCfrac;
	dTimeUTCfrac = modf(dTimeUTC * (24*60*60), &dTimeUTCint);  // Assuming year>1900, i.e. dTimeUTC>0
	SYSTEMTIME st;
	VariantTimeToSystemTime(dTimeUTCint / (24.0*60*60), &st);
	m_iBufferPos += _snprintf(m_writeBuffer + m_iBufferPos, 1024,
		                "<trkpt lat=\"%2.8f\" lon=\"%2.8f\">%s<time>%04d-%02d-%02dT%02d:%02d:%06.3fZ</time></trkpt>\r\n",
						Degree(pt.lat), Degree(pt.lon),
						strEle.c_str(),
						st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond + dTimeUTCfrac);

	// Since a point was added, the track should continue
	m_fBeginFile = false;
}

CTrack::track_t not_selected_track_t(CTrack::track_t uiType)
{
	// NB: track types corresponds to color definitions in *.vpc
	switch (uiType)
	{
	case CTrack::typeCurrentTrack:		return CTrack::typeCurrentTrack_NotSelected;
	case CTrack::typeOldTrack:			return CTrack::typeOldTrack_NotSelected;
	case CTrack::typeRoute:				return CTrack::typeRoute_NotSelected;
	case CTrack::typeRouteInEdition:	return CTrack::typeRoute_NotSelected;
	default:							return uiType;
	}
}

UInt track_t2UInt(CTrack::track_t uiType, bool bSelectedPart)
{
	return (bSelectedPart) ? uiType : not_selected_track_t(uiType);
}

bool CTrack::IsSelectedPart(int iPointIndex)
{
	return (  (iPointIndex >= m_StartCursor)
		   && ((-1 == m_EndCursor) || (iPointIndex <= m_EndCursor))
		   );
}

void CTrack::PaintUnlocked(IPainter * pPainter, track_t uiType)
{
	int currentIndex = 0;
	bool bWasSelectedPart = IsSelectedPart(currentIndex);
	StartTimes::iterator itTimeUTC = m_startTimesUTC.begin();
	for (Track::iterator itSeg = m_Track.begin(); itSeg != m_Track.end(); ++itSeg)
	{
		if (!itSeg->empty())
		{
			bool bSelectedPart = IsSelectedPart(currentIndex);
			pPainter->StartPolyline(track_t2UInt(uiType, bSelectedPart), 0);
			// Add all the points
			for (Segment::iterator it = itSeg->begin(); it != itSeg->end(); ++it)
			{
				// Start of the selected part
				if (bSelectedPart && !bWasSelectedPart)
				{
					pPainter->AddPoint(it->gp);
					// New polyline with color change
					pPainter->FinishObject();
					pPainter->StartPolyline(track_t2UInt(uiType, bSelectedPart), 0);
				}
				// Draw track
				pPainter->AddPoint(it->gp);
				if (itTimeUTC != m_startTimesUTC.end() && *itTimeUTC + m_ulCompetitionTime <= it->timeUTC)
				{
					pPainter->PaintPoint(0x10003, it->gp, 0);
					++itTimeUTC;
				}
				bWasSelectedPart = bSelectedPart;
				++currentIndex;
				bSelectedPart = IsSelectedPart(currentIndex);
				// End of the selected part
				if (bWasSelectedPart && !bSelectedPart)
				{
					// New polyline with color change
					pPainter->FinishObject();
					pPainter->StartPolyline(track_t2UInt(uiType, bSelectedPart), 0);
					pPainter->AddPoint(it->gp);
				}
			}
			// And paint the polyline
			pPainter->FinishObject();
		}
	}
}
void CTrack::CreateFile()
{
	m_fBeginFile = true;
	// Track should begin
	m_fBeginTrack = true;
	m_fTrackPresent = false;
	m_wstrFilenameInt = L"";
	switch (app.m_riTrackFormat())
	{
	case tfPLT:
		CreateFilePLT();
		break;
	case tfGPX:
		CreateFileGPX();
		break;
	}
}

void CTrack::CreateFilePLT()
{
	m_CurrentTrackFormat = tfPLT;
	// Write file header
	m_iBufferPos = _snprintf(m_writeBuffer, 4096,
		"OziExplorer Track Point File Version 2.1\r\n"
		"WGS 84\r\n"
		"Altitude is in Feet\r\n"
		"Reserved\r\n"
		"0,2,%d,,0,0,2,0\r\n"
		"1\r\n", m_iColor);
}

std::string CTrack::GetCreator()
{
	return app.GetGpsVPVersion().AsStringWithName();
}

void CTrack::CreateFileGPX()
{
	m_CurrentTrackFormat = tfGPX;
#ifndef UNDER_WINE
	m_FilePosForAdding = 0;
#endif // UNDER_WINE
	// Write file header
	GetFileName(); // initializes m_strGPXName
	m_iBufferPos = _snprintf(m_writeBuffer, 4096,
		"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\r\n"
		"<gpx version=\"1.1\" creator=\"%s\">\r\n"
		"<trk><name>%s</name>\r\n"
		"<trkseg>\r\n", GetCreator().c_str(), m_strGPXName.c_str());
}

void CTrack::FlushPLT(int iSize)
{
	if (m_fWriting && m_fTrackPresent)
	{
		FILE * pFile = wfopen(GetFileName(), L"ab");
		if (pFile)
		{
			fwrite(m_writeBuffer, 1, iSize, pFile);
			fclose(pFile);
		}
	}
}

void CTrack::FlushGPX(int iSize)
{
	static const char* sCloseXml = "</trkseg></trk></gpx>";
	static const LONG iCloseLength = strlen(sCloseXml);
	if (m_fWriting && m_fTrackPresent)
	{
		HANDLE hFile = ::CreateFile(GetFileName(), (GENERIC_READ | GENERIC_WRITE),
			                        FILE_SHARE_READ, NULL, 
							        OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE != hFile)
		{
			LONG fileSize = GetFileSize(hFile, NULL);
			if (fileSize >= iCloseLength)
				SetFilePointer(hFile, -LONG(iCloseLength), 0, FILE_END);
			DWORD dwWritten;
			WriteFile(hFile, m_writeBuffer, iSize, &dwWritten, NULL);
			WriteFile(hFile, sCloseXml, iCloseLength, &dwWritten, NULL);
			CloseHandle(hFile);
		}
	}
}

//! Tell the track that it is broken (missing points)
void CTrack::Break()
{
	AutoLock l;
	// So it has to begin
	m_fBeginTrack = true;
}

void CTrack::Read(const std::wstring& wstrFilename)
{
	AutoLock l;
	std::wstring wstrExt = wstrFilename.substr(wstrFilename.length()-4, 4);
#ifndef UNDER_WINE
	if (0 == _wcsnicmp(wstrExt.c_str(), L".gpx", 4))
		ReadFirstTrackFromGPX(wstrFilename);
	else
#endif // UNDER_WINE
		ReadPLT(wstrFilename);
}

#ifndef UNDER_WINE
void CTrack::ReadGPX(const std::auto_ptr<CGPXTrack>& apTrack, const std::wstring& wstrTrackname)
{
	m_wstrFilenameExt = wstrTrackname; 
	std::auto_ptr<CGPXTrackSeg> apTrackSeg = apTrack->firstTrackSeg();
	while (!apTrackSeg->eof())
	{
		Break();	// New segment (don't do anything if the previous segment has no point)
		std::auto_ptr<CGPXTrackPoint> apTrackPoint = apTrackSeg->firstTrackPoint();
		while (!apTrackPoint->eof())
		{
			double dAltitude = apTrackPoint->getAltitude();
			if (-777 == dAltitude)
				ResetAltitude();
			else
				SetAltitude(dAltitude);
			AddPoint(GeoPoint(FromDegree(apTrackPoint->getLongitude()),
							  FromDegree(apTrackPoint->getLatitude())
							  ), apTrackPoint->getUTCTime());
			apTrackPoint = apTrackSeg->nextTrackPoint();
		}
		apTrackSeg = apTrack->nextTrackSeg();
	}
}
#endif

#ifndef UNDER_WINE
void CTrack::ReadFirstTrackFromGPX(const std::wstring& wstrFilename)
{
	try
	{
		ComInit MyObjectToInitCOM;
		{
			CGPXFileReader GpxReader(wstrFilename);
			std::auto_ptr<CGPXTrack> apTrack = GpxReader.firstTrack();
			if (!apTrack->eof())
			{
				ReadGPX(apTrack, wstrFilename);
			}
		}
	}
	catch (CGPXFileReader::Error e)
	{
		MessageBox(NULL, (L("Error while reading track: ")+e()).c_str(), L("GPX read error"), MB_ICONEXCLAMATION);
	}
#ifndef UNDER_WINE
	catch (_com_error e)
	{
		MessageBox(NULL, (std::wstring(L("Error while reading track: "))+e.ErrorMessage()).c_str(),
			       L("GPX read error"), MB_ICONEXCLAMATION);
	}
#endif // UNDER_WINE
}
#endif // UNDER_WINE

void CTrack::ReadPLT(const std::wstring& wstrFilename)
{
	m_wstrFilenameExt = wstrFilename;
	char buff[100];
	FILE * pFile = wfopen(wstrFilename.c_str(), L"rt");
	if (!pFile)
		return;
	std::vector<long> vRecord;
	for (int i = 0; i < 6; ++i)
	{
		if (!fgets(buff, sizeof(buff), pFile))
			break;
	}
	std::vector<std::string> listParts;
	while(fgets(buff, sizeof(buff), pFile))
	{
		std::string strCommand = buff;
		listParts.resize(0);
		std::string::size_type pos = 0;
		std::string::size_type nextpos = 0;
		while ((nextpos = strCommand.find(',', pos)) != std::string::npos)
		{
			listParts.push_back(strCommand.substr(pos, nextpos - pos));
			pos = nextpos + 1;
		}
		listParts.push_back(strCommand.substr(pos));
		if (listParts.size() >= 3)
		{
			if (myatof(listParts[2].c_str()) == 1)
				Break();
		}
		double dTimeUTC = 0;
		// _snprintf(m_writeBuffer + m_iBufferPos, 1024, "%2.8f,%2.8f,%d,%.1f,%6.8f,,\r\n", Degree(pt.lat), Degree(pt.lon), m_fBeginTrack ? 1 : 0, (m_fAltitude ? m_dAltitude / cdFoot : -777),dTime);
		if (listParts.size() >= 5)
		{
			double dLocalTime = myatof(listParts[4].c_str());
			LocalVariantTimeToUTCVariantTime(dLocalTime, dTimeUTC);
		}
		if (listParts.size() >= 4)
		{
			double dAltitude = myatof(listParts[4].c_str());
			if (-777 == dAltitude)
				ResetAltitude();
			else
				SetAltitude(dAltitude);
		}
		if (listParts.size() >= 2)
		{
			double dLatitude = myatof(listParts[0].c_str());
			double dLongitude = myatof(listParts[1].c_str());
			AddPoint(GeoPoint(FromDegree(dLongitude), FromDegree(dLatitude)), dTimeUTC);
		}
	}
}
const std::wstring CTrack::GetExtFilename() const
{
	AutoLock l;
	return m_wstrFilenameExt;
}
bool CTrack::IsPresent() const
{
	AutoLock l;
	return m_fTrackPresent;
}
GeoPoint CTrack::GetLastPoint() const
{
	AutoLock l;
	return m_gpLastpoint;
}
void CTrack::SetAltitude(double dAltitude)
{
	AutoLock l;
	m_dAltitude = dAltitude;
	m_fAltitude = true;
}
void CTrack::ResetAltitude()
{
	AutoLock l;
	m_fAltitude = false;
}
void CTrack::SetCompressable()
{
	AutoLock l;
	m_fCompressable = true;
}

const wchar_t * CTrack::GetFileName()
{
	if (m_wstrFilenameInt == L"")
	{
		wchar_t wcFilename[50];
		SYSTEMTIME st;
		GetLocalTime(&st);
		wsprintf(wcFilename, L"%04d.%02d.%02d-%02d.%02d.%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		switch (m_CurrentTrackFormat)
		{
		case tfPLT:
			m_wstrFilenameInt = app.m_rsTrackFolder() + L"\\" + wcFilename + L".plt";		
			break;
		case tfGPX:
			m_wstrFilenameInt = app.m_rsTrackFolder() + L"\\" + wcFilename + L".gpx";
			char cFilename[50];
			sprintf(cFilename, "%04d.%02d.%02d-%02d.%02d.%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
			m_strGPXName = cFilename;
			break;
		}
	}
	return m_wstrFilenameInt.c_str();
}

void CTrack::SetCompetition(const GeoPoint & gp, unsigned long ulCompetitionTime)
{
	bool fOld = (gp == m_gpCompetition && m_ulCompetitionTime <= ulCompetitionTime);
	m_ulCompetitionTime = ulCompetitionTime;
	if (fOld)
		return;
	m_startTimesUTC.clear();
	m_gpCompetition = gp;
	m_ulCompetitionTime = ulCompetitionTime;
	bool isnear = false;
	int bestDistance;
	unsigned long bestTimeUTC;
	for (Track::iterator itSeg = m_Track.begin(); itSeg != m_Track.end(); ++itSeg)
	{
		if (!itSeg->empty())
		{
			for (Segment::iterator it = itSeg->begin(); it != itSeg->end(); ++it)
			{
				int distance = IntDistance(gp, it->gp);
				if (distance < 50)
				{
					if (isnear)
					{
						if (distance <= bestDistance)
						{
							bestDistance = distance;
							bestTimeUTC = it->timeUTC;
						}
					}
					else
					{
						isnear = true;
						bestDistance = distance;
						bestTimeUTC = it->timeUTC;
					}
				}
				else
				{
					if (isnear)
					{
						isnear = false;
						m_startTimesUTC.push_back(bestTimeUTC);
					}
				}
			}
		}
	}
}

// Returns index of the nearest point on the track + the distance in iDistance.
// Returns -1 for the last point (so that the track can grow and the point stay at the end).
int CTrack::FindNearestPointIndex(const GeoPoint & gp, int& iDistance)
{
	int currentIndex = 0, nearestIndex = -1;
	iDistance = INT_MAX; 
	for (Track::iterator itSeg = m_Track.begin(); itSeg != m_Track.end(); ++itSeg)
	{
		for (Segment::iterator it = itSeg->begin(); it != itSeg->end(); ++it)
		{
			int d = IntDistance(it->gp, gp);
			if (d < iDistance)
			{
				iDistance = d;
				nearestIndex = currentIndex;
			}
			++currentIndex;
		}
	}
	if (nearestIndex >= m_nPointCount)	nearestIndex = -1;
	return nearestIndex;
}

// Returns the index of the nearest segment:
// * Index of a segment = index of the second point of the segment: 1 for the first one...
// * Returns 0 if the point seems to be before the first segment.
// * Returns -1 if the projection is after the last segment.
// * Considers gaps as normal segments
// * returns the next target in NextPoint: 2nd point of the segment or end point
int CTrack::FindNearestSegmentIndex(const GeoPoint & gp, GeoPoint& NextPoint)
{
	int currentIndex = 0, nearestIndex = 0;
	// Track is empty => -1 and no point
	if ((m_Track.begin() == m_Track.end()) || (m_Track.begin()->begin() == m_Track.begin()->end()))
		return -1;  
	// Check the distance to the first point:
	GeoPoint gp1 = m_Track.begin()->begin()->gp;
	double dDistance = DoubleDistance(gp1, gp);
	NextPoint = gp1;
	// Check the distance to each segment:
	for (Track::iterator itSeg = m_Track.begin(); itSeg != m_Track.end(); ++itSeg)
	{
		for (Segment::iterator it = itSeg->begin(); it != itSeg->end(); ++it)
		{
			if (gp1 != it->gp)
			{
				double d = DoubleDistanceToSegment(gp, gp1, it->gp);
				if (d < dDistance)
				{
					dDistance = d;
					nearestIndex = currentIndex;
					NextPoint = it->gp;
				}
				gp1 = it->gp;
			}
			++currentIndex;
		}
	}
	// Check the distance to the last point:
	double d = DoubleDistance(gp1, gp);
	if (d <= dDistance) // might be egal
	{
		dDistance = d;
		nearestIndex = -1;
		NextPoint = gp1;
	}
	return nearestIndex;
}

// Index -1 will add the point at the end
void CTrack::InsertPoint(int iNextPointIndex, const GeoPoint & gp)
{
	if (-1 == iNextPointIndex)
	{
		AddPoint(gp, 0);
		return;
	}
	int currentIndex = 0;
	for (Track::iterator itSeg = m_Track.begin(); itSeg != m_Track.end(); ++itSeg)
	{
		for (Segment::iterator it = itSeg->begin(); it != itSeg->end(); ++it)
		{
			if (currentIndex == iNextPointIndex)
			{
				itSeg->insert(it, TrackPoint(gp, 0, NO_iALTITUDE));
				++m_nPointCount;
				return;
			}
			++currentIndex;
		}
	}
}

void CTrack::ErasePoint(int iPointIndex)
{
	int currentIndex = 0;
	for (Track::iterator itSeg = m_Track.begin(); itSeg != m_Track.end(); ++itSeg)
	{
		for (Segment::iterator it = itSeg->begin(); it != itSeg->end(); ++it)
		{
			if (currentIndex == iPointIndex)
			{
				itSeg->erase(it);
				--m_nPointCount;
				return;
			}
			++currentIndex;
		}
	}
}

int CTrack::GetInfoCount() const
{
	return 7;
}

std::wstring CTrack::GetInfo(int infoIndex) const
{
	std::wstring NL = L"\n";
	std::wstring SEP = L", ";
	switch (infoIndex)
	{
	case 0:
		return std::wstring(L("Name")) + L": " + GetExtFilename();
	case 1:
		return std::wstring(L("Point count")) + L": " + IntToText(m_nPointCount);
	case 2:
		{
			std::wstring sResult = std::wstring(L"-- ") + L("Raw lengths") + L" --";
			double dFullLength, dPartLength, dFullGapLength, dPartGapLength;
			int iSegmentCount;
			CalcLengthsUnlocked(m_StartCursor, m_EndCursor, 0,
				                dFullLength, dPartLength, dFullGapLength, dPartGapLength, iSegmentCount);
			sResult += NL + L("Full") + L": " + DistanceToText(dFullLength);
			if (iSegmentCount > 0)
				sResult += SEP + L("Gaps") + L": " + DistanceToText(dFullGapLength);
			sResult += NL + L("Partial") + L": " + DistanceToText(dPartLength);
			if (iSegmentCount > 0)
				sResult += SEP + L("Gaps") + L": " + DistanceToText(dPartGapLength);
			sResult += NL + L("Segment count") + L": " + IntToText(iSegmentCount);
			return sResult + L"\n";
		}
	case 3:
		{
			std::wstring sResult = std::wstring(L"-- ") + L("Smoothed lengths (10m) ") + L" --";
			double dFullLength, dPartLength, dFullGapLength, dPartGapLength;
			int iSegmentCount;
			CalcLengthsUnlocked(m_StartCursor, m_EndCursor, 10,
				                dFullLength, dPartLength, dFullGapLength, dPartGapLength, iSegmentCount);
			sResult += NL + L("Full") + L": " + DistanceToText(dFullLength);
			sResult += NL + L("Partial") + L": " + DistanceToText(dPartLength);
			return sResult + L"\n";
		}
	case 4:
		{
			std::wstring sResult = std::wstring(L"-- ") + L("Times") + L" --";
			unsigned long ulStartTimeUTC, ulEndTimeUTC, ulIndex1TimeUTC, ulIndex2TimeUTC;
			CalcTimesUnlocked(m_StartCursor, m_EndCursor,
				              ulStartTimeUTC, ulEndTimeUTC, ulIndex1TimeUTC, ulIndex2TimeUTC);
			sResult += NL + L("Start time") + L": " + UTCTimeToLocalTimeText(ulStartTimeUTC);
			sResult += NL + L("End time") + L": " + UTCTimeToLocalTimeText(ulEndTimeUTC);
			sResult += NL + L("Duration") + L": " + DurationToText(ulEndTimeUTC-ulStartTimeUTC);
			if ((-1 != m_StartCursor) || (-1 != m_EndCursor))
			{
				sResult += NL + L("Start cursor") + L": " + IntToText(m_StartCursor);
				sResult += NL + L("End cursor") + L": " + IntToText(m_EndCursor);
				sResult += NL + L("Partial Start time") + L": " + UTCTimeToLocalTimeText(ulIndex1TimeUTC);
				sResult += NL + L("Partial End time") + L": " + UTCTimeToLocalTimeText(ulIndex2TimeUTC);
				sResult += NL + L("Partial Duration") + L": " + DurationToText(ulIndex2TimeUTC-ulIndex1TimeUTC);
			}
			return sResult + L"\n";
		}
	case 5:
		{
			std::wstring sResult = std::wstring(L"-- ") + L("Elevation differences") + L" --";
			int iFullEleDiffUp, iPartEleDiffUp, iFullEleDiffDown, iPartEleDiffDown;
			int iFullGapEleDiffUp, iPartGapEleDiffUp, iFullGapEleDiffDown, iPartGapEleDiffDown;
			int iSegmentCount;
			CalcEleDiffUnlocked(m_StartCursor, m_EndCursor, 0,
								iFullEleDiffUp, iPartEleDiffUp, iFullEleDiffDown, iPartEleDiffDown,
								iFullGapEleDiffUp, iPartGapEleDiffUp, iFullGapEleDiffDown, iPartGapEleDiffDown,
								iSegmentCount);
			sResult += NL + L("Full") + L": " + HeightToText(iFullEleDiffUp) + SEP + HeightToText(iFullEleDiffDown);
			if (iSegmentCount > 0)
				sResult += NL + L" " + L("Gaps") + L": " + HeightToText(iFullGapEleDiffUp) + SEP + HeightToText(iFullGapEleDiffDown);
			sResult += NL + L("Partial") + L": " + HeightToText(iPartEleDiffUp) + SEP + HeightToText(iPartEleDiffDown);
			if (iSegmentCount > 0)
				sResult += NL + L" " + L("Gaps") + L": " + HeightToText(iPartGapEleDiffUp) + SEP + HeightToText(iPartGapEleDiffDown);
			return sResult + L"\n";
		}
	case 6:
		{
			std::wstring sResult = std::wstring(L"-- ") + L("Elevation differences, smoothed (30m)") + L" --";
			int iFullEleDiffUp, iPartEleDiffUp, iFullEleDiffDown, iPartEleDiffDown;
			int iFullGapEleDiffUp, iPartGapEleDiffUp, iFullGapEleDiffDown, iPartGapEleDiffDown;
			int iSegmentCount;
			CalcEleDiffUnlocked(m_StartCursor, m_EndCursor, 30,
								iFullEleDiffUp, iPartEleDiffUp, iFullEleDiffDown, iPartEleDiffDown,
								iFullGapEleDiffUp, iPartGapEleDiffUp, iFullGapEleDiffDown, iPartGapEleDiffDown,
								iSegmentCount);
			sResult += NL + L("Full") + L": " + HeightToText(iFullEleDiffUp) + SEP + HeightToText(iFullEleDiffDown);
			sResult += NL + L("Partial") + L": " + HeightToText(iPartEleDiffUp) + SEP + HeightToText(iPartEleDiffDown);
			return sResult + L"\n";
		}
	default:
		return L"";
	}
}

double CTrack::GetFullLength() const
{
	double dFullLength, dPartLength, dFullGapLength, dPartGapLength;
	int iSegmentCount;
	CalcLengthsUnlocked(-1, -1, 0, dFullLength, dPartLength,
		                dFullGapLength, dPartGapLength, iSegmentCount);
	return dFullLength;
}

// Calculate the length between two indexes (-1 for the whole track).
// smoothing = number of meters that the point must differs (0 to deactivate).
void CTrack::CalcLengthsUnlocked(int index1, int index2, double smoothing,
								 double& dFullLength, double& dPartLength,
								 double& dFullGapLength, double& dPartGapLength,
								 int& iSegmentCount) const
{
	dPartLength = 0;
	dPartGapLength = 0;
	iSegmentCount = 0;
	double dNotPartLength = 0;
	double dNotPartGapLength = 0;
	int currentIndex = -1;
	if (index2 < 0) index2 = INT_MAX;
	const GeoPoint* pPreviousGP = NULL;
	for (Track::const_iterator itSeg = m_Track.begin(); itSeg != m_Track.end(); ++itSeg)
	{
		++iSegmentCount;
		Segment::const_iterator it = itSeg->begin();
		if (it == itSeg->end()) continue;
		++currentIndex;
		if (NULL != pPreviousGP)
		{
			double d = DoubleDistance(it->gp, *pPreviousGP);
			if ((currentIndex <= index1) || (currentIndex > index2))
				dNotPartGapLength += d;
			else
				dPartGapLength += d;
		}
		pPreviousGP = &(it->gp);
		// Length of a segment:
		for (++it; it != itSeg->end(); it++)
		{
			++currentIndex;
			double d = DoubleDistance(it->gp, *pPreviousGP);

			// Ignore point because of smoothing?
			if	(  (d < smoothing)				// Distance too small
				&& (currentIndex != index1)		// Not start cursor
				&& (currentIndex != index2)		// Not end cursor
				&& (&(*it) != &itSeg->back()) )	// Not last point of segment
				continue;

			pPreviousGP = &(it->gp);
			if ((currentIndex <= index1) || (currentIndex > index2))
				dNotPartLength += d;
			else
				dPartLength +=d;
		}
	}
	dFullLength = dPartLength + dNotPartLength;
	dFullGapLength = dPartGapLength + dNotPartGapLength;
}


// smoothing = number of meters that the point must differs (0 to deactivate).
void CTrack::CalcEleDiffUnlocked(int index1, int index2, int smoothing,
								 int& iFullEleDiffUp, int& iPartEleDiffUp,
								 int& iFullEleDiffDown, int& iPartEleDiffDown,
								 int& iFullGapEleDiffUp, int& iPartGapEleDiffUp,
								 int& iFullGapEleDiffDown, int& iPartGapEleDiffDown,
							     int& iSegmentCount) const
{
	iPartEleDiffUp = 0;
	iPartGapEleDiffUp = 0;
	iPartEleDiffDown = 0;
	iPartGapEleDiffDown = 0;
	iSegmentCount = 0;
	int iNotPartEleDiffUp = 0;
	int iNotPartGapEleDiffUp = 0;
	int iNotPartEleDiffDown = 0;
	int iNotPartGapEleDiffDown = 0;
	int currentIndex = -1;
	if (index2 < 0) index2 = INT_MAX;
	int iPreviousAltitude = NO_iALTITUDE;
	for (Track::const_iterator itSeg = m_Track.begin(); itSeg != m_Track.end(); ++itSeg)
	{
		++iSegmentCount;
		Segment::const_iterator it = itSeg->begin();
		if (it == itSeg->end()) continue;
		++currentIndex;
		if (NO_iALTITUDE != iPreviousAltitude)
		{
			int d = it->altitude - iPreviousAltitude;
			if ((currentIndex <= index1) || (currentIndex > index2))
			{
				if (d >= 0)
					iNotPartGapEleDiffUp += d;
				else
					iNotPartGapEleDiffDown += d;
			}
			else
			{
				if (d >= 0)
					iPartGapEleDiffUp += d;
				else
					iPartGapEleDiffDown += d;
			}
		}
		iPreviousAltitude = it->altitude;
		// Length of a segment:
		for (++it; it != itSeg->end(); it++)
		{
			++currentIndex;
			int d = it->altitude - iPreviousAltitude;

			// Ignore point because of smoothing?
			if	(  (abs(d) < smoothing)			// Distance too small
				&& (currentIndex != index1)		// Not start cursor
				&& (currentIndex != index2)		// Not end cursor
				&& (&(*it) != &itSeg->back()) )	// Not last point of segment
				continue;

			iPreviousAltitude = it->altitude;
			if ((currentIndex <= index1) || (currentIndex > index2))
			{
				if (d >= 0)
					iNotPartEleDiffUp += d;
				else
					iNotPartEleDiffDown += d;
			}
			else
			{
				if (d >= 0)
					iPartEleDiffUp += d;
				else
					iPartEleDiffDown += d;
			}
		}
	}
	iFullEleDiffUp = iPartEleDiffUp + iNotPartEleDiffUp;
	iFullEleDiffDown = iPartEleDiffDown + iNotPartEleDiffDown;
	iFullGapEleDiffUp = iPartGapEleDiffUp + iNotPartGapEleDiffUp;
	iFullGapEleDiffDown = iPartGapEleDiffDown + iNotPartGapEleDiffDown;
}


void CTrack::CalcTimesUnlocked(int index1, int index2,
							   unsigned long& ulStartTimeUTC, unsigned long& ulEndTimeUTC,
							   unsigned long& ulIndex1TimeUTC, unsigned long& ulIndex2TimeUTC) const
{
	ulStartTimeUTC = 0;
	int currentIndex = -1;
	for (Track::const_iterator itSeg = m_Track.begin(); itSeg != m_Track.end(); ++itSeg)
	{
		for (Segment::const_iterator it = itSeg->begin(); it != itSeg->end(); it++)
		{
			++currentIndex;
			if (0 == currentIndex)
				ulStartTimeUTC = it->timeUTC;
			if (index1 == currentIndex)
				ulIndex1TimeUTC = it->timeUTC;
			if (index2 == currentIndex)
				ulIndex2TimeUTC = it->timeUTC;
			ulEndTimeUTC = it->timeUTC;
		}
	}
}

void CTrack::SetWritingWithFilename(const std::wstring& wstrNewFilename)
{
	SetWriting(true);
	m_wstrFilenameInt = wstrNewFilename;
	m_wstrFilenameExt = wstrNewFilename;
}

void CTrack::AppendToTrackOnlyCoord(CTrack & aTrack) const
{
	for (Track::const_iterator itSeg = m_Track.begin(); itSeg != m_Track.end(); ++itSeg)
	{
		aTrack.Break();
		for (Segment::const_iterator it = itSeg->begin(); it != itSeg->end(); ++it)
		{
			aTrack.AddPoint(it->gp, 0);
		}
	}
}

void CTrack::ReverseAppendToTrackOnlyCoord(CTrack & aTrack) const
{
	for (Track::const_reverse_iterator itSeg = m_Track.rbegin(); itSeg != m_Track.rend(); ++itSeg)
	{
		aTrack.Break();
		for (Segment::const_reverse_iterator it = itSeg->rbegin(); it != itSeg->rend(); ++it)
		{
			aTrack.AddPoint(it->gp, 0);
		}
	}
}

CTrack::TrackPoint& CTrack::GetTrackPoint(int iPointIndex)
{
	int currentIndex = 0;
	for (Track::iterator itSeg = m_Track.begin(); itSeg != m_Track.end(); ++itSeg)
	{
		for (Segment::iterator it = itSeg->begin(); it != itSeg->end(); ++it)
		{
			if (currentIndex == iPointIndex)
			{
				return *it;
			}
			++currentIndex;
		}
	}
	return m_Track.back().back();
}

// Calculate the distance from a point to each end of the track:
// * distance = ditance from the point to the next track point in the direction + distance along the track
// * returns the index of the next point in the forward direction (and the point it self in NextForwardPoint).
Int CTrack::CalculateDistanceOnTrack(const GeoPoint & gp,
									 double& ForwardDistance, GeoPoint& NextForwardPoint)
{
	// Forward distance:
	Int indexForward = FindNearestSegmentIndex(gp, NextForwardPoint);
	if (-1 == indexForward)
		ForwardDistance = 0;
	else
	{
		double dFullLength, dPartLength, dFullGapLength, dPartGapLength;
		int iSegmentCount;
		CalcLengthsUnlocked(indexForward, -1, 0, dFullLength, dPartLength,
		                    dFullGapLength, dPartGapLength, iSegmentCount);
		ForwardDistance = dPartLength + DoubleDistance(gp, GetTrackPoint(indexForward).gp);
	}

	return indexForward;
}


// ---------------------------------------------------------------

CTrackList::CTrackList()
{
}

void CTrackList::GetTrackList(IListAcceptor * pAcceptor)
{
	int iIndex = 0;
	for (std::list<CTrack>::iterator it = m_Tracks.begin(); it != m_Tracks.end();++it, ++iIndex)
		pAcceptor->AddItem(it->GetExtFilename().c_str(), iIndex);
}

Int CTrackList::OpenTracks(const std::wstring& wstrFile)
{
	std::wstring wstrExt = wstrFile.substr(wstrFile.length()-4, 4);
#ifndef UNDER_WINE
	if (0 == _wcsnicmp(wstrExt.c_str(), L".gpx", 4))
		return OpenTracksGPX(wstrFile);
	else
#endif // UNDER_WINE
		return OpenTrackPLT(wstrFile);
}

Int CTrackList::OpenTrackPLT(const std::wstring& wstrFile)
{
	m_Tracks.push_back(CTrack());
	m_Tracks.back().ReadPLT(wstrFile);
	if (m_Tracks.back().IsPresent())
	{
		return m_Tracks.size()-1;
	}
	else
	{
		m_Tracks.pop_back();
		return -1;
	}
}

#ifndef UNDER_WINE
Int CTrackList::OpenTracksGPX(const std::wstring& wstrFile)
{
	Int Result = -1;
	try
	{
		ComInit MyObjectToInitCOM;
		{
			CGPXFileReader GpxReader(wstrFile);
			GpxReader.setReadTime(!app.m_Options[mcoQuickReadGPXTrack]);
			GpxReader.setReadAltitude(!app.m_Options[mcoQuickReadGPXTrack]);
			std::auto_ptr<CGPXTrack> apTrack = GpxReader.firstTrack();
			if (apTrack->eof())
				MessageBox(NULL, L("No track in this file"), L("GPX read error"), MB_ICONEXCLAMATION);
			while (!apTrack->eof())
			{
				std::wstring wstrTrackname;
				if (!(app.m_Options[mcoMultitrackAsSingleTrack] && (-1 != Result)))
				{
					m_Tracks.push_back(CTrack());
					wstrTrackname = apTrack->getName() + L" - " + wstrFile;
				}
				else
					wstrTrackname = wstrFile;

				m_Tracks.back().ReadGPX(apTrack, wstrTrackname);
				
				if (m_Tracks.back().IsPresent())
				{
					Result = m_Tracks.size()-1;
				}
				else if (!(app.m_Options[mcoMultitrackAsSingleTrack] && Result))
					m_Tracks.pop_back();
				apTrack = GpxReader.nextTrack();
			}
		}
	}
	catch (CGPXFileReader::Error e)
	{
		MessageBox(NULL, (L("Error while reading track: ")+e()).c_str(), L("GPX read error"), MB_ICONEXCLAMATION);
	}
#ifndef UNDER_WINE
	catch (_com_error e)
	{
		MessageBox(NULL, (std::wstring(L("Error while reading track: "))+e.ErrorMessage()).c_str(),
			       L("GPX read error"), MB_ICONEXCLAMATION);
	}
#endif // UNDER_WINE
	return Result;
}
#endif // UNDER_WINE

CTrack& CTrackList::GetTrack(Int iIndex)
{
	std::list<CTrack>::iterator it;
	for (it = m_Tracks.begin(); it != m_Tracks.end(); ++it)
	{
		if (0 == iIndex) return *it;
		--iIndex;
	}
	return Last();
}

void CTrackList::CloseTrack(Int iIndex)
{
	std::list<CTrack>::iterator it;
	for (it = m_Tracks.begin(); it != m_Tracks.end(); ++it)
	{
		if (!iIndex)
			break;
		--iIndex;
	}
	if (it != m_Tracks.end())
	{
		m_Tracks.erase(it);
	}
}

// returns -1 when not found
Int CTrackList::FindTrackIndex(const CTrack& aTrack)
{
	Int CurrentIndex = 0;
	std::list<CTrack>::iterator it;
	for (it = m_Tracks.begin(); it != m_Tracks.end(); ++it)
	{
		if (&(*it) == &aTrack)
			return CurrentIndex;
		CurrentIndex++;
	}
	return -1;
}

Int CTrackList::NewTrack(const std::wstring& wstrDefaultname)
{
	m_Tracks.push_back(CTrack(wstrDefaultname));
	return m_Tracks.size()-1;  // Index of the new track
};


CTrackList::iterator CTrackList::FindNearestTrack(const GeoPoint & gp, int& iDistance, int& indexNearestPoint)
{
	iDistance = INT_MAX;
	indexNearestPoint = -1;
	CTrackList::iterator itNearestTrack = end();
	std::list<CTrack>::iterator it;
	for (it = m_Tracks.begin(); it != m_Tracks.end(); ++it)
	{
		int d;
		int index = it->FindNearestPointIndex(gp, d);
		if (d < iDistance)
		{
			iDistance = d;
			itNearestTrack = it;
			indexNearestPoint = index;
		}
	}
	return itNearestTrack;
}

// ---------------------------------------------------------------

// -1 for the current track
CTrack& CAllTracks::GetTrack(Int iIndex)
{
	if (-1 == iIndex)
		return m_CurTrack;
	else
		return m_OldTracks.GetTrack(iIndex);
}

CTrack& CAllTracks::GetNearestTrack(const GeoPoint & gp, int& iIndexNearestPoint)
{
	int iDistMin;
	// Current track?
	iIndexNearestPoint = m_CurTrack.FindNearestPointIndex(gp, iDistMin);
	CTrack*	pResult = &m_CurTrack;
	// Or one of the old tracks?
	int iDistOld, iIndexOld;
	CTrackList::iterator itNearestOldTrack = m_OldTracks.FindNearestTrack(gp, iDistOld, iIndexOld);
	if (iDistOld < iDistMin)  // "<" and not "<=": important when there are no old tracks
	{
		iDistMin = iDistOld; 
		iIndexNearestPoint = iIndexOld;
		pResult = &(*itNearestOldTrack);
	}
	// Or the route?
	if (!m_CurRoute.IsEmpty())
	{
		int iDistRoute, iIndexRoute;
		iIndexRoute = m_CurRoute.AsTrack().FindNearestPointIndex(gp, iDistRoute);
		if (iDistRoute < iDistMin)
		{
			iDistMin = iDistRoute; 
			iIndexNearestPoint = iIndexRoute;
			pResult = &(m_CurRoute.AsTrack());
		}
	}
	return *pResult;
}

// save a route as track to disk
void CAllTracks::SaveRoute(const std::wstring& wstrFilename)
{
	DeleteFile(wstrFilename.c_str());
	Int iIndex = m_OldTracks.NewTrack(wstrFilename);
	m_OldTracks.Last().SetWritingWithFilename(wstrFilename);
	m_CurRoute.AsTrack().AppendToTrackOnlyCoord(m_OldTracks.Last());
	m_OldTracks.Last().SetWriting(false);
	m_CurRoute.MarkAsSaved();
}

void CAllTracks::PaintOldTracksWithCompetition(IPainter * pPainter, const GeoPoint & gp, unsigned long ulCompetitionTime)
{
	Int index = 0;
	for (CTrackList::iterator trit = GetOldTracks().begin(); trit != GetOldTracks().end(); ++trit)
	{
		trit->SetCompetition(gp, ulCompetitionTime);
		trit->PaintUnlocked(pPainter, CTrack::typeOldTrack);
		index++;
	}
	m_CurRoute.PaintRoute(pPainter);
}

void CAllTracks::PaintOldTracks(IPainter * pPainter)
{
	PaintOldTracksWithCompetition(pPainter, GeoPoint(), 0);
}

void CAllTracks::NewRoute()
{
	if (m_CurRoute.NeedsSaving())
	{
		if (MessageBox(NULL, L("Copy old route to track list?"), L("Route not saved"), MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
		{
			Int iIndex = m_OldTracks.NewTrack(L"Unsaved route");
			m_CurRoute.AsTrack().AppendToTrackOnlyCoord(m_OldTracks.Last());
		}
	}
	m_CurRoute.Reinit();
}

void CAllTracks::NewRouteFromTrackIndex(Int iIndex)
{
	if (-1 != iIndex)
		NewRouteFromTrack(m_OldTracks.GetTrack(iIndex));
}

void CAllTracks::NewRouteFromTrack(const CTrack& anOldTrack)
{
	NewRoute();
	m_CurRoute.AppendTrack(anOldTrack);
	m_CurRoute.MarkAsSaved();
}

// ---------------------------------------------------------------

CRoute::CRoute()
: m_pRouteAsTrack(new CTrack(L"Current route")),
  m_InsertMode(rimNone),
  m_bRouteHasChanges(false)
{
}

CRoute::~CRoute()
{
	delete m_pRouteAsTrack;
}

void CRoute::Reinit()
{
	delete m_pRouteAsTrack;
	m_pRouteAsTrack = new CTrack(L"Current route");
}

void CRoute::PaintRoute(IPainter * pPainter)
{
	m_pRouteAsTrack->PaintUnlocked(pPainter, (IsEditing())?CTrack::typeRouteInEdition:CTrack::typeRoute);
}

// Function called during routing to update the current position
// Returns distance to target and next point on the route.
void CRoute::UpdatePosition(const GeoPoint & gpCurrentPosition,
		                    double& ForwardDistance, GeoPoint& NextForwardPoint)
{
	Int indexForward = m_pRouteAsTrack->CalculateDistanceOnTrack(gpCurrentPosition,
		                                                         ForwardDistance, NextForwardPoint);
	if (!IsEditing())
	{
		m_pRouteAsTrack->SetStartCursor(indexForward);
		m_pRouteAsTrack->SetEndCursor(-1);
	}
}

void CRoute::UpdateNoPosition()
{
	if (!IsEditing())
	{
		m_pRouteAsTrack->SetStartCursor(-1);
		m_pRouteAsTrack->SetEndCursor(-1);
	}
}

void CRoute::UpdateAsNotFollowed()
{
	if (!IsEditing())
	{
		m_pRouteAsTrack->SetStartCursor(0);
		m_pRouteAsTrack->SetEndCursor(0);
	}
}

bool CRoute::NeedsSaving()
{
	return m_bRouteHasChanges && (GetPointCount() > 0);
}

void CRoute::MarkAsSaved()
{
	m_bRouteHasChanges = false;
}

void CRoute::AppendPoint(const GeoPoint& pt)
{
	m_bRouteHasChanges = true;
	m_pRouteAsTrack->AddPoint(pt, 0);
}

void CRoute::AppendTrack(const CTrack& aTrack)
{
	m_bRouteHasChanges = true;
	aTrack.AppendToTrackOnlyCoord(*m_pRouteAsTrack);
}

void CRoute::AddPointBeforeEndCursor(const GeoPoint& pt, bool bForwards)
{
	m_bRouteHasChanges = true;
	m_pRouteAsTrack->InsertPoint(m_pRouteAsTrack->GetEndCursor(), pt);
	if (bForwards)
	{
		m_pRouteAsTrack->SetEndCursor(m_pRouteAsTrack->GetEndCursor()+1);
		m_pRouteAsTrack->SetStartCursor(m_pRouteAsTrack->GetStartCursor()+1);
	}
}

void CRoute::InsertPointInNearestSegment(const GeoPoint& pt)
{
	m_bRouteHasChanges = true;
	int segIndex = m_pRouteAsTrack->FindNearestSegmentIndex(pt);
	m_pRouteAsTrack->InsertPoint(segIndex, pt);
}

void CRoute::ErasePoint(int iPointIndex)
{
	m_bRouteHasChanges = true;
	m_pRouteAsTrack->ErasePoint(iPointIndex);
}

int CRoute::GetPointCount() const
{
	return m_pRouteAsTrack->GetPointCount();
}

void CRoute::InsertPoint(const GeoPoint& pt)
{
	switch (m_InsertMode)
	{
	case rimNearestSegment:
		InsertPointInNearestSegment(pt);
		break;
	case rimBackwards:
		AddPointBeforeEndCursor(pt, false);
		break;
	case rimForwards:
		AddPointBeforeEndCursor(pt, true);
		break;
	}
}

void CRoute::SetInsertMode(enumRouteInsertMode mode, const GeoPoint& pt)
{
	m_InsertMode = mode;
	switch (m_InsertMode)
	{
	case rimNone:
	case rimNearestSegment:
		m_pRouteAsTrack->SetEndCursor(-1);
		m_pRouteAsTrack->SetStartCursor(-1);
		break;
	case rimBackwards:
	case rimForwards:
		int segIndex = m_pRouteAsTrack->FindNearestSegmentIndex(pt);
		m_pRouteAsTrack->SetEndCursor(segIndex);
		int previousIndex = (segIndex >=1)?segIndex-1:-1;
		m_pRouteAsTrack->SetStartCursor(previousIndex);
		break;
	}
};

void CRoute::Reverse()
{
	CTrack* pReversedTrack = new CTrack(L"Current route");
	m_pRouteAsTrack->ReverseAppendToTrackOnlyCoord(*pReversedTrack);
	delete m_pRouteAsTrack;
	m_pRouteAsTrack = pReversedTrack;
	m_bRouteHasChanges = true;
}

// ---------------------------------------------------------------

