/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include <windows.h>
#include <sstream>
#include "GMPainter.h"
#include "ImageResize.h"
#ifdef UNDER_CE
#	include <aygshell.h>
// #include "STScreenBuffer.h"
#endif // UNDER_CE
#ifdef UNDER_CE
#include <initguid.h>
#endif // UNDER_CE
#include <math.h>
#include <set>
#include "../MapApp.h"



GEOFILE_DATA m_Missing;
bool m_fMissing = false;
std::wstring m_wstrMapFolder;

CGMPainter::CGMPainter(void)
{
	m_KeepMemoryLow = false;
	m_nMaxCacheSize = 256;

#ifdef USE_GDI_PLUS
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&m_nGDIPlusToken, &gdiplusStartupInput, NULL);
#else
#  if UNDER_CE && _WIN32_WCE < 0x500
#  else
	m_pImagingFactory = NULL;
#  endif
#endif // UNDER_CE
	m_bGotMapVersions = false;
	m_bGeoRectToDownload = false;
}

CGMPainter::~CGMPainter(void)
{
#ifdef USE_GDI_PLUS
	Gdiplus::GdiplusShutdown(m_nGDIPlusToken);
#endif // UNDER_CE
}

void CGMPainter::PostponeVersionUpdate()
{
	m_bGotMapVersions = false;
}

double CGMPainter::GetPreferredScale(double scale)
{
	double s = double(1 << (LEVEL_REVERSE_OFFSET)) / 2 / scale;
	if (s > 1) {
		unsigned long n = 1;
		while (n < s * 0.7) {
			n <<= 1;
		}
		s /= (double) n; // Here should be 1..2
	} else if (s < 1) {
		while (s < 0.7) {
			s *= 2;
		}
	}

	return scale * s;
}

bool CGMPainter::GetFileDataByPoint(GEOFILE_DATA *pData, const GeoPoint & gp, long level) const
{
	long nNumTiles = 1 << (level-1);
	pData->X = gp.lon / (1 << (GPWIDTH - level + 1)) + (nNumTiles >> 1);
	double dLat = Degree(gp.lat);
	long nBitmapOrigo = nNumTiles << 7; 
	double nPixelsPerLonRadian = ((double) (nNumTiles << 8)) / (2*pi);
	double z = sin(dLat / 180 * pi);
	long cntY = (long) floor(nBitmapOrigo - 0.5 * log((1+z)/(1-z)) * nPixelsPerLonRadian);
	pData->Y = cntY / 256;

	pData->level = (unsigned char) level;
	pData->type = gtNone;

	return true;
}

bool GetIntersectionRECT(RECT *pr, const RECT &r1, const RECT &r2)
{
	if (r2.left > r1.right || r2.right < r1.left || r2.top > r1.bottom || r2.bottom < r1.top)
		return false;
	// Intersect abscissa (longitude) intervals
	pr->left = (std::max)(r1.left, r2.left);
	pr->right = (std::min)(r1.right, r2.right);
	// Intersect ordinate (latitude) intervals
	pr->top = (std::max)(r1.top, r2.top);
	pr->bottom = (std::min)(r1.bottom, r2.bottom);
	return true;
}

int CGMPainter::Paint(HDC dc, const RECT& rect, const GeoPoint & gpCenter, double scale, enumGMapType type,
					  bool fDoubleSize, IPainter * pPainter)
{
	if (type == gtNone)
		return 0;
	double dLatCenter = Degree(gpCenter.lat);
	double dLonCenter = Degree(gpCenter.lon);
	// Paint image at zoom level 'level' centered at (dLonCenter, dLatCenter)

	int level = LEVEL_REVERSE_OFFSET;

	if (!app.m_Options[mcoDownloadGoogleMaps])
	{
		level = LEVEL_REVERSE_OFFSET - m_GMFH.GetMinLevel();
		if (level < 0)
			level = 0;
		if (level > LEVEL_REVERSE_OFFSET)
			level = LEVEL_REVERSE_OFFSET;
	}
	scale = double(1 << (LEVEL_REVERSE_OFFSET - level)) / 2 / scale;
	while (scale < (fDoubleSize ? 1.4 : 0.7))
	{
		--level;
		scale *= 2;
	}
	if (scale > 16)
		return 0;

	// Number of tiles along bitmap side at zoom level Level
	long nNumTiles = 1 << (level-1);
	// Coordinates of bitmap center (in pixels)
	long nBitmapOrigo = nNumTiles << 7; 

	double nPixelsPerLonDegree = ((double) (nNumTiles << 8)) / 360;
	double nPixelsPerLonRadian = ((double) (nNumTiles << 8)) / (2*pi);

	// Here we fill in the coordinates to download CurrentView
	m_nLevelToDownload = level;
	m_enTypeToDownload = type;
	double dHalfWidthDeg = rect.right - rect.left;
	dHalfWidthDeg /= nPixelsPerLonDegree * 2 * scale;
	double dHalfHeightDeg = (dHalfWidthDeg / (rect.right - rect.left)) * (rect.bottom - rect.top);
	m_grectLastViewed.Init  (GeoPoint(dLonCenter - dHalfWidthDeg, dLatCenter - dHalfHeightDeg));
	m_grectLastViewed.Append(GeoPoint(dLonCenter + dHalfWidthDeg, dLatCenter + dHalfHeightDeg));

	// Calculate X and Y of the image center (in pixels)
	long cntX = (long) floor(nBitmapOrigo + (dLonCenter * nPixelsPerLonDegree));
	double z = sin(dLatCenter / 180 * pi);
	long cntY = (long) floor(nBitmapOrigo - 0.5 * log((1+z)/(1-z)) * nPixelsPerLonRadian);
	m_nCenterXLastViewed = cntX;
	m_nCenterYLastViewed = cntY;
	m_nCenterZ17LastViewed = (unsigned char) (LEVEL_REVERSE_OFFSET - level);

	long X = cntX - int(double(rect.right - rect.left) / 2 / scale);
	long Y = cntY - int(double(rect.bottom - rect.top) / 2 / scale);

	long NumX = (std::max)(long(X / 256), long(0));
	long NumY = (std::max)(long(Y / 256), long(0));

	// We will collect non-painted tiles here
	std::map<long, GEOFILE_DATA> mapMissing;

	// Start painting
	long nHalfWidth = (rect.right - rect.left) / 2;
	long nHalfHeigth = (rect.bottom - rect.top) / 2;
	long nHorizCenter = (rect.right + rect.left) / 2;
	long nVertCenter = (rect.bottom + rect.top) / 2;
	for (long curX = NumX; ; curX++) {
		long xcoord = long((curX * 256 - cntX) * scale) + nHalfWidth;
		long xcoord_next = long(((curX+1) * 256 - cntX) * scale) + nHalfWidth;
		if (xcoord >= (rect.right - rect.left))
			break;
		for (long curY = NumY; ; curY++) {
			long ycoord = long((curY * 256 - cntY) * scale) + nHalfHeigth;
			long ycoord_next = long(((curY+1) * 256 - cntY) * scale) + nHalfHeigth;
			if (ycoord >= (rect.bottom - rect.top))
				break;

			RECT r_src;
			r_src.top = 0;
			r_src.left = 0;
			r_src.right = r_src.bottom = 256;

			GEOFILE_DATA data;
			bool bDoNotDraw = false;
			switch (type) {
				case gtNone:
				// case gtHybrid:
					bDoNotDraw = true;
					break;
				default:
					data = GEOFILE_DATA(type, (unsigned char) (LEVEL_REVERSE_OFFSET - level), curX, curY);
					break;
			}
			if (!bDoNotDraw) {
				RECT r(rect);
				r.top += ycoord;
				r.bottom = rect.top + ycoord_next;
				r.left += xcoord;
				r.right = rect.left + xcoord_next;

				if (DrawSegment(dc, r_src, r, data)) {
					RECT rect_is;
					if (GetIntersectionRECT(&rect_is, r, rect)) {
						//sqrt((float) (rect_is.right - rect_is.left) * (rect_is.bottom - rect_is.top));
						// Use the distance between the drawing area center and tile center to sort tiles
						long nHC = (r.right + r.left) / 2;
						long nVC = (r.bottom + r.top) / 2;
						long nH = nHorizCenter - nHC;
						long nV = nVertCenter  - nVC;
						long nDist = nH*nH + nV*nV;
						mapMissing[nDist] = data;
					}
				}
			}
		}
	}

	if (!mapMissing.empty()) {
		m_Missing = mapMissing.begin()->second;
	}

	// Draw selected aera (for download, ...)
	if (m_bGeoRectToDownload)
	{
		pPainter->StartPolyline(0xff, 0); // style like typeCurrentTrack
		pPainter->AddPoint(GeoPoint(m_grectToDownload.minLon, m_grectToDownload.minLat));
		pPainter->AddPoint(GeoPoint(m_grectToDownload.minLon, m_grectToDownload.maxLat));
		pPainter->AddPoint(GeoPoint(m_grectToDownload.maxLon, m_grectToDownload.maxLat));
		pPainter->AddPoint(GeoPoint(m_grectToDownload.maxLon, m_grectToDownload.minLat));
		pPainter->AddPoint(GeoPoint(m_grectToDownload.minLon, m_grectToDownload.minLat));
		pPainter->FinishObject();
	}

	return 0;
}

int CGMPainter::DrawSegment(HDC dc, const RECT &srcrect, const RECT &dstrect, GEOFILE_DATA& data)
{
	HBITMAP hbm = NULL;
	bool bHBITMAPInited = false;

	DWORD rop = SRCCOPY;
	if (app.m_Options[mcoInvertSatelliteImages] && (m_GMFH.GetRMS((enumGMapType) data.type)->IsSatellite())) {
		rop = SRCINVERT;
	}

	// Checking if it is in the cache
	GEOFILE_RASTERIZED gfr(data, dstrect.right - dstrect.left, dstrect.bottom - dstrect.top);
	long nNewBMSize = gfr.width * gfr.heigth * 4; // 4 bytes per pixel
	if (nNewBMSize > 2200*1024) {
		// Do not cache such large images, retain only the original
		gfr.width = gfr.heigth = 256;
		nNewBMSize = 256*256*4;
	}

	{ 
	AutoLock l;
	std::map< GEOFILE_RASTERIZED, GEOFILE_CONTENTS >::iterator it = m_mapCachedFiles.find(gfr);
	// If in cache
	if (it != m_mapCachedFiles.end()) {
		// Cool! Also move it to the head of the list of last used tiles
		hbm = it->second.h;

		std::list< GEOFILE_RASTERIZED >::iterator it2 = m_lstLastUsed.begin();
		while (it2 != m_lstLastUsed.end()) {
			if (*it2 == gfr) {
				m_lstLastUsed.erase(it2);
				break;
			}

			++it2;
		}
		m_lstLastUsed.push_back(gfr);

		bHBITMAPInited = true;
	} else {
		CDecoderTileInfo* pItemInfo = NULL;
		if (!m_GMFH.GetFileName(data, pItemInfo)) {
			m_Missing = data;
			m_fMissing = true;
			//// Simply fill the given rect with some color
			//COLORREF clr = RGB(255, 192, 192);
			//HBRUSH hBrush = CreateSolidBrush(clr);
			//if (hBrush) {
			//	FillRect(dc, &dstrect, hBrush);
			//	DeleteObject(hBrush);
			//}
			return 1;
		} else {
			hbm = pItemInfo->LoadTile(dc, this);

			if (hbm == NULL) {
				// It seems the image is broken... Delete the file, but only if internet & downloading are enabled & the storage type allows it
				if (app.m_Options[mcoDownloadGoogleMaps] && app.IsInternetAllowed())
					pItemInfo->DeleteTileIfPossible();
				bHBITMAPInited = false;
			} else {

				// Clean up the cache
				while (!m_lstLastUsed.empty()) {
					MEMORYSTATUS ms;
					ms.dwLength = sizeof(ms);
					GlobalMemoryStatus(&ms);

					if (((ms.dwAvailPhys - nNewBMSize) < 2*1024*1024) || (m_mapCachedFiles.size() > m_nMaxCacheSize))
						DeleteFrontElementFromCache();
					else
						break;
				}

				// Resize and put in the cache
				if ((gfr.width == gfr.heigth) && (gfr.width == 256)) {
				} else {
					HDC srcdc = CreateCompatibleDC(dc);
					HBITMAP srcoldbm = (HBITMAP) SelectObject(srcdc, hbm);
					HDC dstdc = CreateCompatibleDC(dc);
					//HBITMAP dstbm = CreateBitmap(gfr.width, gfr.heigth, 1, 32, NULL);
					//HBITMAP dstbm = CreateCompatibleBitmap(dc, gfr.width, gfr.heigth);

					BITMAPINFOHEADER bmih;
					void* pBits;
					bmih.biSize = sizeof(BITMAPINFOHEADER);
					bmih.biWidth = gfr.width;
					bmih.biHeight = gfr.heigth;
					bmih.biPlanes = 1;
					bmih.biBitCount = GetDeviceCaps(dc, BITSPIXEL);
					bmih.biCompression = BI_RGB;
					bmih.biSizeImage = 0;
					bmih.biXPelsPerMeter = 0;
					bmih.biYPelsPerMeter = 0;
					bmih.biClrUsed = 0;
					bmih.biClrImportant = 0;
					HBITMAP dstbm = CreateDIBSection(dc, (BITMAPINFO *)&bmih, 0, &pBits, NULL, 0);			
					HBITMAP dstoldbm = (HBITMAP) SelectObject(dstdc, dstbm);

					// We can use here any stretch algorithm, even a slow one
#ifdef UNDER_CE
#	if UNDER_CE >= 0x0500
					SetStretchBltMode(dstdc, COLORONCOLOR); // Not sure if it is better
					POINT pt;
					SetBrushOrgEx(dstdc, 0, 0, &pt);
#	endif // UNDER_CE >= 0x0500

#else // UNDER_CE
					SetStretchBltMode(dstdc, COLORONCOLOR); // Not sure if it is better
#endif // UNDER_CE

					StretchBlt(dstdc, 0, 0, gfr.width, gfr.heigth,
						srcdc, 0, 0, 256, 256, SRCCOPY);
#ifdef UNDER_CE
#	if UNDER_CE >= 0x0500
					SetBrushOrgEx(dc, pt.x, pt.y, NULL);
#	endif // UNDER_CE >= 0x0500
#endif // UNDER_CE
		
					SelectObject(srcdc, srcoldbm);
					DeleteDC(srcdc);
					DeleteObject(hbm);
					hbm = NULL;

					SelectObject(dstdc, dstoldbm);
					DeleteDC(dstdc);

					hbm = dstbm;
				}
				GEOFILE_CONTENTS c;
				c.h = hbm;
				m_mapCachedFiles[gfr] = c;

				m_lstLastUsed.push_back(gfr);

				bHBITMAPInited = true;
			}
		}
		if (pItemInfo) delete pItemInfo;
	}

	if (bHBITMAPInited) {
		long nWidth = dstrect.right - dstrect.left;
		long nHeight = dstrect.bottom - dstrect.top;

//		HBITMAP hResized = ScaleBitmapInt(dc, hbm, (WORD) nWidth, (WORD) nHeight);
		HDC srcdc = CreateCompatibleDC(dc);
//		SelectObject(srcdc, hResized);
		HBITMAP srcoldbm = (HBITMAP) SelectObject(srcdc, hbm);

		if ((nWidth == gfr.width) && (nHeight == gfr.heigth)) {
			BitBlt(dc, dstrect.left, dstrect.top, nWidth, nHeight,
				srcdc, srcrect.left, srcrect.top, rop);
		} else {

#ifdef UNDER_CE
#	if UNDER_CE >= 0x0500
			SetStretchBltMode(dc, BILINEAR);
			POINT pt;
			SetBrushOrgEx(dc, 0, 0, &pt);
#	endif // UNDER_CE >= 0x0500

#else // UNDER_CE
			SetStretchBltMode(dc, STRETCH_HALFTONE);
#endif // UNDER_CE

			StretchBlt(dc, dstrect.left, dstrect.top, nWidth, nHeight,
				srcdc, srcrect.left, srcrect.top, srcrect.right - srcrect.left, srcrect.bottom - srcrect.top, rop);
#ifdef UNDER_CE
#	if UNDER_CE >= 0x0500
			SetBrushOrgEx(dc, pt.x, pt.y, NULL);
#	endif // UNDER_CE >= 0x0500
#endif // UNDER_CE
		
		}
//		BitBlt(dc, dstrect.left, dstrect.top, nWidth, nHeight, srcdc, 0, 0, SRCCOPY); 
//		DeleteObject(hResized);

		SelectObject(srcdc, srcoldbm);
		DeleteDC(srcdc);

		// TODO: Check if this code works better 
		// http://www.codeguru.com/cpp/g-m/bitmap/specialeffects/article.php/c4897/
	}
	} // Locked section

	return 0;
}

void CGMPainter::SetMapFolder(const wchar_t * wcFolder, const CVersionNumber& gpsVPVersion)
{
	m_wstrMapFolder = wcFolder;
	m_GMFH.InitFromDir(wcFolder, gpsVPVersion, false);
}

bool CGMPainter::RotationAllowed()
{
	return m_GMFH.empty();
}

std::string CGMPainter::GetRequest()
{
	if (!app.m_Options[mcoDownloadGoogleMaps] || m_wstrMapFolder == L"")
		return "";

	GEOFILE_DATA data = m_Missing;
	if (!m_fMissing) {
		if (!m_GMFH.GetQueuedData(&data)) {
			return "";
		} else {
			
		}
	}

	if ((data.type == gtMap) || (data.type == gtSatellite) || (data.type == gtTopo)) { // gtHybrid
		if (!m_bGotMapVersions) {
			return "http://gpsvp.com/GoogleMaps.txt";
		}
	}

	m_fMissing = false;

	std::string s = m_GMFH.GetRequestURL(data);
	if (s.length() > 0) {
		m_mapRequestsSent[s] = data;
	}

	return s;
}

void CGMPainter::RequestProcessed(const std::string request, const char * data, int size)
{
	AutoLock l;
	if (request == "http://gpsvp.com/GoogleMaps.txt") {
		std::string s(data, size);
		// Remove "&hl=ru"
		char *p = "&hl=ru";
		size_t l = strlen(p);
		size_t found;
		while (true) {
			found = s.find(p);
			if (found != std::string::npos) {
				s.replace(found, l, "");
			} else {
				break;
			}
		}
		if (m_GMFH.ProcessPrefixes(s) == 0) {
			m_bGotMapVersions = true;
		}
	}

	std::map< std::string, GEOFILE_DATA >::iterator it = m_mapRequestsSent.find(request);
	if (it != m_mapRequestsSent.end()) {
		m_GMFH.OnRequestProcessed(request, it->second, data, size);
		DeleteElementFromCache(it->second);
		if (m_Missing == it->second)
			m_fMissing = false;
		m_mapRequestsSent.erase(it);
	} else {
		// We haven't requested it! =8-/
	}
}

void CGMPainter::DownloadAddCurrentView()
{
	m_grectToDownload = m_grectLastViewed;
	// Mark that we already have stored current GeoRect for download
	m_bGeoRectToDownload = true;
}

void CGMPainter::DownloadAddViewOfCurrentTileAtZoom(int Zoom00)
{
	long Z0 = 17 - m_nCenterZ17LastViewed;
	long cntX, cntY;
	if (Z0 <= Zoom00)
	{
		cntX = m_nCenterXLastViewed << (Zoom00 - Z0);
		cntY = m_nCenterYLastViewed << (Zoom00 - Z0);
	}
	else
	{
		cntX = m_nCenterXLastViewed >> (Z0 - Zoom00);
		cntY = m_nCenterYLastViewed >> (Z0 - Zoom00);
	}
	long NumX = (std::max)(long(cntX / 256), long(0));
	long NumY = (std::max)(long(cntY / 256), long(0));
	double CoordWest = GoogleXZ17toLong(NumX, LEVEL_REVERSE_OFFSET-Zoom00-1);
	double CoordNorth = GoogleYZ17toLat(NumY, LEVEL_REVERSE_OFFSET-Zoom00-1);
	// South and east can not be exactly on the south and east border of the tile
	// or else EnumerateAndProcessGeoRect would return the next tiles too.
	int gap = LEVEL_REVERSE_OFFSET-Zoom00;
	double CoordEast = GoogleXZ17toLong(((NumX+1)<<gap)-1, LEVEL_REVERSE_OFFSET-Zoom00-1-gap);
	double CoordSouth = GoogleYZ17toLat(((NumY+1)<<gap)-1, LEVEL_REVERSE_OFFSET-Zoom00-1-gap);
	m_grectToDownload.Init  (GeoPoint(CoordWest, CoordNorth));
	m_grectToDownload.Append(GeoPoint(CoordEast, CoordSouth));
	m_bGeoRectToDownload = true;
}

void CGMPainter::DownloadViewCorner(const int iCorner, const GeoPoint & gpCornerPoint)
{
	switch (iCorner)
	{
	case 1:
		m_grectToDownload.minLon = gpCornerPoint.lon;
		m_grectToDownload.maxLat = gpCornerPoint.lat;
		break;
	case 2:
		m_grectToDownload.maxLon = gpCornerPoint.lon;
		m_grectToDownload.minLat = gpCornerPoint.lat;
		break;
	}
	m_bGeoRectToDownload = true;
}

#ifndef UNDER_CE
void CGMPainter::DownloadViewFormat(const TSheetFormat iFormat, const GeoPoint & gpCenterPoint)
{
	switch (iFormat)
	{
	case sf_A4V_25000:
		m_grectToDownload.Init  (gpCenterPoint.ShiftedPointInMeter(-2625, 3712.5));
		m_grectToDownload.Append(gpCenterPoint.ShiftedPointInMeter(2625, -3712.5));
		break;
	case sf_A4H_25000:
		m_grectToDownload.Init  (gpCenterPoint.ShiftedPointInMeter(-3712.5, 2625));
		m_grectToDownload.Append(gpCenterPoint.ShiftedPointInMeter(3712.5, -2625));
		break;
	case sf_A4V_50000:
		m_grectToDownload.Init  (gpCenterPoint.ShiftedPointInMeter(-5250, 7425));
		m_grectToDownload.Append(gpCenterPoint.ShiftedPointInMeter(5250, -7425));
		break;
	case sf_A4H_50000:
		m_grectToDownload.Init  (gpCenterPoint.ShiftedPointInMeter(-7425, 5250));
		m_grectToDownload.Append(gpCenterPoint.ShiftedPointInMeter(7425, -5250));
		break;
	}
	m_bGeoRectToDownload = true;
}
#endif

void CGMPainter::DownloadClearView()
{
	m_bGeoRectToDownload = false;
}

void CGMPainter::DownloadStartWithCurrentZoom(bool withPreviousZooms)
{
	if (m_bGeoRectToDownload) {
		long nInCache;
		GeoDataSet files;
		long nCount = EnumerateAndProcessGeoRect(m_grectToDownload, m_nLevelToDownload, m_enTypeToDownload, withPreviousZooms,
			                                     &files, &nInCache, NULL);
		wchar_t buf[256];
		wsprintf(buf, L("%d new segments to download (%d in cache). Proceed?"), nCount, nInCache);
		if (IDYES == MessageBox(NULL, buf, L"gpsVP", MB_YESNO | MB_ICONQUESTION)) {
			m_GMFH.AddFileToDownload(files);
		} else {
		}
	}
}

void CGMPainter::GenerateTilesTrackForCurrentView(CTrackList& aTrackList)
{
	int trackIndex = aTrackList.NewTrack(L("Tiles for selected view at Z0=")+IntToText(GetLastZoom_00()));
	CTrack& track = aTrackList.GetTrack(trackIndex);
	GeoDataSet filesInCache;
	EnumerateAndProcessGeoRect(m_grectToDownload, m_nLevelToDownload, m_enTypeToDownload, false,
			                   NULL, NULL, &filesInCache);
	for(GeoDataSet::iterator it = filesInCache.begin(); it != filesInCache.end(); it++)
	{
		track.AddPoint(GeoPoint(GoogleXZ17toLong(it->X,   it->level), GoogleYZ17toLat(it->Y,   it->level)), 0);
		track.AddPoint(GeoPoint(GoogleXZ17toLong(it->X+1, it->level), GoogleYZ17toLat(it->Y,   it->level)), 0);
		track.AddPoint(GeoPoint(GoogleXZ17toLong(it->X+1, it->level), GoogleYZ17toLat(it->Y+1, it->level)), 0);
		track.AddPoint(GeoPoint(GoogleXZ17toLong(it->X,   it->level), GoogleYZ17toLat(it->Y+1, it->level)), 0);
		track.AddPoint(GeoPoint(GoogleXZ17toLong(it->X,   it->level), GoogleYZ17toLat(it->Y,   it->level)), 0);
		track.Break();
	}
}

void fputws_utf8(FILE* pFile, const std::wstring& wstrToWrite)
{
	int charcount = WideCharToMultiByte(CP_UTF8, 0, wstrToWrite.c_str(), wstrToWrite.length(), 0, 0, 0, 0);
    char* tempStr = new char[charcount+1];
	WideCharToMultiByte(CP_UTF8, 0, wstrToWrite.c_str(), wstrToWrite.length(), tempStr, charcount, 0, 0);
	tempStr[charcount] = '\0';
	fputs(tempStr, pFile);
    delete [] tempStr;
}

//bool IsUsingColorPalette(const std::wstring& strImageFullname, int zipIndex)
//{
//	#ifdef USE_GDI_PLUS
//		Gdiplus::Bitmap bm(strImageFullname.c_str());
//		Gdiplus::PixelFormat pf = bm.GetPixelFormat();
//		return (PixelFormatIndexed == (pf & PixelFormatIndexed));
//	#else // USE_GDI_PLUS
//		// May be wrong...
//		return (strImageFullname.substr(strImageFullname.length()-3) != L"jpg");
//	#endif // USE_GDI_PLUS
//}

std::wstring DblToTxtE(double value)
{
	wchar_t wcBuff[100] = {0};
	swprintf(wcBuff, 100, L"%e", value);
	return wcBuff;
}

std::wstring DblToTxtF(double value)
{
	wchar_t wcBuff[100] = {0};
	swprintf(wcBuff, 100, L"%f", value);
	return wcBuff;
}

void WriteVRTBand_Tile256Colors(FILE* pFile, GeoDataSet& filesInCache, long minX, long minY,
								CGMFileHolder& GMFH,
				                int iBand, const std::wstring& sColor,
								const std::wstring& wstrMapPath)
{
	fputws_utf8(pFile, std::wstring(L" <VRTRasterBand dataType=\"Byte\" band=\"")+IntToText(iBand)+L"\">\n");
	fputws_utf8(pFile, std::wstring(L"  <ColorInterp>")+sColor+L"</ColorInterp>\n");
	for(GeoDataSet::iterator it = filesInCache.begin(); it != filesInCache.end(); it++)
	{
		std::wstring name = GMFH.GetUnzippedFileName(*it);
		long xOff = 256*(it->X - minX);
		long yOff = 256*(it->Y - minY);
		fputws_utf8(pFile, L"  <ComplexSource>\n");
		if (wstrMapPath.empty())
		  fputws_utf8(pFile, std::wstring(L"   <SourceFilename relativeToVRT=\"0\">")+name+L"</SourceFilename>\n");
		else
		{
			name = name.substr(wstrMapPath.size());
			fputws_utf8(pFile, std::wstring(L"   <SourceFilename relativeToVRT=\"1\">")+name+L"</SourceFilename>\n");
		}
		fputws_utf8(pFile, L"   <SourceBand>1</SourceBand>\n");
		fputws_utf8(pFile, L"   <SourceProperties RasterXSize=\"256\" RasterYSize=\"256\" DataType=\"Byte\" BlockXSize=\"256\" BlockYSize=\"1\"/>\n");
		fputws_utf8(pFile, L"   <SrcRect xOff=\"0\" yOff=\"0\" xSize=\"256\" ySize=\"256\"/>\n");
		fputws_utf8(pFile, std::wstring(L"   <DstRect xOff=\"")+IntToText(xOff)+L"\" yOff=\""+IntToText(yOff)+L"\" xSize=\"256\" ySize=\"256\"/>\n");
		fputws_utf8(pFile, std::wstring(L"   <ColorTableComponent>")+IntToText(iBand)+L"</ColorTableComponent>\n");
		fputws_utf8(pFile, L"  </ComplexSource>\n");
	}
	fputws_utf8(pFile, L" </VRTRasterBand>\n");
}

void WriteVRTBand_Tile16MColors(FILE* pFile, GeoDataSet& filesInCache, long minX, long minY,
								CGMFileHolder& GMFH,
				                int iBand, const std::wstring& sColor,
								const std::wstring& wstrMapPath)
{
	fputws_utf8(pFile, std::wstring(L" <VRTRasterBand dataType=\"Byte\" band=\"")+IntToText(iBand)+L"\">\n");
	fputws_utf8(pFile, std::wstring(L"  <ColorInterp>")+sColor+L"</ColorInterp>\n");
	for(GeoDataSet::iterator it = filesInCache.begin(); it != filesInCache.end(); it++)
	{
		std::wstring name = GMFH.GetUnzippedFileName(*it);
		long xOff = 256*(it->X - minX);
		long yOff = 256*(it->Y - minY);
		fputws_utf8(pFile, L"  <SimpleSource>\n");
		if (wstrMapPath.empty())
		  fputws_utf8(pFile, std::wstring(L"   <SourceFilename relativeToVRT=\"0\">")+name+L"</SourceFilename>\n");
		else
		{
			name = name.substr(wstrMapPath.size());
			fputws_utf8(pFile, std::wstring(L"   <SourceFilename relativeToVRT=\"1\">")+name+L"</SourceFilename>\n");
		}
		fputws_utf8(pFile, std::wstring(L"   <SourceBand>")+IntToText(iBand)+L"</SourceBand>\n");
		fputws_utf8(pFile, L"   <SourceProperties RasterXSize=\"256\" RasterYSize=\"256\" DataType=\"Byte\" BlockXSize=\"256\" BlockYSize=\"1\"/>\n");
		fputws_utf8(pFile, L"   <SrcRect xOff=\"0\" yOff=\"0\" xSize=\"256\" ySize=\"256\"/>\n");
		fputws_utf8(pFile, std::wstring(L"   <DstRect xOff=\"")+IntToText(xOff)+L"\" yOff=\""+IntToText(yOff)+L"\" xSize=\"256\" ySize=\"256\"/>\n");
		fputws_utf8(pFile, L"  </SimpleSource>\n");
	}
	fputws_utf8(pFile, L" </VRTRasterBand>\n");
}

std::wstring ExtractPath(std::wstring fullName)
{
	std::replace(fullName.begin(), fullName.end(), L'/', L'\\');
	std::wstringstream wssPath(fullName);
	std::wstring strPath;
	std::wstring strCurrent, strPrevious;
	while (std::getline(wssPath, strCurrent, L'\\'))
	{
		strPath += strPrevious;
		strPrevious = strCurrent + L"\\";
	}
	return strPath;
}

std::wstring RemoveExtension(std::wstring fullName)
{
	std::replace(fullName.begin(), fullName.end(), L'/', L'\\');
	std::wstringstream wssPath(fullName);
	std::wstring strPath;
	std::wstring strCurrent, strPrevious;
	while (std::getline(wssPath, strCurrent, L'.'))
	{
		strPath += strPrevious;
		strPrevious = strCurrent;
	}
	return strPath;
}

#ifndef UNDER_CE
void CGMPainter::ExportCurrentZoom()
{
	GeoDataSet filesInCache;
	EnumerateAndProcessGeoRect(m_grectToDownload, m_nLevelToDownload, m_enTypeToDownload, false,
			                   NULL, NULL, &filesInCache);

	// Analysing the data (limits: tiles coordinates min and max, zoom level, color depth)
	long minX = LONG_MAX;
	long maxX = 0;
	long minY = LONG_MAX;
	long maxY = 0;
	long Z17 = -1;
	enumGMapType mapType;
	GeoDataSet::iterator it = filesInCache.begin();
	if (it != filesInCache.end())
	{
		Z17 = it->level;
		mapType = (enumGMapType)it->type;
		for( ; it != filesInCache.end(); it++)
		{
			if (minX > it->X) minX = it->X;
			if (minY > it->Y) minY = it->Y;
			if (maxX < it->X) maxX = it->X;
			if (maxY < it->Y) maxY = it->Y;
		}
	}
	else return; // Nothing to export

	// Size of the resulting image:
	long XSize = 256*(maxX-minX+1);
	long YSize = 256*(maxY-minY+1);

	// WGS84-coordinates of the resulting image:
	double CoordWest  = GoogleXZ17toLong(minX,   Z17);
	double CoordEast  = GoogleXZ17toLong(maxX+1, Z17);
	double CoordNorth = GoogleYZ17toLat (minY,   Z17);
	double CoordSouth = GoogleYZ17toLat (maxY+1, Z17);

	// Mercator-coordinates in meters:
	double XSpherMercWest  = LongToXSphericalMercator(CoordWest);
	double XSpherMercEast  = LongToXSphericalMercator(CoordEast);
	double YSpherMercNorth = LatToYSphericalMercator(CoordNorth);
	double YSpherMercSouth = LatToYSphericalMercator(CoordSouth);

	// Size of a single pixel in mercator-coordinates:
	double PixelSizeAlongX_SpherMerc = (XSpherMercEast-XSpherMercWest)/XSize;
	double PixelSizeAlongY_SpherMerc = -(YSpherMercNorth-YSpherMercSouth)/YSize;

	// Size of a single pixel in WGS84-coordinates:
	double PixelSizeAlongX_WGS84 = (CoordEast-CoordWest)/XSize;
	double PixelSizeAlongY_WGS84 = (CoordSouth-CoordNorth)/XSize;

	// Choosing a file name in the cache directory:
	std::wstring sMapName = m_GMFH.GetRMS(mapType)->GetFilePrefix();
	wchar_t wcExportSubFolder[MAX_PATH + 1] = {0};
	SYSTEMTIME st;
	GetLocalTime(&st);
	wsprintf(wcExportSubFolder, L"Export-%04d-%02d-%02d-%02d-%02d-%02d",
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	std::wstring wstrMapPath = m_wstrMapFolder + L"\\" + sMapName + L"\\";
	std::wstring wstrFolderPath = wstrMapPath + wcExportSubFolder + L"\\";
	std::wstring wcBaseFileName = L"Map";
    WMKDIR(wstrFolderPath.c_str());

	std::wstring wstrFilename;
	FILE* pFile;

	//// Generating a "worldfile" as WGS84
	//// ---------------------------------
	//// - Can be read by most GIS systems but do not define coordinates system.
	//// - GDAL can generate it from the .vrt file that why I don't generate
	////   one for EPSG:3857 but for EPSG:4326
	////
	//double CenterOfFirstPointCoordX = CoordWest + 0.5*PixelSizeAlongX_WGS84;
	//double CenterOfFirstPointCoordY = CoordNorth + 0.5*PixelSizeAlongY_WGS84;
	//wstrFilename = wstrFolderPath + wcBaseFileName + L"-EPSG4326.wld";
	//pFile = wfopen(wstrFilename.c_str(), L"wb");
	//fputws_utf8(pFile, DblToTxtE(PixelSizeAlongX_WGS84) + L"\n");
	//fputws_utf8(pFile, L"0.0\n");
	//fputws_utf8(pFile, L"0.0\n");
	//fputws_utf8(pFile, DblToTxtE(PixelSizeAlongY_WGS84) + L"\n");
	//fputws_utf8(pFile, DblToTxtE(CenterOfFirstPointCoordX) + L"\n");
	//fputws_utf8(pFile, DblToTxtE(CenterOfFirstPointCoordY) + L"\n");
	//fclose(pFile);

	//// Generating a .CAL
	//// -----------------
	//// - Calibration file for the commercial product TTQV (www.ttqv.com)
	//// - Can easilly be converted with Mapc2Mapc (www.the-thorns.org.uk/mapping)
	//double ScaleAera = (CoordEast - CoordWest) * (CoordNorth - CoordSouth) / (XSize * YSize);
	//wstrFilename = wstrFolderPath + wcBaseFileName + L"_png.cal";
	//pFile = wfopen(wstrFilename.c_str(), L"wb");
	//fputws_utf8(pFile, std::wstring(L"; Calibration File for QV Map\n"));
	//fputws_utf8(pFile, std::wstring(L"; generated by gpsVP\n"));
	//fputws_utf8(pFile, std::wstring(L"name = 10 = ")+wcBaseFileName+L".png\n");
	//fputws_utf8(pFile, std::wstring(L"fname = 10 = ")+wcBaseFileName+L".png\n");
	//fputws_utf8(pFile, std::wstring(L"nord = 6 = ")+DblToTxtF(CoordNorth)+L"\n");
	//fputws_utf8(pFile, std::wstring(L"sued = 6 = ")+DblToTxtF(CoordSouth)+L"\n");
	//fputws_utf8(pFile, std::wstring(L"ost = 6 = ")+DblToTxtF(CoordEast)+L"\n");
	//fputws_utf8(pFile, std::wstring(L"west = 6 = ")+DblToTxtF(CoordWest)+L"\n");
	//fputws_utf8(pFile, std::wstring(L"scale_area = 6 =  ")+DblToTxtF(ScaleAera)+L"\n");
	//fputws_utf8(pFile, std::wstring(L"proj_mode = 10 = proj\n"));
	//fputws_utf8(pFile, std::wstring(L"projparams = 10 = proj=merc\n"));
	//fputws_utf8(pFile, std::wstring(L"datum1 = 10 = WGS 84# 6378137# 298.257223563# 0# 0# 0#\n"));
	//fputws_utf8(pFile, std::wstring(L"c1_x = 7 =  0\n"));
	//fputws_utf8(pFile, std::wstring(L"c1_y = 7 =  0\n"));
	//fputws_utf8(pFile, std::wstring(L"c2_x = 7 =  ")+IntToText(XSize-1)+L"\n");
	//fputws_utf8(pFile, std::wstring(L"c2_y = 7 =  0\n"));
	//fputws_utf8(pFile, std::wstring(L"c3_x = 7 =  ")+IntToText(XSize-1)+L"\n");
	//fputws_utf8(pFile, std::wstring(L"c3_y = 7 =  ")+IntToText(YSize-1)+L"\n");
	//fputws_utf8(pFile, std::wstring(L"c4_x = 7 =  0\n"));
	//fputws_utf8(pFile, std::wstring(L"c4_y = 7 =  ")+IntToText(YSize-1)+L"\n");
	//fputws_utf8(pFile, std::wstring(L"c5_x = 7 =  0\n"));
	//fputws_utf8(pFile, std::wstring(L"c5_y = 7 =  0\n"));
	//fputws_utf8(pFile, std::wstring(L"c6_x = 7 =  0\n"));
	//fputws_utf8(pFile, std::wstring(L"c6_y = 7 =  0\n"));
	//fputws_utf8(pFile, std::wstring(L"c7_x = 7 =  0\n"));
	//fputws_utf8(pFile, std::wstring(L"c7_y = 7 =  0\n"));
	//fputws_utf8(pFile, std::wstring(L"c8_x = 7 =  0\n"));
	//fputws_utf8(pFile, std::wstring(L"c8_y = 7 =  0\n"));
	//fputws_utf8(pFile, std::wstring(L"c9_x = 7 =  0\n"));
	//fputws_utf8(pFile, std::wstring(L"c9_y = 7 =  0\n"));
	//fputws_utf8(pFile, std::wstring(L"c1_lat = 7 =  ")+DblToTxtF(CoordNorth)+L"\n");
	//fputws_utf8(pFile, std::wstring(L"c1_lon = 7 =  ")+DblToTxtF(CoordWest)+L"\n");
	//fputws_utf8(pFile, std::wstring(L"c2_lat = 7 =  ")+DblToTxtF(CoordNorth)+L"\n");
	//fputws_utf8(pFile, std::wstring(L"c2_lon = 7 =  ")+DblToTxtF(CoordEast)+L"\n");
	//fputws_utf8(pFile, std::wstring(L"c3_lat = 7 =  ")+DblToTxtF(CoordSouth)+L"\n");
	//fputws_utf8(pFile, std::wstring(L"c3_lon = 7 =  ")+DblToTxtF(CoordEast)+L"\n");
	//fputws_utf8(pFile, std::wstring(L"c4_lat = 7 =  ")+DblToTxtF(CoordSouth)+L"\n");
	//fputws_utf8(pFile, std::wstring(L"c4_lon = 7 =  ")+DblToTxtF(CoordWest)+L"\n");
	//fclose(pFile);

	// Generating .VRT files
	// ---------------------
    // - Virtual format for GDAL, see http://www.gdal.org/gdal_vrttut.html
    // - Can be used directly by GDAL oder converted to a real picture using:
	//    gdal_translate -of png -co worldfile=yes EXPORT.vrt EXPORT.png
	//   or
	//    gdal_translate -of GTiff EXPORT.vrt EXPORT.tif
	// - 2 files because the syntax depends on input pixel format (256 or 16 million colors).
	//   Output is always 16 millions or else I would have to calculate a new palette.
	wstrFilename = wstrFolderPath + wcBaseFileName + L"-256colors.vrt";
	pFile = wfopen(wstrFilename.c_str(), L"wb");
	fputws_utf8(pFile, std::wstring(L"<VRTDataset rasterXSize=\"")+IntToText(XSize)+L"\" rasterYSize=\""+IntToText(YSize)+L"\">\n");
	fputws_utf8(pFile, L" <SRS>EPSG:3857</SRS>\n");
	fputws_utf8(pFile, std::wstring(L" <GeoTransform> ")+DblToTxtE(XSpherMercWest)+L", "+DblToTxtE(PixelSizeAlongX_SpherMerc)+L", 0.0000000000000000e+000, "+DblToTxtE(YSpherMercNorth)+L", 0.0000000000000000e+000, "+DblToTxtE(PixelSizeAlongY_SpherMerc)+L"</GeoTransform>\n");
    WriteVRTBand_Tile256Colors(pFile, filesInCache, minX, minY, m_GMFH, 1, L"Red",   wstrMapPath);
    WriteVRTBand_Tile256Colors(pFile, filesInCache, minX, minY, m_GMFH, 2, L"Green", wstrMapPath);
    WriteVRTBand_Tile256Colors(pFile, filesInCache, minX, minY, m_GMFH, 3, L"Blue",  wstrMapPath);
	fputws_utf8(pFile, L"</VRTDataset>\n");
	fclose(pFile);

	wstrFilename = wstrFolderPath + wcBaseFileName + L"-16Mcolors.vrt";
	pFile = wfopen(wstrFilename.c_str(), L"wb");
	fputws_utf8(pFile, std::wstring(L"<VRTDataset rasterXSize=\"")+IntToText(XSize)+L"\" rasterYSize=\""+IntToText(YSize)+L"\">\n");
	fputws_utf8(pFile, L" <SRS>EPSG:3857</SRS>\n");
	fputws_utf8(pFile, std::wstring(L" <GeoTransform> ")+DblToTxtE(XSpherMercWest)+L", "+DblToTxtE(PixelSizeAlongX_SpherMerc)+L", 0.0000000000000000e+000, "+DblToTxtE(YSpherMercNorth)+L", 0.0000000000000000e+000, "+DblToTxtE(PixelSizeAlongY_SpherMerc)+L"</GeoTransform>\n");
    WriteVRTBand_Tile16MColors(pFile, filesInCache, minX, minY, m_GMFH, 1, L"Red",   wstrMapPath);
    WriteVRTBand_Tile16MColors(pFile, filesInCache, minX, minY, m_GMFH, 2, L"Green", wstrMapPath);
    WriteVRTBand_Tile16MColors(pFile, filesInCache, minX, minY, m_GMFH, 3, L"Blue",  wstrMapPath);
	fputws_utf8(pFile, L"</VRTDataset>\n");
	fclose(pFile);

	// Are some tiles in a .7z file?
	// -----------------------------
	wstrFilename = wstrFolderPath + L"copyfiles.bat";
	pFile = wfopen(wstrFilename.c_str(), L"ab");
	fputws_utf8(pFile, L"Rem Files that can be copied directly (without unzipping)\r\n");
	fclose(pFile);

	std::set<std::wstring> zipFilesSet;
	for(GeoDataSet::iterator it = filesInCache.begin(); it != filesInCache.end(); it++)
	{
		CDecoderTileInfo* pTileInfo;
		m_GMFH.GetFileName(*it, pTileInfo);
		if ((NULL != pTileInfo) && (pTileInfo->Is7z()))
		{
			std::wstring name = pTileInfo->Get7zFilename().substr(wstrMapPath.size()); // extracts relative name of the zip file
	 		std::wstring unzippedName = m_GMFH.GetUnzippedFileName(*it);
			unzippedName = unzippedName.substr(wstrMapPath.size()+ExtractPath(name).size()); // extracts relative name of the file

			std::wstring zipNameModified = name;
			std::replace(zipNameModified.begin(), zipNameModified.end(), L'/', L'_');
			std::replace(zipNameModified.begin(), zipNameModified.end(), L'\\', L'_');
			wstrFilename = wstrFolderPath + zipNameModified + L".txt";
			pFile = wfopen(wstrFilename.c_str(), L"ab");
			fputws_utf8(pFile, std::wstring(L"\"")+unzippedName+L"\"\r\n");
			fclose(pFile);

			zipFilesSet.insert(name);
		}
		else
		{
			wstrFilename = wstrFolderPath + L"copyfiles.bat";
			pFile = wfopen(wstrFilename.c_str(), L"ab");
	 		std::wstring unzippedName = m_GMFH.GetUnzippedFileName(*it);
			unzippedName = unzippedName.substr(wstrMapPath.size()); // extracts relative name of the file
			std::replace(unzippedName.begin(), unzippedName.end(), L'/', L'\\');
			fputws_utf8(pFile, std::wstring(L"xcopy \"..\\")+unzippedName+L"\" \""+ExtractPath(unzippedName)+L"*\" \r\n");
			fclose(pFile);
		}
		if (pTileInfo) delete pTileInfo;
	}

	// Generating an explaination/batch file
	// -------------------------------------
	wstrFilename = wstrFolderPath + wcBaseFileName + L".bat";
	pFile = wfopen(wstrFilename.c_str(), L"wb");
	fputws_utf8(pFile, L"@Echo off\r\n");
	fputws_utf8(pFile, L"REM Create the following batch file next to gpsvp.exe in order to:\r\n");
	fputws_utf8(pFile, L"REM  - Call the script setfw.bat, so that gdal_translate can be found (Something like: call C:\\Programm Files\\FWTools2.4.7\\setfw.bat)\r\n");
	fputws_utf8(pFile, L"REM  - Put the path to 7z.exe in the PATH if necessary\r\n");
	fputws_utf8(pFile, std::wstring(L"IF Exist \"")+app.m_wstrBasePath+L"InitExport.bat\" call \""+app.m_wstrBasePath+L"InitExport.bat\"\r\n");
	fputws_utf8(pFile, L"\r\n");
	fputws_utf8(pFile, L"Echo One of the .vrt file can be used directly by GDAL (http://www.gdal.org)\r\n");
	fputws_utf8(pFile, L"Echo On Windows GDAL is for example included in FWTools 2.4.7 (http://fwtools.maptools.org/)\r\n");
	fputws_utf8(pFile, L"Echo This script assemble convert the .vrt-file into a geotiff image\r\n");
	fputws_utf8(pFile, L"\r\n");
	if (zipFilesSet.size() > 0)
	{
//		fputws_utf8(pFile, std::wstring(L"Echo This map needs tiles from ")+IntToText(zipFilesSet.size())+L" .7z archive files.\r\n");
//		fputws_utf8(pFile, L"SET /P UserInput=Extract the .7z archives files? [y/n]\r\n");
//		fputws_utf8(pFile, L"IF not \"%UserInput%\"=\"y\" goto :END\r\n");
		for(std::set<std::wstring>::iterator it7z = zipFilesSet.begin(); it7z != zipFilesSet.end(); it7z++)
		{			
			std::wstring zipNameModified = *it7z;
			std::replace(zipNameModified.begin(), zipNameModified.end(), L'/', L'_');
			std::replace(zipNameModified.begin(), zipNameModified.end(), L'\\', L'_');
			fputws_utf8(pFile, std::wstring(L"7z.exe x ..\\")+*it7z+L" -o"+wstrFolderPath+ExtractPath(*it7z)+L" @"+zipNameModified+L".txt\r\n");
		}
	}
	fputws_utf8(pFile, L"call copyfiles.bat\r\n");
	fputws_utf8(pFile, L"\r\n");
	fputws_utf8(pFile, L"REM Trying both versions (256 and 16M colors), only one will works...\r\n");
	fputws_utf8(pFile, std::wstring(L"REM gdal_translate -of png -co worldfile=yes ")+ wcBaseFileName + L"-256colors.vrt " + wcBaseFileName+L"-256.png\r\n");
	fputws_utf8(pFile, std::wstring(L"gdal_translate -of GTiff ")+ wcBaseFileName + L"-256colors.vrt " + wcBaseFileName+L"-256.tif\r\n");
	fputws_utf8(pFile, std::wstring(L"REM gdal_translate -of png -co worldfile=yes ")+ wcBaseFileName + L"-16Mcolors.vrt " + wcBaseFileName+L"-16M.png\r\n");
	fputws_utf8(pFile, std::wstring(L"gdal_translate -of GTiff ")+ wcBaseFileName + L"-16Mcolors.vrt " + wcBaseFileName+L"-16M.tif\r\n");
	fputws_utf8(pFile, L"\r\n");
//	if (zipFilesSet.size() > 0)
//	{
//		fputws_utf8(pFile, L"Echo Removing extracted tiles...\r\n");
//		for(std::set<std::wstring>::iterator it7z = zipFilesSet.begin(); it7z != zipFilesSet.end(); it7z++)
//		{			
//			fputws_utf8(pFile, std::wstring(L"rd /s ")+RemoveExtension(*it7z)+L"\r\n");
//		}
//	}
	fputws_utf8(pFile, L":END\r\n");
	fputws_utf8(pFile, L"Exit /b 0\r\n");
	fclose(pFile);

	// Displaying a few Infos about the export
	std::wstring sInfo = std::wstring(L("Exported to map folder as ")) + wcBaseFileName + L".*";
	MessageBox(0, sInfo.c_str(), L("Export done"), MB_ICONINFORMATION);
}
#endif

void CGMPainter::RefreshTiles(const GeoRect *pRegion)
{
	GeoDataSet files;
	size_t nCount = m_GMFH.ListFilesInsideRegion(&files, m_enTypeToDownload, pRegion);
	wchar_t buf[256];
	wsprintf(buf, L("%d segments to refresh. Proceed?"), nCount);
	if (IDYES == MessageBox(NULL, buf, L"gpsVP", MB_YESNO | MB_ICONQUESTION)) {
		m_GMFH.AddFileToDownload(files);
	} else {
	}
}

void CGMPainter::RefreshAll()
{
	RefreshTiles(NULL);
}

void CGMPainter::RefreshInsideRegion()
{
	RefreshTiles(&m_grectLastViewed);
}

// return value = number of tiles to download
// pSetNotInCache = when not NULL receives the list of tiles to download
// pnInCacheCount = when not NULL receives the number of tiles already in cache
// pSetInCache = when not NULL receives the list of tiles already in cache
long CGMPainter::EnumerateAndProcessGeoRect(const GeoRect &gr, long nLevel, enumGMapType type, bool bDownloadLowerLevels,
	                                        GeoDataSet *pSetNotInCache, long *pnInCacheCount, GeoDataSet *pSetInCache)
{
	GEOFILE_DATA topleft, bottomright;
	GetFileDataByPoint(&topleft, GeoPoint(gr.minLon, gr.minLat), nLevel);
	GetFileDataByPoint(&bottomright, GeoPoint(gr.maxLon, gr.maxLat), nLevel);

	RECT r;
	r.top = topleft.Y; r.bottom = bottomright.Y;
	r.left = topleft.X; r.right = bottomright.X;

	long nNotInCache = 0;
	long nInCache = 0;
	long nMinLevel = (bDownloadLowerLevels) ? 1 : nLevel - 1;
	GEOFILE_DATA data;
	data.type = type;
	for (int level = nLevel; level > nMinLevel; level--) {
		data.level = (unsigned char)  (LEVEL_REVERSE_OFFSET - level);
		for (long x = r.left; x <= r.right; x++) {
			data.X = x;
			for (long y = r.bottom; y <= r.top; y++) {
				data.Y = y;
				if (m_GMFH.IsFileInCache(data)) {
					nInCache++;
					if (pSetInCache)
						pSetInCache->insert(data);
				} else {
					nNotInCache++;
					if (pSetNotInCache)
						pSetNotInCache->insert(data);
				}
			}
		}
		// Proceed with the less detailed zoom level
		r.left /= 2; r.right /= 2; r.top /= 2; r.bottom /= 2;
	}
	if (pnInCacheCount)
		*pnInCacheCount = nInCache;
	return nNotInCache;
}

long CGMPainter::DownloadMapBy(enumGMapType type, CTrack &track, long nPixelRadius, long nDetailedLevel)
{
	return 1;
}

void CGMPainter::ProcessWMHIBERNATE()
{
	AutoLock l;

	// Kill all contents of the image cache 
	m_lstLastUsed.erase(m_lstLastUsed.begin(), m_lstLastUsed.end());

	std::map< GEOFILE_RASTERIZED, GEOFILE_CONTENTS >::iterator it = m_mapCachedFiles.begin();
	while (it != m_mapCachedFiles.end()) {
		DeleteObject(it->second.h);
		++it;
	}
	m_mapCachedFiles.erase(m_mapCachedFiles.begin(), m_mapCachedFiles.end());
}

size_t CGMPainter::GetDownloadQueueSize()
{
	return m_GMFH.GetDownloadQueueSize();
}

void CGMPainter::RelocateFiles()
{
	m_GMFH.RelocateFiles(0); 
}
bool CGMPainter::NeedRelocateFiles()
{
	return m_GMFH.NeedRelocateFiles();
}

void CGMPainter::SetKeepMemoryLow(bool value)
{
	if (m_KeepMemoryLow != value)
	{
		m_KeepMemoryLow = value;
		if (m_KeepMemoryLow)
			SetMaxCacheSize(16);
		else
			SetMaxCacheSize(256);
	}
}

void CGMPainter::DeleteFrontElementFromCache()
{
	AutoLock l;

	GEOFILE_RASTERIZED g(m_lstLastUsed.front());
	m_lstLastUsed.pop_front();

	std::map< GEOFILE_RASTERIZED, GEOFILE_CONTENTS >::iterator it = m_mapCachedFiles.find(g);
	if (it != m_mapCachedFiles.end())
	{
		DeleteObject(it->second.h);
		m_mapCachedFiles.erase(it);
	}
}

void CGMPainter::DeleteElementFromCache(const GEOFILE_DATA &data)
{
	AutoLock l;

	std::list< GEOFILE_RASTERIZED > lstToDelete;
	std::list< GEOFILE_RASTERIZED >::iterator it = m_lstLastUsed.begin();
	while (it != m_lstLastUsed.end()) {
		if (it->data == data) {
			std::map< GEOFILE_RASTERIZED, GEOFILE_CONTENTS >::iterator it2 = m_mapCachedFiles.find(*it);
			if (it2 != m_mapCachedFiles.end())
			{
				DeleteObject(it2->second.h);
				m_mapCachedFiles.erase(it2);
			}
			std::list< GEOFILE_RASTERIZED >::iterator it3 = it;
			++it;
			m_lstLastUsed.erase(it3);
		} else {
			++it;
		}
	}
}

void CGMPainter::InitImagingFactoryOnce()
{
	#ifndef USE_GDI_PLUS
	#  if UNDER_CE && _WIN32_WCE < 0x500
	#  else
	if (!m_pImagingFactory)
	{
		#  if defined(BARECE) || defined(UNDER_WINE)
		#  else
		app.InitComIfNecessary();
		HRESULT hr=CoCreateInstance(CLSID_ImagingFactory, 
							NULL, 
							CLSCTX_INPROC_SERVER, 
							IID_IImagingFactory, 
							(void**)&m_pImagingFactory);
		#  endif
	}
	#  endif
	#endif
}

HBITMAP CGMPainter::BufferToHBitmap(char *buffer, size_t len, HDC dc)
{
	HBITMAP hbm = NULL;

	#if UNDER_CE && _WIN32_WCE < 0x500
	  // not possible under WM2003
	#else
		#ifndef USE_GDI_PLUS
		#  if defined(BARECE) || defined(UNDER_WINE)
		#  else
			InitImagingFactoryOnce();
			IImage* pImage = NULL;
			m_pImagingFactory->CreateImageFromBuffer(buffer, len,
													BufferDisposalFlagNone,
													&pImage );
			ImageInfo imageInfo;
			pImage->GetImageInfo(&imageInfo);

			HDC dstdc = CreateCompatibleDC(dc);
			BITMAPINFOHEADER bmih;
			void* pBits;
			bmih.biSize = sizeof(BITMAPINFOHEADER);
			bmih.biWidth = imageInfo.Width;
			bmih.biHeight = imageInfo.Height;
			bmih.biPlanes = 1;
			bmih.biBitCount = GetDeviceCaps(dc, BITSPIXEL);
			bmih.biCompression = BI_RGB;
			bmih.biSizeImage = 0;
			bmih.biXPelsPerMeter = 0;
			bmih.biYPelsPerMeter = 0;
			bmih.biClrUsed = 0;
			bmih.biClrImportant = 0;
			HBITMAP dstbm = CreateDIBSection(dstdc, (BITMAPINFO *)&bmih, 0, &pBits, NULL, 0);			
			HBITMAP dstoldbm = (HBITMAP) SelectObject(dstdc, dstbm);
			RECT rectForDraw;
			rectForDraw.top = 0;
			rectForDraw.left = 0;
			rectForDraw.right = imageInfo.Width;
			rectForDraw.bottom = imageInfo.Height;
			pImage->Draw(dstdc, &rectForDraw, NULL);
			SelectObject(dstdc, dstoldbm);
			DeleteDC(dstdc);
			hbm = dstbm;
		#  endif
		#else // UNDER_CE
			IStream* pIStream    = NULL;
			if (S_OK == CreateStreamOnHGlobal(buffer, FALSE, (LPSTREAM*)&pIStream))
			{
				Gdiplus::Bitmap bm(pIStream);
				Gdiplus::Color clr;
				bm.GetHBITMAP(clr, &hbm);
				pIStream->Release();
			}
		#endif // UNDER_CE

	#endif

	return hbm;
}
