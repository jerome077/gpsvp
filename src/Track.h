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
