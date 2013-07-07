/*
Copyright (c) 2005-2008, Jerome077 and others
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "RasterMapFinder.h"

// ---------------------------------------------------------------------------------------

std::wstring CCustomMapFinder::GetNameWithoutINIExtension(const WIN32_FIND_DATA &FindFileData)
{
	std::wstring strFilename = FindFileData.cFileName;
	return strFilename.substr(0, strFilename.length()-4);
}

std::wstring CCustomMapFinder::GetNameWithoutSQLiteExtension(const WIN32_FIND_DATA &FindFileData)
{
	std::wstring strFilename = FindFileData.cFileName;
	return strFilename.substr(0, strFilename.length()-9);
}

bool CCustomMapFinder::isNormalDirectory(const WIN32_FIND_DATA& ffd)
{
	return (  ((ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
		   && ('.' != ffd.cFileName[0]) // no "." at the beginning
		   );
}

bool CCustomMapFinder::hasFile(const std::wstring& mapRootFolder, const WIN32_FIND_DATA& ffd, const std::wstring& fileMask)
{
	WIN32_FIND_DATA wwd;
	std::wstring strSearch = mapRootFolder + L"\\" + ffd.cFileName + L"\\" + fileMask;
	HANDLE h = FindFirstFile(strSearch.c_str(), &wwd);
	if (h && (h != INVALID_HANDLE_VALUE))
	{
		FindClose(h);
		if (!(wwd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			return true;
	}
	return false;
}

// ---------------------------------------------------------------------------------------

CMapConfigFinder::CMapConfigFinder(const std::wstring& strAppBasePath, const std::wstring& strMapsRoot)
{
	WIN32_FIND_DATA FindFileData;

	// Find all config files from the MapConfigs folder:
	std::wstring strMapConfigsPath = strAppBasePath + L"MapConfigs\\";
	std::wstring strSearch = strMapConfigsPath + L"*.ini";
	HANDLE hFind = FindFirstFile(strSearch.c_str(), &FindFileData);
	if (INVALID_HANDLE_VALUE != hFind)
	{
		do
		{
			CMapItem mapConfig;
			mapConfig.MapName = GetNameWithoutINIExtension(FindFileData);
			mapConfig.MapPath = strMapConfigsPath + FindFileData.cFileName;
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
				&& hasFile(strMapsRoot, FindFileData, L"mapcfg.ini")
			   )
			{
				CMapItem mapConfig;
				mapConfig.MapName = FindFileData.cFileName;
				mapConfig.MapPath = strMapsRoot + L"\\" + FindFileData.cFileName + L"\\mapcfg.ini";
				m_list.push_back(mapConfig);
			}
		}
		while (FindNextFile(hFind, &FindFileData));
		FindClose(hFind);
	}
}

// ---------------------------------------------------------------------------------------

CSQLiteMapFinder::CSQLiteMapFinder(const std::wstring& strMapsRoot)
{
	WIN32_FIND_DATA FindFileData;

	// Find all .sqlitedb files directly from cache folder:
	std::wstring strSearch = strMapsRoot + L"\\*.sqlitedb";
	HANDLE hFind = FindFirstFile(strSearch.c_str(), &FindFileData);
	if (INVALID_HANDLE_VALUE != hFind)
	{
		do
		{
			CMapItem mapItem;
			mapItem.MapName = GetNameWithoutSQLiteExtension(FindFileData);
			mapItem.MapPath = strMapsRoot + L"\\" + FindFileData.cFileName;
			m_list.push_back(mapItem);
		}
		while (FindNextFile(hFind, &FindFileData));
		FindClose(hFind);
	}
}

// ---------------------------------------------------------------------------------------

CMultiSQLiteMapFinder::CMultiSQLiteMapFinder(const std::wstring& strMapsRoot)
{
	WIN32_FIND_DATA FindFileData;

	// Find all .sqlitedb files collections in subfolfers of the cache folder:
	std::wstring strSearch = strMapsRoot + L"\\*";
	HANDLE hFind = FindFirstFile(strSearch.c_str(), &FindFileData);
	if (INVALID_HANDLE_VALUE != hFind)
	{
		do
		{
			if (   isNormalDirectory(FindFileData)
				&& hasFile(strMapsRoot, FindFileData, L"*.sqlitedb")
			   )
			{
				CMapItem mapItem;
				mapItem.MapName = FindFileData.cFileName;
				mapItem.MapPath = strMapsRoot + L"\\" + FindFileData.cFileName + L"\\*.sqlitedb";
				m_list.push_back(mapItem);
			}
		}
		while (FindNextFile(hFind, &FindFileData));
		FindClose(hFind);
	}
}

// ---------------------------------------------------------------------------------------
