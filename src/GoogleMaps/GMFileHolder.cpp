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
#include "GMFileHolder.h"
#include "../PlatformDef.h"
#include "../Lock.h"
#include "../MapApp.h"
#ifndef _MSC_VER
#include <errno.h>
#endif

//const unsigned char MTSERVER_MAXID = 3;
//const unsigned char KHSERVER_MAXID = 3;

const DWORD MID_REQUEST_MSEC = 1000;

CGMFileHolder::CGMFileHolder(void)
	: m_WMSMapsListed(false)
{
	m_bInitialized = false;
	m_wdLastRequestTicks = 0;

	// Create map source namers
	m_vecRMS.push_back(new CNullSource());  // gtNone
	m_vecRMS.push_back(new COSMSource());   // gtOsm
	m_vecRMS.push_back(new CGMapSource());  // gtMap
	m_vecRMS.push_back(new CGSatSource());  // gtSatellite
	m_vecRMS.push_back(new CGTopoSource()); // gtTopo
	m_vecRMS.push_back(new CMSMapSource()); // gtMSMap
	m_vecRMS.push_back(new CMSSatSource()); // gtMSSat
	m_vecRMS.push_back(new CMSHybSource()); // gtMSHyb
//	m_vecRMS.push_back(new CNullSource());  // gtHybrid
}

CGMFileHolder::~CGMFileHolder(void)
{
	for (long i=gtNone, iEnd=GetGMapCount(); i<iEnd; i++) {
		if (m_vecRMS[i] != NULL) {
			delete m_vecRMS[i];
			m_vecRMS[i] = NULL;
		}
	}
}


// Class which looks after all possible map configuration files
class CMapConfigFinder
{
public:
	// Datatype for the items of the list:
	class CMapConfig
	{
	public:
		std::wstring MapName;
		std::wstring IniFileWithPath;
	};
	// iterator for the list:
	typedef std::list<CMapConfig>::const_iterator const_iterator;
	CMapConfigFinder::const_iterator begin() { return m_list.begin(); };
	CMapConfigFinder::const_iterator end() { return m_list.end(); };

	// constructor
	CMapConfigFinder(const std::wstring& strMapsRoot)
	{
		WIN32_FIND_DATA FindFileData;

		// Find all config files from the MapConfigs folder:
		std::wstring strMapConfigsPath = app.m_wstrBasePath + L"MapConfigs\\";
		std::wstring strSearch = strMapConfigsPath + L"*.ini";
		HANDLE hFind = FindFirstFile(strSearch.c_str(), &FindFileData);
		if (INVALID_HANDLE_VALUE != hFind)
		{
			do
			{
				CMapConfig mapConfig;
				mapConfig.MapName = GetNameWithoutINIExtension(FindFileData);
				mapConfig.IniFileWithPath = strMapConfigsPath + FindFileData.cFileName;
				m_list.push_back(mapConfig);
			}
			while (FindNextFile(hFind, &FindFileData));
			FindClose(hFind);
		}

		// Find all folders in the map cache with a config file:
		strSearch = strMapsRoot + L"\\*";
		hFind = FindFirstFile(strSearch.c_str(), &FindFileData);
		if (INVALID_HANDLE_VALUE != hFind)
		{
			do
			{
				if (   isNormalDirectory(FindFileData)
					&& hasMapCfgFile(strMapsRoot, FindFileData)
				   )
				{
					CMapConfig mapConfig;
					mapConfig.MapName = FindFileData.cFileName;
					mapConfig.IniFileWithPath = strMapsRoot + L"\\" + FindFileData.cFileName + L"\\mapcfg.ini";
					m_list.push_back(mapConfig);
				}
			}
			while (FindNextFile(hFind, &FindFileData));
			FindClose(hFind);
		}
	};

protected:
	std::list<CMapConfig> m_list;

	std::wstring GetNameWithoutINIExtension(const WIN32_FIND_DATA &FindFileData)
	{
		std::wstring strFilename = FindFileData.cFileName;
		return strFilename.substr(0, strFilename.length()-4);
	}

	bool isNormalDirectory(const WIN32_FIND_DATA& ffd)
	{
		return (  ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
			   && ('.' != ffd.cFileName[0]) // no "." at the beginning
			   );
	}

	bool hasMapCfgFile(const std::wstring& mapRootFolder, const WIN32_FIND_DATA& ffd)
	{
		WIN32_FIND_DATA wwd;
		std::wstring strSearch = mapRootFolder + L"\\" + ffd.cFileName + L"\\mapcfg.ini";
		HANDLE h = FindFirstFile(strSearch.c_str(), &wwd);
		if (h && (h != INVALID_HANDLE_VALUE))
		{
			FindClose(h);
			if (!(wwd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				return true;
		}
		return false;
	}
};

void CGMFileHolder::FindAndAddWMSMaps(const CVersionNumber& gpsVPVersion)
{
	// Test cases for the variables can be through the following line activated:
	// Test_CStringSchema();

	WPARAM currentWMSNumber = gtFirstWMSMapType;

    bool bMapErrors = false;
	std::wstring sMapErrors = L"Following maps won't be loaded (wrong configuration):";
	bool bMapVersionWarnings = false;
	std::wstring sMapVersionWarnings = L"Following maps are for a newer version of gpsVP:";
	CMapConfigFinder configList(m_strMapsRoot);
	CMapConfigFinder::const_iterator iter = configList.begin();
	while (configList.end() != iter)
	{
		CUserWMSMapSource* mapSource = new CUserWMSMapSource(currentWMSNumber,
			                                                 iter->MapName,
			                                                 iter->IniFileWithPath,
												             m_strMapsRoot,
												             gpsVPVersion);
		if (CUserWMSMapSource::cecError == mapSource->GetConfigError())
		{
			bMapErrors = true;
			sMapErrors += L" " + iter->MapName;
			delete mapSource; //don't keep the map
		}
		else if (CUserWMSMapSource::cecMapVersionNewerAsGpsVP == mapSource->GetConfigError())
		{
			bMapVersionWarnings = true;
			sMapVersionWarnings += L" " + iter->MapName;
			m_vecRMS.push_back(mapSource); // Keep the map anyway
			currentWMSNumber++;
		}
		else
		{
			m_vecRMS.push_back(mapSource); // Keep the map
			currentWMSNumber++;
		}
		if (currentWMSNumber > gtLastGMapType) break; // Maximal count of WMS-Maps reached
		iter++;
	}
	if (bMapErrors)
		MessageBox(NULL, sMapErrors.c_str(), L"Wrong maps", MB_ICONEXCLAMATION);
	if (bMapVersionWarnings)
		MessageBox(NULL, sMapVersionWarnings.c_str(), L"Please update gpsVP", MB_ICONEXCLAMATION);
}

// GetUnzippedFileName:
// * returns the theoritical tile file name without checking if the file exist in the cache.
std::wstring CGMFileHolder::GetUnzippedFileName(const GEOFILE_DATA& data) const
{
	AutoLock l;

	std::wstring path, filename;
	FILE *pFile = NULL;
	if (!GetDiskFileName(data, path, filename))
		return L"";
	else
		return m_strMapsRoot + L"/" + path + L"/" + filename;
}

// GetFileName:
// * returns true, zipIndex=-1, name = tile file name, when the tile is not zipped.
// * returns true, zipIndex>=0, name = zip file name, when the tile is zipped.
// * returns false, name = theoritical tile file name, when the tile does not exist in the cache.
bool CGMFileHolder::GetFileName(const GEOFILE_DATA& data, std::wstring& name, int& zipIndex) const
{
	AutoLock l;

	zipIndex = -1;
	std::wstring path, filename;
	FILE *pFile = NULL;
	if (!GetDiskFileName(data, path, filename))
		return false;

	// Check if there is a file
	std::wstring fullname = m_strMapsRoot + L"/" + path + L"/" + filename;
	pFile = wfopen(fullname.c_str(), L"r");
	if (pFile)
	{
		fclose(pFile);
		pFile = NULL;
		name = fullname;
		return true;
	}

	// Check if there is a file in a zipfile (actually 7z file, trying all possible files on the path)
	#if UNDER_CE && _WIN32_WCE < 0x500
	#else
	std::wstring strZip, strNameInZip; 
	std::replace(path.begin(), path.end(), L'\\', L'/');
	std::wstringstream wssPath(path);
	std::wstring strPathToCurrent = m_strMapsRoot + L"/";
	std::wstring strCurrent;
	int startPosCurrentInPath = 0;
	while (std::getline(wssPath, strCurrent, L'/'))
	{
		strZip = strPathToCurrent + strCurrent + L".7z";
		strNameInZip = path.substr(startPosCurrentInPath) + L"/" + filename;
		pFile = wfopen(strZip.c_str(), L"r");
		if (pFile)
		{
			fclose(pFile);
			pFile = NULL;
			CDecoder7z* pDec7z = M_Decoder7zPool.GetDecoder(strZip);
			if (pDec7z->IsFileOk())
			{
				zipIndex = pDec7z->FindItem(strNameInZip.c_str());
				if (-1 != zipIndex)
				{
					name = strZip;
					return true;
				}
			}
		}
		strPathToCurrent += strCurrent + L"/";
		startPosCurrentInPath += strCurrent.length() + 1;
	}
	#endif
	return false;
}

long CGMFileHolder::InitFromDir(const wchar_t *pszRoot, const CVersionNumber& gpsVPVersion, bool bCreateIndexIfNeeded)
{
	AutoLock l;

	m_strMapsRoot = pszRoot;

	// Checking if the folder contains file 404.*
	FILE *f404 = wfopen((m_strMapsRoot + L"\\404.png").c_str(), L"r");
	if (f404 && (m_strDefaultFileName.empty())) {
		m_strDefaultFileName = m_strMapsRoot + L"\\404.png";
	}
	if (f404) {
		fclose(f404);
		f404 = NULL;
	}

	// Retrieve from 'attrib' file the attributes to apply to any new
	// tile image file. Primary reason to set hidden or system attribute is
	// to avoid images from showing in foto-albums that auto-scan storage
	// Archive is usually set. Hidden will prevent most album scans. System even more
    // m_dwMapsAttr = FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM;
	m_dwMapsAttr = GetFileAttributes((m_strMapsRoot + L"\\attrib").c_str());
	// I consider this a temporary workaround, it may be nicer and clearer for users
	// to create a proper settigns in VP (also for the 404 above) rather than 
	// non-intuitive non-documented "settings" like this

	// New code: do we need above still for should it be scrapped?
	if (INVALID_FILE_ATTRIBUTES == m_dwMapsAttr)
		m_dwMapsAttr = FILE_ATTRIBUTE_ARCHIVE;
	if (app.m_Options[mcoHideCacheTiles])
		m_dwMapsAttr = m_dwMapsAttr | FILE_ATTRIBUTE_HIDDEN;

	NeedRelocateFiles();

	// Adding WMS-Maps to the list
	// (Currently only the first time. You have to restart the program if you change the cache folder)
	if (! m_WMSMapsListed)
	{
		FindAndAddWMSMaps(gpsVPVersion);
		m_WMSMapsListed = true;
	}

	return 0;
}

void CGMFileHolder::Deinit()
{
}

std::string CGMFileHolder::GetRequestURL(const GEOFILE_DATA& data)
{
	AutoLock l;

	return m_vecRMS[data.type]->GetRequestURL(data);
}

bool CGMFileHolder::GetQueuedData(GEOFILE_DATA* pData)
{
	AutoLock l;

	if (!m_setToDownload.empty()) {
		DWORD dwNow = GetTickCount();
		if ((dwNow - m_wdLastRequestTicks) < MID_REQUEST_MSEC) {
			Sleep(MID_REQUEST_MSEC - dwNow + m_wdLastRequestTicks);
		}
		m_wdLastRequestTicks = GetTickCount();
		*pData = *m_setToDownload.rbegin();
		return true;
	} else {
		return false;
	}
}

long CGMFileHolder::OnRequestProcessed(const std::string request, GEOFILE_DATA& gfdata, const char * data, int size)
{
	AutoLock l;
	// No need to autolock here
	if (size < 100) {
		// Strange file...
		return 1;
	}

	//std::string::size_type q = request.find('?');
	//if (q == std::string::npos)
	//	return 1;
	//++q;

	std::wstring path, name;
	if (!GetDiskFileName(gfdata, path, name, m_strMapsRoot)) {
		return 2;
	}

	std::wstring tmpfilename = m_strMapsRoot + L"/__tmpfile";
	std::wstring filename = path + L"/" + name;

	// First, we compare old and new contents. If the are identical then just touch file.
	FILE * file = wfopen(filename.c_str(), L"rb");
	bool bWillChange = true;
	if (file) {
		fseek(file, 0, SEEK_END);
		long nSize = ftell(file);
		fseek(file, 0, SEEK_SET);
		if (nSize == size) {
			const int chunksize = 1*1024;
			unsigned char buf[chunksize];
			int cur = 0;
			while (true) {
				long r = fread(buf, 1, chunksize, file);
				if (memcmp(buf, data+cur, r) != 0) {
					break;
				}
				cur += r;
				if (cur == size) {
					bWillChange = false;
					break;
				} else if (r < chunksize) {
					// Error reading?
					break;
				}
			}
		}
		fclose(file);
	}
	if (!bWillChange) {
		HANDLE hFile = CreateFile(filename.c_str(), GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 
			NULL, OPEN_EXISTING, 0, 0);
		if (hFile) {
			SYSTEMTIME stNow;
			GetSystemTime(&stNow);
			FILETIME ftNow;
			SystemTimeToFileTime(&stNow, &ftNow);
			SetFileTime(hFile, NULL, NULL, &ftNow);
			CloseHandle(hFile);
		}
	} else {
		//   DO NOT WRITE to the right place right now, as the painting
		// thread could find an empty file (being written to) and kill it.
		file = wfopen(tmpfilename.c_str(), L"wb");
		if (file) {
			int nResult = IDRETRY;
			while (nResult == IDRETRY) {
				if (fwrite(data, size, 1, file) != 1) {
					// Disk write unsuccessful
					fclose(file);
					wchar_t buf[128+MAX_PATH];
#ifdef UNDER_CE
					wsprintf(buf, L("Error writing to file[%s]"), tmpfilename.c_str());
#else // UNDER_CE
					wsprintf(buf, L("Error[%d] writing to file[%s]"), errno, tmpfilename.c_str());
#endif // UNDER_CE
					nResult = MessageBox(NULL, buf, L"gpsVP", MB_RETRYCANCEL | MB_ICONERROR);
					if (nResult == IDCANCEL) {
						return 3;
					}
				} else {
					break;
				}
			}
			fclose(file);

			// Apply attributes if needed (assume archive is default)
			if (m_dwMapsAttr != FILE_ATTRIBUTE_ARCHIVE) {
				SetFileAttributes(tmpfilename.c_str(), m_dwMapsAttr);
			}

			// Move file from temporary to proper location
			bool res = (DeleteFile(filename.c_str()) != 0);
			res = (MoveFile(tmpfilename.c_str(), filename.c_str()) != 0);
		}
	}
	m_setToDownload.erase(gfdata);

	return 0;
}

size_t CGMFileHolder::GetDownloadQueueSize()
{
	AutoLock l;

	return m_setToDownload.size();
}

long CGMFileHolder::AddFileToDownload(const GEOFILE_DATA& data)
{
	AutoLock l;

	return (m_setToDownload.insert(data).second ? 0 : 1);
}

long CGMFileHolder::AddFileToDownload(const GeoDataSet& data)
{
	AutoLock l;

	m_setToDownload.insert(data.begin(), data.end());

	return 0;
}

bool CGMFileHolder::IsFileInCache(const GEOFILE_DATA& data)
{
	std::wstring name;
	int zipIndex;
	return GetFileName(data, name, zipIndex);
}

HANDLE CGMFileHolder::RelocateFiles(HANDLE h, long nMaxMSec)
{
	if (nMaxMSec != INFINITE)
		return NULL;

	// DWORD nStartTicks;
	// nStartTicks = GetTickCount();

	// Recursively walk through the folders and move files not in place
	RelocateFilesInDir(m_strMapsRoot, L"");

	// Remove empty folders
	DeleteDirIfEmpty(m_strMapsRoot, false);

	return NULL;
}

size_t CGMFileHolder::ListFilesInsideRegion(GeoDataSet *pSet, enumGMapType type, const GeoRect *pRegion)
{
	// Get the prefix
	GEOFILE_DATA data(type, 1, 1, 1);
	std::wstring name, path;
	bool bRes = m_vecRMS[type]->GetDiskFileName(data, path, name, L"");
	std::wstring prefix = path.substr(0, path.find(L"/"));

	// Get the minimum FILETIME
	SYSTEMTIME st;
	GetSystemTime(&st);
	SystemTimeToFileTime(&st, &m_ftOldTile);
	ULARGE_INTEGER u;
	u.HighPart = m_ftOldTile.dwHighDateTime;
	u.LowPart = m_ftOldTile.dwLowDateTime;
	u.QuadPart -= (ULONGLONG) m_nOldTileDays * 24 * 60 * 60 * 10000000;
	m_ftOldTile.dwHighDateTime = u.HighPart;
	m_ftOldTile.dwLowDateTime = u.LowPart;	
	FileTimeToSystemTime(&m_ftOldTile, &st);

    return ListFilesInsideRegion(pSet, type, m_strMapsRoot + L"/" + prefix, pRegion);
}

bool CGMFileHolder::IsInsideRegion(const GEOFILE_DATA &data, const GeoRect &region)
{
	long nNumTiles = 1 << (LEVEL_REVERSE_OFFSET - data.level - 2);
	double a = exp(pi*2*(1.0 - (double) data.Y/nNumTiles));
	double z = (a-1)/(a+1);
	double lat = asin(z)/pi*180;
	double lon = ((double) data.X / (nNumTiles) - 1) * 180;
	GeoPoint lu(lon, lat);
	a = exp(pi*2*(1.0-((double) data.Y+1)/nNumTiles));
	z = (a-1)/(a+1);
	lat = asin(z)/pi*180;
	lon = ((double) (data.X+1) / (nNumTiles) - 1) * 180;
	GeoPoint rl(lon, lat);
	GeoRect r;
	r.Init(lu);
	r.Append(rl);
	return r.Intersect(region);
}

size_t CGMFileHolder::ListFilesInsideRegion(GeoDataSet *pSet, enumGMapType type, const std::wstring &wstrCurPath, const GeoRect *pRegion)
{
	size_t nCount = 0;
	const wchar_t *pszMask = L"\\*";
	WIN32_FIND_DATA fd;
	HANDLE hSearch = FindFirstFile((wstrCurPath + pszMask).c_str(), &fd);
	BOOL bNextFound = TRUE;

	GEOFILE_DATA data;

	while ((hSearch != INVALID_HANDLE_VALUE) && (bNextFound)) {
		std::wstring name(fd.cFileName);
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			// Folder: pass to the lower recursion level. Check it is not . or ..
			if ((name != L".") && (name != L"..")) {
				nCount += ListFilesInsideRegion(pSet, type, wstrCurPath + L"/" + name, pRegion);
			}
		} else {
			// Parse file name into segments
			bool bInside = false;
			if (m_vecRMS[type]->IsGoodFileName(data, name)) {
				if (!pRegion)
					bInside = true;
				else {
					// Check if this tile inside a region
					bInside = IsInsideRegion(data, *pRegion);
				}
			}
			if (bInside) {
				std::wstring fullname = (wstrCurPath + L"/" + name).c_str();
				HANDLE hSrc = CreateFile(fullname.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, 
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				if (hSrc) {
					FILETIME ftModified;
					GetFileTime(hSrc, NULL, NULL, &ftModified);
					if (CompareFileTime(&ftModified, &m_ftOldTile) < 0) {
						// Need to refresh
						pSet->insert(data);
						++nCount;
					}
				}
				CloseHandle(hSrc);
			} else {
				// Incomprehensible name--leave the file in place
			}
		}

		bNextFound = FindNextFile(hSearch, &fd);
	}
	if (hSearch) {
		FindClose(hSearch);
		hSearch = NULL;
	}

	return nCount; // This dir completed
}

bool CGMFileHolder::RelocateFilesInDir(std::wstring wstrCurPath, std::wstring wstrPartPath)
{
	const wchar_t *pszMask = L"\\*";
	WIN32_FIND_DATA fd;
	HANDLE hSearch = FindFirstFile((wstrCurPath + pszMask).c_str(), &fd);
	BOOL bNextFound = TRUE;

	GEOFILE_DATA data;

	while ((hSearch != INVALID_HANDLE_VALUE) && (bNextFound)) {
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			// Folder: pass to the lower recursion level. Check it is not . or ..
			std::wstring name = fd.cFileName;
			if ((name != L".") && (name != L"..")) {
				std::wstring newpath;
				if (wstrPartPath == L"") {
					newpath = fd.cFileName;
				} else {
					newpath = wstrPartPath + L"/" + fd.cFileName;
				}
				RelocateFilesInDir(wstrCurPath + L"/" + fd.cFileName, newpath);
			}
		} else {
		// Parse file name into segments
			bool bNameParsed = false;
			for (long i=gtMap, iEnd=GetGMapCount(); i<iEnd; i++) {
				if (m_vecRMS[i]->IsGoodFileName(data, fd.cFileName)) {
					bNameParsed = true;
					break;
				}
			}
			if (bNameParsed) {
				std::wstring p, n;
				GetDiskFileName(data, p, n);
				if (n != fd.cFileName) {
					bNameParsed = false;
				}
			} else {
				// Incomprehensible name--leave the file in place
			}

			if (bNameParsed) {
				std::wstring goodpath, goodname;
				if (GetDiskFileName(data, goodpath, goodname, m_strMapsRoot)) {
					if (goodpath != wstrCurPath) {
						bool bDeleteAtGood = false;
						bool bDeleteSource = false;
						std::wstring name = (wstrCurPath + L"/" + fd.cFileName).c_str();
						std::wstring atgoodname = (goodpath + L"/" + fd.cFileName).c_str();
						// Compare modification dates and leave the newer file
						HANDLE hAtGoodPath = CreateFile(atgoodname.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, 
							OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

						if (hAtGoodPath == INVALID_HANDLE_VALUE) {
						} else {
							HANDLE hSrc = CreateFile(name.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, 
							OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

							if (hSrc) {
								FILETIME ftAtGoodModified, ftModified;
								GetFileTime(hAtGoodPath, NULL, NULL, &ftAtGoodModified);
								GetFileTime(hSrc, NULL, NULL, &ftModified);
								if (CompareFileTime(&ftModified, &ftAtGoodModified) > 0) {
									bDeleteAtGood = true;
								} else {
									bDeleteSource = true;
								}
								CloseHandle(hSrc);
								hSrc = NULL;
							} else {
								// It seems we can't read the file (access rights?).
								// Generally, it's hard to tell what to do... Just leave everything as is?
							}
							CloseHandle(hAtGoodPath);
							hAtGoodPath = NULL;
						}
						
						if (bDeleteSource) {
							DeleteFile(name.c_str());
						} else if (bDeleteAtGood) {
							DeleteFile(atgoodname.c_str());
						}

						if (!bDeleteSource) {
							MoveFile(name.c_str(), atgoodname.c_str());
						}
					}
				}
			}
		}


		bNextFound = FindNextFile(hSearch, &fd);
	}
	if (hSearch) {
		FindClose(hSearch);
		hSearch = NULL;
	}

	return true; // This dir completed
}

bool CGMFileHolder::DeleteDirIfEmpty(std::wstring sDir, bool bDeleteThis)
{
	std::wstring sPath = sDir;
	bool bIsDirEmpty = true;

	WIN32_FIND_DATA fd;
	HANDLE hSearch = FindFirstFile((sDir + L"\\*").c_str(), &fd);
	BOOL bNextFound = TRUE;
	while ((hSearch != INVALID_HANDLE_VALUE) && (bNextFound)) {
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			std::wstring name = fd.cFileName;
			if ((name != L".") && (name != L"..")) {
				bIsDirEmpty = DeleteDirIfEmpty(sDir + L"/" + fd.cFileName) && bIsDirEmpty;
			}
		} else {
			bIsDirEmpty = false;
			break;
		}

		bNextFound = FindNextFile(hSearch, &fd);
	}
	if (hSearch) {
		FindClose(hSearch);
		hSearch = NULL;
	}

	if (bIsDirEmpty) {
		if (bDeleteThis) {
			return (RemoveDirectory(sDir.c_str()) != 0);
		} else {
			return false;
		}
	}
	return false;
}

bool CGMFileHolder::GetDiskFileName(const GEOFILE_DATA& gfdata, std::wstring &path, 
	std::wstring &name, const std::wstring root) const
{
	return m_vecRMS[gfdata.type]->GetDiskFileName(gfdata, path, name, root);
}

bool CGMFileHolder::NeedRelocateFiles()
{
	bool bResult = false;

	// This checks are for 0.4.4 and 0.4.5 betas only.
	wchar_t *pBadWildcards[] = {
		L"\\zoom*", 
		L"\\gmap\\zoom*"
	};
	WIN32_FIND_DATA fd;
	for (int i=0; i<sizeof(pBadWildcards)/sizeof(pBadWildcards[0]); i++) {
		HANDLE hSearch = FindFirstFile((m_strMapsRoot + pBadWildcards[i]).c_str(), &fd);
		if (hSearch != INVALID_HANDLE_VALUE) {
			bResult = true;
			FindClose(hSearch);
			hSearch = NULL;
			break;
		}
	}

	return bResult;
}

long CGMFileHolder::ProcessPrefixes(const std::string &s)
{
	const char *cur = s.c_str();
	enumGMapType vecTypesInRequest[] = {gtMap, gtSatellite, gtNone /* gtHybrid */, gtTopo};
	for (int i=0; i<sizeof(vecTypesInRequest)/sizeof(vecTypesInRequest[0]); i++) {
		std::list<std::string> lst;
		const char *pGMap = strstr(cur, "["); 
		if (pGMap != NULL) {
			pGMap++;
			while (*pGMap != ']') {
				const char *pQuot = strstr(pGMap, "\""); pQuot++;
				const char *pQuotEnd = strstr(pQuot, "\"");
				std::string s(pQuot, pQuotEnd-pQuot);
				lst.push_back(s);
				pGMap = pQuotEnd + 1;
			}
			if (vecTypesInRequest[i] != gtNone) {
				m_vecRMS[vecTypesInRequest[i]]->SetServerPrefixes(lst);
			}
			cur = pGMap;
		} else {
			// Malformed reply...
			return 2;
		}
	}

	return 0;
}

std::wstring CGMFileHolder::GetWMSMapName(long indexWMS) const
{
	return ((CUserWMSMapSource *)m_vecRMS[gtFirstWMSMapType+indexWMS])->GetName();
}

GeoPoint CGMFileHolder::GetDemoPoint(enumGMapType type, double &scale) const
{
	return m_vecRMS[type]->GetDemoPoint(scale);
}
