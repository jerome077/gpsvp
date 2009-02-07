/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef TRACK_H
#define TRACK_H

#include <Ole2.h>
#include "Common.h"
#include <list>
#include <vector>
#include <set>
#include "GeoPoint.h"
#include "IPainter.h"
#include "Lock.h"

using namespace std;

//! Track representation, both active and loaded
class CTrack
{
	enum {cnMaxPoints = 600, cnCompressRatio = 10};
	//! Type for list of track points
	struct TrackPoint
	{
		TrackPoint(const GeoPoint & gp_, unsigned long time_) : gp(gp_), time(time_){}
		GeoPoint gp;
		unsigned long time;
	};
	typedef list<TrackPoint> Segment;
	typedef list<Segment> Track;
	//! List of track points
	Track m_Track;
	Track m_CompressedTrack;
	int m_nPointCount;
	//! Name for file
	wstring m_wstrFilenameInt;
	wstring m_wstrFilenameExt;
	//! The track will begin with next point
	bool m_fBeginTrack;
	bool m_fTrackPresent;
	GeoPoint m_gpLastpoint;
	double m_dAltitude;
	bool m_fAltitude;
	Int m_iColor;
	bool m_fCompressable;
	//! Время предыдущей точки
	double m_dLastTime;
	unsigned long m_ulLastTime;
	char m_writeBuffer[4096 + 1024];
	int m_iBufferPos;
	bool m_fWriting;
	int m_iId;
	GeoPoint m_gpCompetition;
	typedef std::list<unsigned long> StartTimes;
	StartTimes m_startTimes;
	unsigned long m_ulCompetitionTime;

	const wchar_t * GetFileName();
	void CreateFile();
public:
	//! Constructor
	CTrack() : 
		m_fTrackPresent(false), 
		m_iColor(128), 
		m_fAltitude(false), 
		m_fCompressable(false), 
		m_nPointCount(0),
		m_dLastTime(0),
		m_fWriting(false)
	{
		CreateFile();
		static int id = 0;
		m_iId = ++id;
	}
	//! Destructor
	~CTrack() 
	{
		// Close file if opened
		Flush(m_iBufferPos);
	}
	void SetColor(Int iColor) {m_iColor = iColor;}
	//! Add point to the end of track
	void AddPoint(GeoPoint pt, double time, double dHDOP = 1);
	//! Paint track to painter
	void PaintUnlocked(IPainter * pPainter, unsigned int uiType);
	//! Tell the track that it is broken (missing points)
	void Break();
	void Read(const wchar_t * wcFilename);
	const wstring GetExtFilename();
	bool IsPresent();
	GeoPoint GetLastPoint();
	void SetAltitude(double dAltitude);
	void ResetAltitude();
	void SetCompressable();
	bool IsWriting()
	{
		return m_fWriting;
	}
	void SetWriting(bool fWriting)
	{
		if (m_fWriting != fWriting)
		{
			Flush(m_iBufferPos);
			CreateFile();
			m_fWriting = fWriting;
		}
	}
	void Flush(int iSize)
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
	void StartNewTrack()
	{
		Flush(m_iBufferPos);
		CreateFile();
	}
	int GetId()
	{
		return m_iId;
	}
	void SetCompetition(const GeoPoint & gp, unsigned long ulCompetitionTime);
};

#endif // TRACK_H
