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
	m_Track.back().push_back(TrackPoint(pt, ulTimeUTC));
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
	// Output string to file
	m_iBufferPos += _snprintf(m_writeBuffer + m_iBufferPos, 1024, "%2.8f,%2.8f,%d,%.1f,%6.8f,,\r\n",
		                      Degree(pt.lat), Degree(pt.lon), m_fBeginTrack ? 1 : 0,
							  (m_fAltitude ? m_dAltitude / cdFoot : -777), dLocalTime);
	// Since a point was added, the track should continue
	m_fBeginFile = false;
}

void CTrack::WriteGPX(GeoPoint pt, double dTimeUTC, double dHDOP)
{
	// Output string to file
	if (m_fBeginTrack && !m_fBeginFile)
		m_iBufferPos += _snprintf(m_writeBuffer + m_iBufferPos, 1024, "</trkseg><trkseg>\r\n");
	std::string strEle;
	if (m_fAltitude)
		strEle = "<ele>"+DoubleToStr(m_dAltitude)+"</ele>";
	SYSTEMTIME st;
	VariantTimeToSystemTime(dTimeUTC, &st);
	m_iBufferPos += _snprintf(m_writeBuffer + m_iBufferPos, 1024,
		                "<trkpt lat=\"%2.8f\" lon=\"%2.8f\">%s<time>%04d-%02d-%02dT%02d:%02d:%02dZ</time></trkpt>\r\n",
						Degree(pt.lat), Degree(pt.lon),
						strEle.c_str(),
						st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	// Since a point was added, the track should continue
	m_fBeginFile = false;
}

void CTrack::PaintUnlocked(IPainter * pPainter, unsigned int uiType)
{
	StartTimes::iterator itTimeUTC = m_startTimesUTC.begin();
	for (Track::iterator itSeg = m_Track.begin(); itSeg != m_Track.end(); ++itSeg)
	{
		if (!itSeg->empty())
		{
			// Now we use polylines with type 0xff
			pPainter->StartPolyline(uiType /*(m_wstrFilename.empty() ? 0xfc : 0xff)*/, 0);
			// Add all the points
			for (Segment::iterator it = itSeg->begin(); it != itSeg->end(); ++it)
			{
				pPainter->AddPoint(it->gp);
				if (itTimeUTC != m_startTimesUTC.end() && *itTimeUTC + m_ulCompetitionTime <= it->timeUTC)
				{
					pPainter->PaintPoint(0x10003, it->gp, 0);
					++itTimeUTC;
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
		"0,2,%ld,,0,0,2,0\r\n"
		"1\r\n", m_iColor);
}

std::string CTrack::GetCreator()
{
	return app.GetGpsVPVersion().AsStringWithName();
}

void CTrack::CreateFileGPX()
{
	m_CurrentTrackFormat = tfGPX;
	m_FilePosForAdding = 0;
	// Write file header
	GetFileName(); // initialisiert m_strGPXName
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
	static const DWORD iCloseLength = strlen(sCloseXml);
	if (m_fWriting && m_fTrackPresent)
	{
		HANDLE hFile = ::CreateFile(GetFileName(), (GENERIC_READ | GENERIC_WRITE),
			                        FILE_SHARE_READ, NULL, 
							        OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE != hFile)
		{
			DWORD fileSize = GetFileSize(hFile, NULL);
			if (fileSize >= iCloseLength)
				SetFilePointer(hFile, -iCloseLength, 0, FILE_END);
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
	if (0 == _wcsnicmp(wstrExt.c_str(), L".gpx", 4))
		ReadFirstTrackFromGPX(wstrFilename);
	else
		ReadPLT(wstrFilename);
}

void CTrack::ReadGPX(const std::auto_ptr<CGPXTrack>& apTrack, const std::wstring& wstrFilename)
{
	m_wstrFilenameExt = apTrack->getName() + L" - " + wstrFilename; 
	std::auto_ptr<CGPXTrackSeg> apTrackSeg = apTrack->firstTrackSeg();
	while (!apTrackSeg->eof())
	{
		std::auto_ptr<CGPXTrackPoint> apTrackPoint = apTrackSeg->firstTrackPoint();
		while (!apTrackPoint->eof())
		{
			AddPoint(GeoPoint(FromDegree(apTrackPoint->getLongitude()),
							  FromDegree(apTrackPoint->getLatitude())
							  ), apTrackPoint->getUTCTime());
			apTrackPoint = apTrackSeg->nextTrackPoint();
		}
		apTrackSeg = apTrack->nextTrackSeg();
		if (!apTrackSeg->eof())
			Break();
	}
}

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
	catch (_com_error e)
	{
		MessageBox(NULL, (std::wstring(L("Error while reading track: "))+e.ErrorMessage()).c_str(),
			       L("GPX read error"), MB_ICONEXCLAMATION);
	}
}

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
		if (listParts.size() >= 2)
		{
			double dLatitude = myatof(listParts[0].c_str());
			double dLongitude = myatof(listParts[1].c_str());
			AddPoint(GeoPoint(FromDegree(dLongitude), FromDegree(dLatitude)), dTimeUTC);
		}
	}
}
const wstring CTrack::GetExtFilename()
{
	AutoLock l;
	return m_wstrFilenameExt;
}
bool CTrack::IsPresent()
{
	AutoLock l;
	return m_fTrackPresent;
}
GeoPoint CTrack::GetLastPoint()
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


void CTrackList::GetTrackList(IListAcceptor * pAcceptor)
{
	int iIndex = 0;
	for (list<CTrack>::iterator it = m_Tracks.begin(); it != m_Tracks.end();++it, ++iIndex)
		pAcceptor->AddItem(it->GetExtFilename().c_str(), iIndex);
}

bool CTrackList::OpenTracks(const std::wstring& wstrFile)
{
	std::wstring wstrExt = wstrFile.substr(wstrFile.length()-4, 4);
	if (0 == _wcsnicmp(wstrExt.c_str(), L".gpx", 4))
		return OpenTracksGPX(wstrFile);
	else
		return OpenTrackPLT(wstrFile);
}

bool CTrackList::OpenTrackPLT(const std::wstring& wstrFile)
{
	m_Tracks.push_back(CTrack());
	m_Tracks.back().ReadPLT(wstrFile);
	if (m_Tracks.back().IsPresent())
	{
		return true;
	}
	else
	{
		m_Tracks.pop_back();
		return false;
	}
}

bool CTrackList::OpenTracksGPX(const std::wstring& wstrFile)
{
	bool Result = false;
	try
	{
		ComInit MyObjectToInitCOM;
		{
			CGPXFileReader GpxReader(wstrFile);
			GpxReader.setReadTime(!app.m_Options[mcoQuickReadGPXTrack]);
			std::auto_ptr<CGPXTrack> apTrack = GpxReader.firstTrack();
			if (apTrack->eof())
				MessageBox(NULL, L("No track in this file"), L("GPX read error"), MB_ICONEXCLAMATION);
			while (!apTrack->eof())
			{
				m_Tracks.push_back(CTrack());
				m_Tracks.back().ReadGPX(apTrack, wstrFile);
				if (m_Tracks.back().IsPresent())
				{
					Result = true;
				}
				else
					m_Tracks.pop_back();
				apTrack = GpxReader.nextTrack();
			}
		}
	}
	catch (CGPXFileReader::Error e)
	{
		MessageBox(NULL, (L("Error while reading track: ")+e()).c_str(), L("GPX read error"), MB_ICONEXCLAMATION);
	}
	catch (_com_error e)
	{
		MessageBox(NULL, (std::wstring(L("Error while reading track: "))+e.ErrorMessage()).c_str(),
			       L("GPX read error"), MB_ICONEXCLAMATION);
	}
	return Result;
}

void CTrackList::CloseTrack(Int iIndex)
{
	list<CTrack>::iterator it;
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
