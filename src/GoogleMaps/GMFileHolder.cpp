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
#include "GMFileHolder.h"
#include "../PlatformDef.h"
#include "../Lock.h"

//const unsigned char MTSERVER_MAXID = 3;
//const unsigned char KHSERVER_MAXID = 3;

const DWORD MID_REQUEST_MSEC = 1000;

CGMFileHolder::CGMFileHolder(void)
	: m_WMSMapsListed(false)
{
	m_bInitialized = false;
	m_wdLastRequestTicks = 0;

	// Создаём именаторы
	m_vecRMS.push_back(new CNullSource());  // gtNone
	m_vecRMS.push_back(new COSMSource());   // gtOsm
	m_vecRMS.push_back(new CGMapSource());  // gtMap
	m_vecRMS.push_back(new CGSatSource());  // gtSatellite
	m_vecRMS.push_back(new CGTopoSource()); // gtTopo
	m_vecRMS.push_back(new CMSMapSource()); // gtMSMap
	m_vecRMS.push_back(new CMSSatSource()); // gtMSSat
	m_vecRMS.push_back(new CMSHybSource()); // gtMSHyb
	m_vecRMS.push_back(new CNullSource());  // gtHybrid
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
		std::wstring strMapConfigsPath = FindApplicationBasePath() + L"MapConfigs\\";
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

	std::wstring FindApplicationBasePath()
	{
		TCHAR szTempName[MAX_PATH]=TEXT("\0");
		int iLen = GetModuleFileName(NULL,szTempName,MAX_PATH);
		if (0 != iLen)
		{
			std::wstring strPath = szTempName;
			size_t found1 = strPath.rfind(L"\\");
			if (std::string::npos != found1)
			{
				return strPath.substr(0, found1+1); // +1 to keep the trailing slash
			}
			else return L"";
		}
		else return L"";
	}
};

void CGMFileHolder::FindAndAddWMSMaps(const CVersionNumber& gpsVPVersion)
{
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

const long CGMFileHolder::GetFileName(std::wstring& name, const GEOFILE_DATA& data) const
{
	AutoLock l;

	std::wstring path, filename;
	FILE *pFile = NULL;
	if (!GetDiskFileName(data, path, filename)) {
	} else {
		filename = m_strMapsRoot + L"/" + path + L"/" + filename;
		pFile = wfopen(filename.c_str(), L"r");
	}

	if (pFile) {
		fclose(pFile);
		pFile = NULL;
		name = filename;
		return 0;
	} else {
		// Не найдено
//		if (m_strDefaultFileName.empty()) {
			return 1;
//		} else {
//			name = m_strDefaultFileName;
//			return 0;
//		}
	}
}

long CGMFileHolder::InitFromDir(const wchar_t *pszRoot, const CVersionNumber& gpsVPVersion, bool bCreateIndexIfNeeded)
{
	AutoLock l;

	m_strMapsRoot = pszRoot;

	// Проверяем, лежит ли в этой директории файл 404.*
	FILE *f404 = wfopen((m_strMapsRoot + L"\\404.png").c_str(), L"r");
	if (f404 && (m_strDefaultFileName.empty())) {
		m_strDefaultFileName = m_strMapsRoot + L"\\404.png";
	}
	if (f404) {
		fclose(f404);
		f404 = NULL;
	}

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
		// Какой-то странный файл...
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

	// Сразу писать в правильное место НЕЛЬЗЯ!
	// Иначе поток, который рисует, может наткнуться на файл нулевой длины 
	// (который в текущий момент как раз записывается) и грохнет его.
	std::wstring tmpfilename = m_strMapsRoot + L"/__tmpfile";

	FILE * file = wfopen(tmpfilename.c_str(), L"wb");
	if (file) {
		std::wstring filename = path + L"/" + name;
		int nResult = IDRETRY;
		while (nResult == IDRETRY) {
			if (fwrite(data, size, 1, file) != 1) {
				// Не получилось записать на диск
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

		bool res = DeleteFile(filename.c_str());
		res = MoveFile(tmpfilename.c_str(), filename.c_str());

		// На всякий случай, вдруг поместили...
		m_setToDownload.erase(gfdata);
	}

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

bool CGMFileHolder::IsFileInCache(const GEOFILE_DATA& data)
{
	std::wstring name;
	return (GetFileName(name, data) == 0);
}

HANDLE CGMFileHolder::RelocateFiles(HANDLE h, long nMaxMSec)
{
	if (nMaxMSec != INFINITE)
		return NULL;

	// DWORD nStartTicks;
	// nStartTicks = GetTickCount();

	// Рекурсивно обойти директорию и перетащить файлы, которые не на своём месте.
	RelocateFilesInDir(m_strMapsRoot, L"");

	// Удалить пустые директории
	DeleteDirIfEmpty(m_strMapsRoot, false);

	return NULL;
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
			// Добавить в дальнейший поиск. Проверяем, что не . или ..
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
			// Файл - разобрать имя на фрагменты
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
				// Непонятное имя - оставляем файл на месте
			}

			if (bNameParsed) {
				std::wstring goodpath, goodname;
				if (GetDiskFileName(data, goodpath, goodname, m_strMapsRoot)) {
					if (goodpath != wstrCurPath) {
						bool bDeleteAtGood = false;
						bool bDeleteSource = false;
						std::wstring name = (wstrCurPath + L"/" + fd.cFileName).c_str();
						std::wstring atgoodname = (goodpath + L"/" + fd.cFileName).c_str();
						// Сравнить даты модификации файлов и оставить самый новый
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
								// Похоже, нам не дали читать файл (прав нет?). 
								// Вообще, непонятно, что здесь нужно делать... Оставить всё на своих местах?
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

	// Пока в бетах (0.4.4 и 0.4.5) было только "zoom=%d" в корне директории с картами.
	// На всякий случай - и gmap/zoom тоже, мало ли, вдруг VShorin себе так наскачивал.
	// Их и проверяем.
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
	enumGMapType vecTypesInRequest[] = {gtMap, gtSatellite, gtHybrid, gtTopo};
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
			m_vecRMS[vecTypesInRequest[i]]->SetServerPrefixes(lst);
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
