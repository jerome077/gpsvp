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
#include "GMPainter.h"
#include "ImageResize.h"
#ifdef UNDER_CE
#	include <aygshell.h>
// #include "STScreenBuffer.h"
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

#ifndef UNDER_CE
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&m_nGDIPlusToken, &gdiplusStartupInput, NULL);
#endif // UNDER_CE
	m_bGotMapVersions = false;
	m_bGeoRectToDownload = false;
}

CGMPainter::~CGMPainter(void)
{
#ifndef UNDER_CE
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
	pData->X = gp.lon / (1 << (24 - level + 1)) + (nNumTiles >> 1);
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
	// Найдём пересечение по горизонтали
	pr->left = max(r1.left, r2.left);
	pr->right = min(r1.right, r2.right);
	pr->top = max(r1.top, r2.top);
	pr->bottom = min(r1.bottom, r2.bottom);
	return true;
}

int CGMPainter::Paint(HDC dc, RECT& rect, const GeoPoint & gpCenter, double scale, enumGMapType type, bool fDoubleSize)
{
	if (type == gtNone)
		return 0;
	double dLatCenter = Degree(gpCenter.lat);
	double dLonCenter = Degree(gpCenter.lon);
	// Рисуем картинку уровня level с центром в dLonCenter/dLatCenter

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

	// количество блоков вдоль стороны Битмапа уровня Level
	long nNumTiles = 1 << (level-1);
	// координаты (в пикселах) середины Битмапа уровня Level
	long nBitmapOrigo = nNumTiles << 7; 

	double nPixelsPerLonDegree = ((double) (nNumTiles << 8)) / 360;
	double nPixelsPerLonRadian = ((double) (nNumTiles << 8)) / (2*pi);

	// Здесь заполняем координаты для скачивания CurrentView
	m_nLevelToDownload = level;
	m_enTypeToDownload = type;
	double dHalfWidthDeg = rect.right - rect.left;
	dHalfWidthDeg /= nPixelsPerLonDegree * 2 * scale;
	double dHalfHeightDeg = (dHalfWidthDeg / (rect.right - rect.left)) * (rect.bottom - rect.top);
	m_grectLastViewed.Init  (GeoPoint(dLonCenter - dHalfWidthDeg, dLatCenter - dHalfHeightDeg));
	m_grectLastViewed.Append(GeoPoint(dLonCenter + dHalfWidthDeg, dLatCenter + dHalfHeightDeg));

	// Вычисляем X и Y (в пикселах) для центра картинки
	long cntX = (long) floor(nBitmapOrigo + (dLonCenter * nPixelsPerLonDegree));
	double z = sin(dLatCenter / 180 * pi);
	long cntY = (long) floor(nBitmapOrigo - 0.5 * log((1+z)/(1-z)) * nPixelsPerLonRadian);

	long X = cntX - int(double(rect.right - rect.left) / 2 / scale);
	long Y = cntY - int(double(rect.bottom - rect.top) / 2 / scale);

	long NumX = max(long(X / 256), long(0));
	long NumY = max(long(Y / 256), long(0));

	// Здесь будем складывать неотрисованные квадраты
	std::map<long, GEOFILE_DATA> mapMissing;

	// Начинаем рисовать
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
				case gtHybrid:
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
						// Use a distance between the drawing area center and a tile center to sort tiles
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

	return 0;
}

int CGMPainter::DrawSegment(HDC dc, RECT &srcrect, RECT &dstrect, GEOFILE_DATA& data)
{
	HBITMAP hbm = NULL;
	bool bHBITMAPInited = false;

	DWORD rop = SRCCOPY;
	if (app.m_Options[mcoInvertSatelliteImages] && (m_GMFH.GetRMS((enumGMapType) data.type)->IsSatellite())) {
		rop = SRCINVERT;
	}

	// Смотрим, есть ли в кэше [tr: Look if it is in tha cache?]
	GEOFILE_RASTERIZED gfr(data, dstrect.right - dstrect.left, dstrect.bottom - dstrect.top);
	long nNewBMSize = gfr.width * gfr.heigth * 4; // 4 байта на пиксел [tr: 4 bytes per pixel]
	if (nNewBMSize > 2200*1024) {
		// Такие большие картинки не кешируем, будем хранить только оригинал
		// [tr: Such large pictures ???, will retain only the original?]
		gfr.width = gfr.heigth = 256;
		nNewBMSize = 256*256*4;
	}

	std::map< GEOFILE_RASTERIZED, GEOFILE_CONTENTS >::iterator it = m_mapCachedFiles.find(gfr);
	if (it != m_mapCachedFiles.end()) {
		// Круть! Заодно ещё перетащить в голову списка использованных
		// [tr: Cool! ??????]
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
		std::wstring w;
		long nRes = m_GMFH.GetFileName(w, data);
		if (nRes) {
			m_Missing = data;
			m_fMissing = true;
			//// Просто закрасить чем-нибудь переданный rect
			//COLORREF clr = RGB(255, 192, 192);
			//HBRUSH hBrush = CreateSolidBrush(clr);
			//if (hBrush) {
			//	FillRect(dc, &dstrect, hBrush);
			//	DeleteObject(hBrush);
			//}
			return 1;
		} else {
#ifdef UNDER_CE
#  ifdef BARECE
#  else
			hbm = SHLoadImageFile(w.c_str());
#  endif
#else // UNDER_CE
			Gdiplus::Bitmap bm(w.c_str());
			Gdiplus::Color clr;
			bm.GetHBITMAP(clr, &hbm);
#endif // UNDER_CE

			if (hbm == NULL) {
				// Картинка, видимо, битая... Удалить файл. [tr: The picture seems to ???... Delete the file.]
				DeleteFile(w.c_str());
				bHBITMAPInited = false;
			} else {

				// Подчищаем кеш [tr: Erase the cache?]
				while (!m_lstLastUsed.empty()) {
					MEMORYSTATUS ms;
					ms.dwLength = sizeof(ms);
					GlobalMemoryStatus(&ms);

					if (((ms.dwAvailPhys - nNewBMSize) < 2*1024*1024) || (m_mapCachedFiles.size() > m_nMaxCacheSize))
						DeleteFrontElementFromCache();
					else
						break;
				}

				// Отресайзить и положить в кеш [tr: ??? and put it in the cache?]
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

					// Здесь можно использовать любой алгоритм стретча, можно медленный
					// [tr: ??? stretch algorithm can be slow ???]
#ifdef UNDER_CE
#	if UNDER_CE >= 0x0500
					SetStretchBltMode(dstdc, BILINEAR);
					POINT pt;
					SetBrushOrgEx(dstdc, 0, 0, &pt);
#	endif // UNDER_CE >= 0x0500

#else // UNDER_CE
					SetStretchBltMode(dstdc, STRETCH_HALFTONE);
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

		// Check if this code works better 
		// http://www.codeguru.com/cpp/g-m/bitmap/specialeffects/article.php/c4897/
	}

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
		if (m_GMFH.ProcessPrefixes(s) == 0) {
			m_bGotMapVersions = true;
		}
	}

	std::map< std::string, GEOFILE_DATA >::iterator it = m_mapRequestsSent.find(request);
	if (it != m_mapRequestsSent.end()) {
		m_GMFH.OnRequestProcessed(request, it->second, data, size);
		if (m_Missing == it->second)
			m_fMissing = false;
		m_mapRequestsSent.erase(it);
	} else {
		// Ну что уж тут поделаешь...
	}
}

void CGMPainter::DownloadAddCurrentView()
{
	m_grectToDownload = m_grectLastViewed;
	// Помечаем, что больше не надо запоминать текущий GeoRect
	m_bGeoRectToDownload = true;
}

long CGMPainter::EnumerateAndProcessGeoRect(const GeoRect &gr, long nLevel, enumGMapType type, 
	long *pnInCacheCount, bool bJustCount)
{
	GEOFILE_DATA topleft, bottomright;
	GetFileDataByPoint(&topleft, GeoPoint(m_grectToDownload.minLon, m_grectToDownload.minLat), nLevel);
	GetFileDataByPoint(&bottomright, GeoPoint(m_grectToDownload.maxLon, m_grectToDownload.maxLat), nLevel);

	RECT r;
	r.top = topleft.Y; r.bottom = bottomright.Y;
	r.left = topleft.X; r.right = bottomright.X;

	long nCount = 0;
	long nInCache = 0;
	long nMinLevel = (app.m_Options[mcoDownloadLowerLevels]) ? 1 : m_nLevelToDownload - 1;
	GEOFILE_DATA data;
	data.type = type;
	for (int level = m_nLevelToDownload; level > nMinLevel; level--) {
		data.level = (unsigned char)  (LEVEL_REVERSE_OFFSET - level);
		for (long x = r.left; x <= r.right; x++) {
			data.X = x;
			for (long y = r.bottom; y <= r.top; y++) {
				data.Y = y;
				if (m_GMFH.IsFileInCache(data)) {
					nInCache++;
				} else {
					nCount++;
					if (!bJustCount) {
						m_GMFH.AddFileToDownload(data);
					}
				}
			}
		}
		// Переходим от текущего слоя к менее детальному.
		r.left /= 2; r.right /= 2; r.top /= 2; r.bottom /= 2;
	}
	if (pnInCacheCount)
		*pnInCacheCount = nInCache;
	return nCount;
}

void CGMPainter::DownloadStartWithCurrentZoom()
{
	if (m_bGeoRectToDownload) {
		long nInCache;
		long nCount = EnumerateAndProcessGeoRect(m_grectToDownload, m_nLevelToDownload, m_enTypeToDownload, &nInCache, true);
		wchar_t buf[256];
		wsprintf(buf, L("%d new segments to download (%d in cache). Proceed?"), nCount, nInCache);
		if (IDYES == MessageBox(NULL, buf, L"gpsVP", MB_YESNO | MB_ICONQUESTION)) {
			EnumerateAndProcessGeoRect(m_grectToDownload, m_nLevelToDownload, m_enTypeToDownload, NULL, false);
			m_bGeoRectToDownload = false;
		} else {
		}
	}
}

long CGMPainter::DownloadMapBy(enumGMapType type, CTrack &track, long nPixelRadius, long nDetailedLevel)
{
	return 1;
}

void CGMPainter::ProcessWMHIBERNATE()
{
	// Грохнуть всё содержимое кеша картинок
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
	GEOFILE_RASTERIZED g(m_lstLastUsed.front());
	m_lstLastUsed.pop_front();

	std::map< GEOFILE_RASTERIZED, GEOFILE_CONTENTS >::iterator it = m_mapCachedFiles.find(g);
	if (it != m_mapCachedFiles.end())
	{
		DeleteObject(it->second.h);
		m_mapCachedFiles.erase(it);
	}
}
