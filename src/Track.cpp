#include "Track.h"
#include "MapApp.h"

extern int GetTrackStep();

void CTrack::AddPoint(GeoPoint pt, double time, double dHDOP)
{
	AutoLock l;
	// Calculate time for file
	double dTime;
	if (time != 0)
	{
		dTime = time;
	}
	else
	{
		SYSTEMTIME stTime;
		GetLocalTime(&stTime);
		SystemTimeToVariantTime(&stTime, &dTime);
	}

	m_dLastTime = dTime;
	unsigned long ulTime = (unsigned long)(dTime * 24 * 60 * 60);
	m_ulLastTime = ulTime;
	// Only if point is different
	if (m_Track.empty())
		m_Track.push_back(Segment());
	if (m_fBeginTrack && !m_Track.back().empty())
		m_Track.push_back(Segment());
	if (!m_Track.back().empty() && pt == m_Track.back().back().gp)
		return;
	if (!m_Track.back().empty() && (IntDistance(pt, m_Track.back().back().gp) < (GetTrackStep() /* * dHDOP */) ))
		return;
	if (!m_Track.back().empty() && (IntDistance(pt, m_Track.back().back().gp) > 100) && abs(dTime - m_dLastTime) > (3.0 / 24 / 60 / 60))
		Break();
	m_gpLastpoint = pt;
	m_fTrackPresent = true;
	// Add it to list
	m_Track.back().push_back(TrackPoint(pt, ulTime));
	++m_nPointCount;
	// If we are writing to a file
	if(m_fWriting)
	{
		// Output string to file
		m_iBufferPos += _snprintf(m_writeBuffer + m_iBufferPos, 1024, "%2.8f,%2.8f,%d,%.1f,%6.8f,,\r\n", Degree(pt.lat), Degree(pt.lon), m_fBeginTrack ? 1 : 0, (m_fAltitude ? m_dAltitude / cdFoot : -777),dTime);
		// Flush the file to save the point
		if (m_iBufferPos >= 4096)
		{
			Flush(4096);
			m_iBufferPos -= 4096;
			memcpy(m_writeBuffer, m_writeBuffer + 4096, m_iBufferPos);
		}
		// Since a point was added, the track should continue
	}
	m_fBeginTrack = false;
	// Если трек можно сжать
	if (m_fCompressable)
	{
		// Пока надо сжимать
		while (m_nPointCount > cnMaxPoints)
		{
			// Уберём пустые сегменты (хотя их не должно быть)
			while(m_Track.front().empty())
				m_Track.pop_front();
			// Перекинем одну точку
			m_CompressedTrack.back().push_back(
				m_Track.front().front());
			// Флаг, не надо ли начать следующий сегмент
			bool fNextSegment = false;
			// Убираем нужное количество точек
			for (int i = 0; i < cnCompressRatio; ++i)
			{
				// Убираем одну точку
				m_Track.front().pop_front();
				--m_nPointCount;
				// Убираем пустые сегменты (мог появиться один)
				while(m_Track.front().empty())
				{
					m_Track.pop_front();
					// И тогда в сжатом треке тоже надо начать новый сегмент
					fNextSegment = true;
				}
			}
			// Если надо начать, то начинаем. Есть гарантия, что предыдущий 
			// сегмент не пустой
			if (fNextSegment)
				m_CompressedTrack.push_back(Segment());
		}
	}
}

void CTrack::PaintUnlocked(IPainter * pPainter, unsigned int uiType)
{
	StartTimes::iterator itTime = m_startTimes.begin();
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
				if (itTime != m_startTimes.end() && *itTime + m_ulCompetitionTime <= it->time)
				{
					pPainter->PaintPoint(0x10003, it->gp, 0);
					++itTime;
				}
			}
			// And paint the polyline
			pPainter->FinishObject();
		}
	}
}
void CTrack::CreateFile()
{
	// Write file header
	m_iBufferPos = _snprintf(m_writeBuffer, 4096,
		"OziExplorer Track Point File Version 2.1\r\n"
		"WGS 84\r\n"
		"Altitude is in Feet\r\n"
		"Reserved\r\n"
		"0,2,%ld,,0,0,2,0\r\n"
		"1\r\n", m_iColor);
	// Track should begin
	m_fBeginTrack = true;
	m_fTrackPresent = false;
	m_wstrFilenameInt = L"";
}
//! Tell the track that it is broken (missing points)
void CTrack::Break()
{
	AutoLock l;
	// So it has to begin
	m_fBeginTrack = true;
}
void CTrack::Read(const wchar_t * wcFilename)
{
	AutoLock l;
	m_wstrFilenameExt = wcFilename;
	char buff[100];
	FILE * pFile = wfopen(wcFilename, L"rt");
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
		double dTime = 0;
		// _snprintf(m_writeBuffer + m_iBufferPos, 1024, "%2.8f,%2.8f,%d,%.1f,%6.8f,,\r\n", Degree(pt.lat), Degree(pt.lon), m_fBeginTrack ? 1 : 0, (m_fAltitude ? m_dAltitude / cdFoot : -777),dTime);
		if (listParts.size() >= 5)
			dTime = myatof(listParts[4].c_str());
		if (listParts.size() >= 2)
		{
			double dLatitude = myatof(listParts[0].c_str());
			double dLongitude = myatof(listParts[1].c_str());
			AddPoint(GeoPoint(FromDegree(dLongitude), FromDegree(dLatitude)), dTime);
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
		wchar_t wcFilename[1000];
		SYSTEMTIME st;
		GetLocalTime(&st);
		wsprintf(wcFilename, L"%s\\%04d.%02d.%02d-%02d.%02d.%02d.plt", app.m_rsTrackFolder().c_str(), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		m_wstrFilenameInt = wcFilename;
	}
	return m_wstrFilenameInt.c_str();
}

void CTrack::SetCompetition(const GeoPoint & gp, unsigned long ulCompetitionTime)
{
	bool fOld = (gp == m_gpCompetition && m_ulCompetitionTime <= ulCompetitionTime);
	m_ulCompetitionTime = ulCompetitionTime;
	if (fOld)
		return;
	m_startTimes.clear();
	m_gpCompetition = gp;
	m_ulCompetitionTime = ulCompetitionTime;
	bool isnear = false;
	int bestDistance;
	unsigned long bestTime;
	for (Track::iterator itSeg = m_Track.begin(); itSeg != m_Track.end(); ++itSeg)
	{
		if (!itSeg->empty())
		{
			for (Segment::iterator it = itSeg->begin(); it != itSeg->end(); ++it)
			{
				//if (it->time + 20 > m_ulLastTime)
				//	break;
				int distance = IntDistance(gp, it->gp);
				if (distance < 50)
				{
					if (isnear)
					{
						if (distance <= bestDistance)
						{
							bestDistance = distance;
							bestTime = it->time;
						}
					}
					else
					{
						isnear = true;
						bestDistance = distance;
						bestTime = it->time;
					}
				}
				else
				{
					if (isnear)
					{
						isnear = false;
						m_startTimes.push_back(bestTime);
					}
				}
			}
		}
	}
}
