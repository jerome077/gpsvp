#pragma once

#include "GMFileHolder.h"
#include "../GeoPoint.h"
#include <windows.h>
#ifndef UNDER_CE
#include <gdiplus.h>
#endif // UNDER_CE
#include <list>

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
	void SetMapFolder(const wchar_t * wcFolder);

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
