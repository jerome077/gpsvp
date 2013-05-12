/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#pragma once

#include "GMCommon.h"
#include "GMFileHolder.h"
#include "../GeoPoint.h"
#include <windows.h>

#ifdef USE_GDI_PLUS
#	include <gdiplus.h>
#endif // USE_GDI_PLUS
#ifdef UNDER_CE
#  if UNDER_CE && _WIN32_WCE < 0x500
#  else
#    include <imaging.h> 
#  endif // _WIN32_WCE
#endif // UNDER_CE

#include <list>
#include "../VersionNumber.h"
#include "../Track.h"

struct GEOFILE_CONTENTS {
	HBITMAP h;
};

struct GEOFILE_RASTERIZED {
	GEOFILE_DATA data;
	long width;
	long heigth;

	GEOFILE_RASTERIZED(const GEOFILE_DATA &newdata, const long newwidth, const long newheigth) :
		data(newdata), width(newwidth), heigth(newheigth) {};

	bool operator< (const GEOFILE_RASTERIZED &other) const
	{
		if (data < other.data)
			return true;
		if (other.data < data)
			return false;
		if (width < other.width)
			return true;
		if (width > other.width)
			return false;
		if (heigth < other.heigth)
			return true;
		return false;
	}

	bool operator== (const GEOFILE_RASTERIZED &other) const
	{
		return ((data == other.data) && (width == other.width) && (heigth == other.heigth));
	}
};

class CGMPainter
{
public:
	CGMPainter(void);
	virtual ~CGMPainter(void);
	void SetMapFolder(const wchar_t * wcFolder, const CVersionNumber& gpsVPVersion);

	void SetMaxCacheSize(long nBitmapsCount) 
	{
		m_nMaxCacheSize = nBitmapsCount;
		for(size_t i=m_mapCachedFiles.size(); i>m_nMaxCacheSize; i--)
			DeleteFrontElementFromCache();
	};

	void ProcessWMHIBERNATE();
	// Level - 0 .. 18
	// DC, rect - where to draw
	int Paint(HDC dc, const RECT& rect, const GeoPoint & gpCenter, double scale, enumGMapType type,
		      bool fDoubleSize, IPainter * pPainter);
	bool RotationAllowed();

	void PostponeVersionUpdate();
	std::string GetRequest();
	void RequestProcessed(const std::string request, const char * data, int size);

	double GetPreferredScale(double scale);

	void DownloadAddCurrentView();
	void DownloadAddViewOfCurrentTileAtZoom(int Zoom00);
	void DownloadViewCorner(const int iCorner, const GeoPoint & gpCornerPoint);
	#ifndef UNDER_CE
    void DownloadViewFormat(const TSheetFormat iFormat, const GeoPoint & gpCenterPoint);
	void ExportCurrentZoom();
	#endif
	void DownloadClearView();
	void DownloadStartWithCurrentZoom(bool withPreviousZooms);
    void GenerateTilesTrackForCurrentView(CTrackList& aTrackList);
	void RefreshTiles(const GeoRect *pRegion);
	void RefreshInsideRegion();
	void RefreshAll();
	void SetOldTileDays(int n) { m_GMFH.SetOldTileDays(n); }
	bool IsSelectingZoomToDownload() { return m_bGeoRectToDownload; };
	size_t GetDownloadQueueSize();
	long DownloadMapBy(enumGMapType type, CTrack &track, long nPixelRadius, long nDetailedLevel);
	void RelocateFiles(); 
	bool NeedRelocateFiles(); 
	long GetGMapCount() const { return m_GMFH.GetGMapCount(); };
	long GetUserMapCount() const { return m_GMFH.GetUserMapCount(); };
	std::wstring GetUserMapName(long indexUserMap) const { return m_GMFH.GetUserMapName(indexUserMap); };
    GeoPoint GetDemoPoint(enumGMapType type, double &scale) const { return m_GMFH.GetDemoPoint(type, scale); };
    bool HasDemoPoint(enumGMapType type) const
	{
		double m_dummyScale;
		return (GeoPoint(0, 0) != GetDemoPoint(type, m_dummyScale));
	};
	void SetKeepMemoryLow(bool value);
	int GetLastZoom_00() const { return m_nLevelToDownload-1; };
	int GetLastZoom_01() const { return m_nLevelToDownload; };
	int GetLastZoom_17() const { return LEVEL_REVERSE_OFFSET - m_nLevelToDownload; };
	long GetLastTileX() const { return m_nCenterXLastViewed/256; };
	long GetLastTileY() const { return m_nCenterYLastViewed/256; };

protected:
	int DrawSegment(HDC dc, const RECT &srcrect, const RECT &dstrect, GEOFILE_DATA& data);
	bool GetFileDataByPoint(GEOFILE_DATA *pData, const GeoPoint & gp, long level) const;
	long EnumerateAndProcessGeoRect(const GeoRect &gr, long nLevel, enumGMapType type, bool bDownloadLowerLevels,
	                                GeoDataSet *pSetNotInCache, long *pnInCacheCount, GeoDataSet *pSetInCache);
	void DeleteFrontElementFromCache();
	void DeleteElementFromCache(const GEOFILE_DATA &data);
	HBITMAP LoadTileOrZippedTile(const std::wstring& strFullname, int zipIndex, HDC dc);
	void InitImagingFactoryOnce();

private:
	// Opened files
	std::map< GEOFILE_RASTERIZED, GEOFILE_CONTENTS > m_mapCachedFiles;
	std::list< GEOFILE_RASTERIZED > m_lstLastUsed;

	std::map< std::string, GEOFILE_DATA > m_mapRequestsSent;

	size_t m_nMaxCacheSize;

	CGMFileHolder m_GMFH;

	unsigned long m_nGDIPlusToken;

	// If we have obtained map version numbers from the server
	bool m_bGotMapVersions;

	// GeoRect that was Paint()ed last
	GeoRect m_grectLastViewed;
	long m_nCenterXLastViewed;
	long m_nCenterYLastViewed;
	unsigned char m_nCenterZ17LastViewed;
	// GeoRect user chose to download
	GeoRect m_grectToDownload;
	bool m_bGeoRectToDownload;
	// Zoom level that was Paint()ed last
	long m_nLevelToDownload;
	// Map type to download. Берём в момент, когда отмечают зум [??? Fixed at the moment when user ?marks? zoom]
	enumGMapType m_enTypeToDownload;

	bool m_KeepMemoryLow;

	#ifndef USE_GDI_PLUS
	#  if UNDER_CE && _WIN32_WCE < 0x500
	#  else
	IImagingFactory* m_pImagingFactory;
	#  endif
	#endif
};
