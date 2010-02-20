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

#include <ole2.h>
#include "Common.h"
#include <list>
#include <vector>
#include <set>
#include "GeoPoint.h"
#include "IPainter.h"
#include "Lock.h"
#include "VersionNumber.h"
#ifndef UNDER_WINE
#	include "FileFormats/GPX.h"
#endif // UNDER_WINE

const int NO_iALTITUDE = -777;

//! Track representation, both active and loaded
class CTrack
{
	//! Track type like defined in Normal.vpc:
public:
	typedef unsigned int track_t;
	static const track_t typeCurrentTrack	= 0xff;
	static const track_t typeOldTrack		= 0xfc;
	static const track_t typeRoute		    = 0xf9;
	static const track_t typeRouteInEdition = 0xf8;
	static const track_t typeCurrentTrack_NotSelected	= 0xfa;
	static const track_t typeOldTrack_NotSelected		= 0xfb;
	static const track_t typeRoute_NotSelected			= 0xf7;
protected:
	//
	enum {cnMaxPoints = 600, cnCompressRatio = 10};
	enum enumTrackFormat
	{
		tfPLT = 0,
		tfGPX = 1
	};
	//! Type for list of track points
	struct TrackPoint
	{
		TrackPoint(const GeoPoint & gp_, unsigned long time_, int altitude_) : gp(gp_),
			timeUTC(time_), altitude(altitude_) {};
		GeoPoint gp;
		unsigned long timeUTC;
		int altitude;  // or NO_iALTITUDE when no altitude
	};
	typedef std::list<TrackPoint> Segment;
	typedef std::list<Segment> Track;
	//! List of track points
	Track m_Track;
	Track m_CompressedTrack;
	int m_nPointCount;
	//! Name for file
	std::wstring m_wstrFilenameInt;
	std::wstring m_wstrFilenameExt;
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
	std::string m_strGPXName;
	fpos_t m_FilePosForAdding;
	// ! For analysis or edition:
	int m_StartCursor; // Index of the first selected point beginning with 0. -1 to deactivate.
	int m_EndCursor;   // Index of the last selected point beginning with 0. -1 to deactivate.

	const wchar_t * GetFileName();
	void CreateFile();
	void CreateFile(enumTrackFormat riTrackFormat);
	void CreateFilePLT();
	void CreateFileGPX();
	std::string GetCreator();
	void FlushPLT(int iSize);
	void FlushGPX(int iSize);
	void WritePLT(GeoPoint pt, double dTimeUTC);
	void WriteGPX(GeoPoint pt, double dTimeUTC, double dHDOP);
	bool IsSelectedPart(int iPointIndex);
	void CalcLengthsUnlocked(int index1, int index2, double smoothing,
						     double& dFullLength, double& dPartLength,
							 double& dFullGapLength, double& dPartGapLength,
							 int& iSegmentCount) const;
	void CalcEleDiffUnlocked(int index1, int index2, int smoothing,
							 int& iFullEleDiffUp, int& iPartEleDiffUp,
							 int& iFullEleDiffDown, int& iPartEleDiffDown,
							 int& iFullGapEleDiffUp, int& iPartGapEleDiffUp,
						     int& iFullGapEleDiffDown, int& iPartGapEleDiffDown,
							 int& iSegmentCount) const;
	void CalcTimesUnlocked(int index1, int index2,
						   unsigned long& ulStartTimeUTC, unsigned long& ulEndTimeUTC,
						   unsigned long& ulIndex1TimeUTC, unsigned long& ulIndex2TimeUTC) const;
	TrackPoint& GetTrackPoint(int iPointIndex);
public:

	//! Constructor
	CTrack() : 
		m_fTrackPresent(false), 
		m_iColor(128), 
		m_fAltitude(false), 
		m_fCompressable(false), 
		m_nPointCount(0),
		m_dLastTimeUTC(0),
		m_fWriting(false),
		m_StartCursor(-1),
		m_EndCursor(-1),
		m_CurrentTrackFormat(tfPLT)
	{
		static int id = 0;
		m_iId = ++id;
	}
	CTrack(const std::wstring& wstrDefaultname) : 
		m_fTrackPresent(false), 
		m_iColor(128), 
		m_fAltitude(false), 
		m_fCompressable(false), 
		m_nPointCount(0),
		m_dLastTimeUTC(0),
		m_fWriting(false),
		m_StartCursor(-1),
		m_EndCursor(-1),
		m_CurrentTrackFormat(tfPLT)
	{
		static int id = 0;
		m_iId = ++id;
		m_wstrFilenameExt = wstrDefaultname;
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
	virtual void PaintUnlocked(IPainter * pPainter, track_t uiType);
	//! Tell the track that it is broken (missing points)
	void Break();
	void Read(const std::wstring& wstrFilename);
	void ReadPLT(const std::wstring& wstrFilename);
#ifndef UNDER_WINE
	void ReadGPX(const std::auto_ptr<CGPXTrack>& apTrack, const std::wstring& wstrTrackname);
#endif // UNDER_WINE
	void ReadFirstTrackFromGPX(const std::wstring& wstrFilename);
	const std::wstring GetExtFilename() const;
	bool IsPresent() const;
	GeoPoint GetLastPoint() const;
	void SetAltitude(double dAltitude);
	void ResetAltitude();
	void SetCompressable();
	bool IsWriting()
	{
		return m_fWriting;
	}
	void SetWriting(bool fWriting);
	void SetWritingWithFilename(const std::wstring& wstrNewFilename);
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
	int FindNearestPointIndex(const GeoPoint & gp, int& iDistance);
	int FindNearestSegmentIndex(const GeoPoint & gp, GeoPoint& NextPoint);
	int FindNearestSegmentIndex(const GeoPoint & gp) { GeoPoint dummy; return FindNearestSegmentIndex(gp, dummy); };
	void SetStartCursor(int iPointIndex) { m_StartCursor = iPointIndex; };
	void SetEndCursor(int iPointIndex)   { m_EndCursor = iPointIndex; };
	int GetStartCursor() { return m_StartCursor; };
	int GetEndCursor()   { return m_EndCursor; };
	int GetInfoCount() const;
	std::wstring GetInfo(int infoIndex) const;
	double GetFullLength() const;
	int InsertPoint(int iNextPointIndex, const GeoPoint & gp);
	void ErasePoint(int iPointIndex);
	GeoPoint GetPoint(int iPointIndex);
	int GetPointCount() const { return m_nPointCount; };
	// Copy the points of this track at the end of another track
	void AppendToTrackOnlyCoord(CTrack & aTrack) const;
	void ReverseAppendToTrackOnlyCoord(CTrack & aTrack) const;
	Int CalculateDistanceOnTrack(const GeoPoint & gp, double& ForwardDistance, GeoPoint& NextForwardPoint);
};

// Simple list of CTrack:
class CTrackList
{
protected:
	std::list<CTrack> m_Tracks;
	Int OpenTrackPLT(const std::wstring& wstrFile);
	Int OpenTracksGPX(const std::wstring& wstrFile);
public:
	CTrackList();
	typedef std::list<CTrack>::iterator iterator;
	iterator begin() { return m_Tracks.begin(); };
	iterator end() { return m_Tracks.end(); };
	CTrack& Last() { return m_Tracks.back(); };
	CTrack& GetTrack(Int iIndex);

	void GetTrackList(IListAcceptor * pAcceptor);
	Int OpenTracks(const std::wstring& wstrFile); // returns track index or -1
	void CloseTrack(Int iIndex);
	Int FindTrackIndex(const CTrack& aTrack);
	Int NewTrack(const std::wstring& wstrDefaultname);
	iterator FindNearestTrack(const GeoPoint & gp, int& iDistance, int& indexNearestPoint);
};

enum enumRouteInsertMode
{
	rimNone = 0,  // rimNone => Editing is off
	rimNearestSegment = 1,
	rimBackwards = 2,
	rimForwards = 3
};

// CRoute: 
//  Allows to follow a track and also to edit it.
class CRoute
{
protected:
	// The route is stored as a track:
	CTrack* m_pRouteAsTrack;
	// The selected edition mode:
	enumRouteInsertMode m_InsertMode;
	// Flag for modifications:
	bool m_bRouteHasChanges;
	// Where the route will be saved:
	std::wstring m_wstrRouteFilename;
	// To preview where a point could be inserted:
	GeoPoint m_gpPreviewInsertionPoint;
	// To undo the last insertion (-1 when unknown)
	int m_IndexLastNewPoint;
public:
	CRoute();
	~CRoute();
	void Reinit();
	bool IsEmpty() const { return (0 == GetPointCount()); };
	void PaintRoute(IPainter * pPainter);
	void PaintInsertionPoint(IPainter * pPainter, const GeoPoint & gpNewPoint);
	CTrack & AsTrack() { return *m_pRouteAsTrack; };
	int GetPointCount() const;

	// ** Route following **
	void UpdatePosition(const GeoPoint & gpCurrentPosition,
		                double& ForwardDistance, GeoPoint& NextForwardPoint);
	void UpdateNoPosition();
	void UpdateAsNotFollowed();

	// ** Route editing **
	bool NeedsSaving();
	void MarkAsSaved();
	// Insert a point depending on the current edition mode
	void InsertPoint(const GeoPoint& pt);
	enumRouteInsertMode GetInsertMode() { return m_InsertMode; };
	void SetInsertMode(enumRouteInsertMode mode, const GeoPoint& pt = GeoPoint());
	bool IsEditing() { return (rimNone != m_InsertMode); };
	bool SetPreviewInsertionPoint(const GeoPoint & gp); // returns true when redraw needed
	bool CanUndo();
	void Undo();

	// Add a point at the end:
	void AppendPoint(const GeoPoint& pt);
	// Append a whole track at the end of the route:
	void AppendTrack(const CTrack& aTrack);
	// Add a point before the end cursor (or at the end if the end cursor is -1):
	void AddPointBeforeEndCursor(const GeoPoint& pt, bool bForwards);
	// Add a point in the middle of the nearest segment:
	void InsertPointInNearestSegment(const GeoPoint& pt);
	// Erase a point:
	void ErasePoint(int iPointIndex);
	// Reverse the route:
	void Reverse();
};


// List of CTrack for the old tracks + the current track:
// NB: The current track has -1 as index.
class CAllTracks
{
protected:
	CTrackList m_OldTracks;
	CTrack m_CurTrack;
	CRoute m_CurRoute;
public:
	CTrackList& GetOldTracks() { return m_OldTracks; };
	CTrack& GetCurTrack() { return m_CurTrack; };

	void StartNewTrack() { m_CurTrack.StartNewTrack(); };
	Int OpenTracks(const std::wstring& wstrFile) { return m_OldTracks.OpenTracks(wstrFile); };
	void CloseTrack(Int iIndex) { if (-1 < iIndex) m_OldTracks.CloseTrack(iIndex); }; // Only for old tracks

	CTrack& GetTrack(Int iIndex);
	CTrack& Last() { return m_OldTracks.Last(); };
	CTrack& GetNearestTrack(const GeoPoint & gp, int& iIndexNearestPoint);
	void PaintOldTracks(IPainter * pPainter);
	void PaintOldTracksWithCompetition(IPainter * pPainter, const GeoPoint & gp, unsigned long ulCompetitionTime);

	// Route:
	CRoute& GetCurRoute() { return m_CurRoute; };
	bool NewRoute();                                                            // returns false if canceled
	bool NewRouteFromTrackIndex(Int iIndex, bool bMarkAsSaved = true);          // returns false if canceled
	bool NewRouteFromTrack(const CTrack& anOldTrack, bool bMarkAsSaved = true); // returns false if canceled
	void SaveRoute(const std::wstring& wstrFilename, bool bKeepTrackOpen, bool bMarkAsSaved = true);
	bool IsEditingRoute() { return m_CurRoute.IsEditing(); };
};

#endif // TRACK_H
