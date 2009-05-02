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
#include "VersionNumber.h"
#include "FileFormats\GPX.h"

using namespace std;

//! Track representation, both active and loaded
class CTrack
{
	enum {cnMaxPoints = 600, cnCompressRatio = 10};
	enum enumTrackFormat
	{
		tfPLT = 0,
		tfGPX = 1
	};
	//! Type for list of track points
	struct TrackPoint
	{
		TrackPoint(const GeoPoint & gp_, unsigned long time_) : gp(gp_), timeUTC(time_){}
		GeoPoint gp;
		unsigned long timeUTC;
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
	bool m_fBeginFile;
	bool m_fTrackPresent;
	GeoPoint m_gpLastpoint;
	double m_dAltitude;
	bool m_fAltitude;
	Int m_iColor;
	bool m_fCompressable;
	//! Time of the last point
	double m_dLastTimeUTC;
	char m_writeBuffer[4096 + 1024];
	int m_iBufferPos;
	bool m_fWriting;
	int m_iId;
	GeoPoint m_gpCompetition;
	typedef std::list<unsigned long> StartTimes;
	StartTimes m_startTimesUTC;
	unsigned long m_ulCompetitionTime;
	enumTrackFormat m_CurrentTrackFormat;
	string m_strGPXName;
   fpos_t m_FilePosForAdding;

	const wchar_t * GetFileName();
	void CreateFile();
	void CreateFilePLT();
	void CreateFileGPX();
	std::string GetCreator();
	void FlushPLT(int iSize);
	void FlushGPX(int iSize);
	void WritePLT(GeoPoint pt, double dTimeUTC);
	void WriteGPX(GeoPoint pt, double dTimeUTC, double dHDOP);
public:

	//! Constructor
	CTrack() : 
		m_fTrackPresent(false), 
		m_iColor(128), 
		m_fAltitude(false), 
		m_fCompressable(false), 
		m_nPointCount(0),
		m_dLastTimeUTC(0),
		m_fWriting(false)
	{
		static int id = 0;
		m_iId = ++id;
	}
	void Init()
	{
		CreateFile();
	}
	//! Destructor
	~CTrack() 
	{
		// Close file if opened
		Flush(m_iBufferPos);
	}
	void SetColor(Int iColor) {m_iColor = iColor;}
	//! Add point to the end of track
	void AddPoint(GeoPoint pt, double timeUTC, double dHDOP = 1);
	//! Paint track to painter
	void PaintUnlocked(IPainter * pPainter, unsigned int uiType);
	//! Tell the track that it is broken (missing points)
	void Break();
	void Read(const std::wstring& wstrFilename);
	void ReadPLT(const std::wstring& wstrFilename);
	void ReadGPX(const std::auto_ptr<CGPXTrack>& apTrack, const std::wstring& wstrFilename);
	void ReadFirstTrackFromGPX(const std::wstring& wstrFilename);
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
		switch (m_CurrentTrackFormat)
		{
		case tfPLT:
			FlushPLT(iSize);
			break;
		case tfGPX:
			FlushGPX(iSize);
			break;
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

class CTrackList
{
protected:
	list<CTrack> m_Tracks;
	bool OpenTrackPLT(const std::wstring& wstrFile);
	bool OpenTracksGPX(const std::wstring& wstrFile);
public:
	typedef list<CTrack>::iterator iterator;
	iterator begin() { return m_Tracks.begin(); };
	iterator end() { return m_Tracks.end(); };
	CTrack& Last() { return m_Tracks.back(); };

	void GetTrackList(IListAcceptor * pAcceptor);
	bool OpenTracks(const std::wstring& wstrFile);
	void CloseTrack(Int iIndex);
};

#endif // TRACK_H
