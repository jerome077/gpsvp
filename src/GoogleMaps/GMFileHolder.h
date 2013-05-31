﻿/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include "../Track.h"
#include "RasterServerSources.h"
#include "../VersionNumber.h"
#include "../FileFormats/Decoder_Common.h"
#if UNDER_CE && _WIN32_WCE < 0x500
#else
#  include "../FileFormats/Decoder_7z.h"
#endif

typedef std::set< GEOFILE_DATA > GeoDataSet;

class CGMFileHolder
{
public:
	CGMFileHolder(void);
	virtual ~CGMFileHolder(void);

	void SetDefaultFileName(const wchar_t * pszDefaultFile)
	{
		m_strDefaultFileName = pszDefaultFile;
	};

	long InitFromDir(const wchar_t *pszRoot, const CVersionNumber& gpsVPVersion, bool bCreateIndexIfNeeded = true);
	void Deinit();

	bool GetFileName(const GEOFILE_DATA& data, CDecoderTileInfo*& itemInfo) const;
	std::wstring GetUnzippedFileName(const GEOFILE_DATA& data) const;
	// This method provides request URL to the painter when it lacks a tile
	std::string GetRequestURL(const GEOFILE_DATA& data);
	// Get an acquired tile
	bool GetQueuedData(GEOFILE_DATA* pData);
	long OnRequestProcessed(const std::string request, GEOFILE_DATA& gfdata, const char * data, int size);
	long ProcessPrefixes(const std::string &s);

	size_t GetDownloadQueueSize();
	long AddFileToDownload(const GEOFILE_DATA& data);
	long AddFileToDownload(const GeoDataSet& data);
	bool IsFileInCache(const GEOFILE_DATA& data);

	long GetMaxLevel() const { return m_nMaxLevel; };
	long GetMinLevel() const { return m_nMinLevel; };
	bool empty() { return false; };

	HANDLE RelocateFiles(HANDLE h, long nMaxMSec = INFINITE); 
	bool NeedRelocateFiles(); 

	bool IsInsideRegion(const GEOFILE_DATA &data, const GeoRect &region);
	size_t ListFilesInsideRegion(GeoDataSet *pSet, enumGMapType type, const GeoRect *pRegion);
	size_t ListFilesInsideRegion(GeoDataSet *pSet, enumGMapType type, const std::wstring &wstrCurPath, const GeoRect *pRegion);
	void SetOldTileDays(int nDays) { m_nOldTileDays = nDays; }

	const CRasterMapSource *GetRMS(enum enumGMapType t) const
	{
		if ((size_t)t < m_vecRMS.size())
			return m_vecRMS[t];
		else 
			return NULL;
	};

	long GetGMapCount() const { return m_vecRMS.size(); };
	long GetUserMapCount() const { return m_vecRMS.size()-gtFirstUserMapType; };
	std::wstring GetUserMapName(long indexUserMap) const;
	GeoPoint GetDemoPoint(enumGMapType type, double &scale) const;

protected:
	long BuildInternalIndex();

	bool RelocateFilesInDir(std::wstring wstrCurPath, std::wstring wstrPartPath);
	bool DeleteDirIfEmpty(std::wstring sDir, bool bDeleteThis = true);

	bool GetDiskFileName(const GEOFILE_DATA& gfdata, std::wstring &path, std::wstring &name, const std::wstring root = L"") const;

    void FindAndAddUserMaps(const CVersionNumber& gpsVPVersion);

private:
	// Common prefix for all raster map folders
	std::wstring m_strMapsRoot;
	DWORD m_dwMapsAttr;
	// Inited
	bool m_bInitialized;

	GeoDataSet m_setToDownload;
	FILETIME m_ftOldTile;
	int m_nOldTileDays;

	// File name returned in case the requested tile is absent
	std::wstring m_strDefaultFileName;

	// Minimum and maximum zoom levels (0 and 18 for maps)
	long m_nMinLevel, m_nMaxLevel; 

	// Current server number
	std::vector<PRasterMapSource> m_vecRMS;
	bool m_UserMapsListed;

	unsigned char m_nMTServerId;
	unsigned char m_nKHServerId;

	DWORD m_wdLastRequestTicks;
};
