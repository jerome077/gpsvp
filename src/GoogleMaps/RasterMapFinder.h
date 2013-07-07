/*
Copyright (c) 2005-2008, Jerome077 and others
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef RASTER_MAP_FINDER_H
#define RASTER_MAP_FINDER_H

#include <string>
#include <list>
#include <windows.h>

// ---------------------------------------------------------------------------------------
// Basis class
// ---------------------------------------------------------------------------------------
class CCustomMapFinder {
public:
	// Datatype for the items of the list:
	class CMapItem
	{
	public:
		std::wstring MapName;
		std::wstring MapPath;
	};

	// iterator for the list:
	typedef std::list<CMapItem>::const_iterator const_iterator;
	CCustomMapFinder::const_iterator begin() { return m_list.begin(); };
	CCustomMapFinder::const_iterator end() { return m_list.end(); };

protected:
	std::list<CMapItem> m_list;

	std::wstring GetNameWithoutINIExtension(const WIN32_FIND_DATA &FindFileData);
	std::wstring GetNameWithoutSQLiteExtension(const WIN32_FIND_DATA &FindFileData);
	bool isNormalDirectory(const WIN32_FIND_DATA& ffd);
	bool hasFile(const std::wstring& mapRootFolder, const WIN32_FIND_DATA& ffd, const std::wstring& fileMask);
};

// ---------------------------------------------------------------------------------------
// Class which looks after .ini map configuration files
// ---------------------------------------------------------------------------------------
class CMapConfigFinder: public CCustomMapFinder {
public:
	// constructor
	CMapConfigFinder(const std::wstring& strAppBasePath, const std::wstring& strMapsRoot);
};

// ---------------------------------------------------------------------------------------
// Class which looks after simple sqlite maps
// ---------------------------------------------------------------------------------------
class CSQLiteMapFinder: public CCustomMapFinder {
public:
	// constructor
	CSQLiteMapFinder(const std::wstring& strMapsRoot);
};

// ---------------------------------------------------------------------------------------
// Class which looks after multiple sqlite maps (several files in a subfolder)
// ---------------------------------------------------------------------------------------
class CMultiSQLiteMapFinder: public CCustomMapFinder {
public:
	// constructor
	CMultiSQLiteMapFinder(const std::wstring& strMapsRoot);
};

#endif
