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

#include "GMFileHolder.h"
#include "../GeoPoint.h"
#include <windows.h>
#ifndef UNDER_CE
#include <gdiplus.h>
#endif // UNDER_CE
#include <list>
#include "../VersionNumber.h"

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
		if (m_mapCachedFiles.size() > m_nMaxCacheSize) {
		}
	};

	void ProcessWMHIBERNATE();
	// Level - 0 .. 18
	// DC, rect - где рисовать
	int Paint(HDC dc, RECT& rect, const GeoPoint & gpCenter, double scale, enumGMapType type, bool fDoubleSize);
	bool RotationAllowed();

	void PostponeVersionUpdate();
	std::string GetRequest();
	void RequestProcessed(const std::string request, const char * data, int size);

	double GetPreferredScale(double scale);

	void DownloadAddCurrentView();
	void DownloadStartWithCurrentZoom();
	bool IsSelectingZoomToDownload() { return m_bGeoRectToDownload; };
	size_t GetDownloadQueueSize();

	long DownloadMapBy(enumGMapType type, CTrack &track, long nPixelRadius, long nDetailedLevel);
	void RelocateFiles(); 
	bool NeedRelocateFiles(); 
	long GetGMapCount() const { return m_GMFH.GetGMapCount(); };
	long GetWMSMapCount() const { return m_GMFH.GetWMSMapCount(); };
	std::wstring GetWMSMapName(long indexWMS) const { return m_GMFH.GetWMSMapName(indexWMS); };
    GeoPoint GetDemoPoint(enumGMapType type, double &scale) const { return m_GMFH.GetDemoPoint(type, scale); };

protected:
	int DrawSegment(HDC dc, RECT &srcrect, RECT &dstrect, GEOFILE_DATA& data);
	bool GetFileDataByPoint(GEOFILE_DATA *pData, const GeoPoint & gp, long level) const;
	long EnumerateAndProcessGeoRect(const GeoRect &gr, long nLevel, enumGMapType type, 
		long *pnInCacheCount, bool bJustCount);
private:
	// Открытые файлы
	std::map< GEOFILE_RASTERIZED, GEOFILE_CONTENTS > m_mapCachedFiles;
	std::list< GEOFILE_RASTERIZED > m_lstLastUsed;

	std::map< std::string, GEOFILE_DATA > m_mapRequestsSent;

	size_t m_nMaxCacheSize;

	CGMFileHolder m_GMFH;

	unsigned long m_nGDIPlusToken;

	// Флаг, что получили с сервера номера версий карты
	bool m_bGotMapVersions;

	// GeoRect, с которым последний раз рисовали карту
	GeoRect m_grectLastViewed;
	// GeoRect, который выбрал пользователь
	GeoRect m_grectToDownload;
	bool m_bGeoRectToDownload;
	// Level, с которым последний раз рисовали карту
	long m_nLevelToDownload;
	// Тип картинки, который выкачивать. Берём в момент, когда отмечают зум
	enumGMapType m_enTypeToDownload;
};
