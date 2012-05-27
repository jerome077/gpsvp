/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef VERSIONNUMBER_H
#define VERSIONNUMBER_H

#ifdef USE_STDIO_H
#	include <stdio.h>
#endif // USE_STDIO_H

#include <string>
#include <algorithm>
#include <windows.h>
#include "PlatformDef.h"

// <algorithm> provides replace()


class CVersionNumber
{
public:
	// ---------------------------------------------------------------
	// Only ASCII in Revision.
	CVersionNumber(int MainVersion, int SubVersion, int SubSubVersion, std::string Revision)
		: m_MainVersion(MainVersion), m_SubVersion(SubVersion), m_SubSubVersion(SubSubVersion)
	{
		/* if (Revision == "!") {
			// Windows-specific code
			char *cpAppFileName[MAX_DIR_CHARS];
			GetModuleFileName(NULL, cpAppFileName, MAX_DIR_CHARS);
			strftime(, "%Y%M%D-%H%M%S_(rev%s)");
			return;
		} */
		if (Revision.empty()) m_Revision = "";
		else m_Revision = "."+Revision;
		m_Revision.replace(m_Revision.begin(), m_Revision.end(), " ", "_");
	};
	// ---------------------------------------------------------------
	CVersionNumber(int MainVersion, int SubVersion, int SubSubVersion)
		: m_MainVersion(MainVersion), m_SubVersion(SubVersion), m_SubSubVersion(SubSubVersion), m_Revision ("")
	{};
	// ---------------------------------------------------------------
	CVersionNumber(const CVersionNumber& source)
		: m_MainVersion(source.m_MainVersion), m_SubVersion(source.m_SubVersion), m_SubSubVersion(source.m_SubSubVersion), m_Revision(source.m_Revision)
	{};
	// ---------------------------------------------------------------
	CVersionNumber(const std::string& sVersion)
		: m_MainVersion(0), m_SubVersion(0), m_SubSubVersion(0), m_Revision("")
	{
		size_t found1 = sVersion.find(".");
		m_MainVersion = atoi(sVersion.substr(0, found1).c_str());
		if (found1++ == std::string::npos) return;

		size_t found2 = sVersion.find(".", found1);
		m_SubVersion = atoi(sVersion.substr(found1, found2-found1).c_str());
		if (found2++ == std::string::npos) return;

		size_t found3 = sVersion.find(".", found2);
		m_SubSubVersion = atoi(sVersion.substr(found2, found3-found2).c_str());
		if (found3 == std::string::npos) return;

		m_Revision = sVersion.substr(found3);  // with the preceding separator
	};
	// ---------------------------------------------------------------
	CVersionNumber(const std::wstring& sVersion)
		: m_MainVersion(0), m_SubVersion(0), m_SubSubVersion(0), m_Revision("")
	{
		size_t found1 = sVersion.find(L".");
		m_MainVersion = _wtoi(sVersion.substr(0, found1).c_str());
		if (found1++ == std::string::npos) return;

		size_t found2 = sVersion.find(L".", found1);
		m_SubVersion = _wtoi(sVersion.substr(found1, found2-found1).c_str());
		if (found2++ == std::string::npos) return;

		size_t found3 = sVersion.find(L".", found2);
		m_SubSubVersion = _wtoi(sVersion.substr(found2, found3-found2).c_str());
		if (found3 == std::string::npos) return;

		std::wstring wsRev = sVersion.substr(found3); // with the preceding separator
		m_Revision.assign(wsRev.begin(), wsRev.end());
	};
	// ---------------------------------------------------------------
	int GetMainVersion()   const { return m_MainVersion;   };
	int GetSubVersion()    const { return m_SubVersion;    };
	int GetSubSubVersion() const { return m_SubSubVersion; };
	std::string GetRevision() const { return m_Revision.substr(m_Revision.empty()?0:1); };  // cutting the separator

	std::string AsString() const
	{
		char buffer[64];
		sprintf(buffer, "%d.%d.%d%s", m_MainVersion, m_SubVersion, m_SubSubVersion, m_Revision.c_str());
		return buffer;
	}

	std::string AsStringWithName() const
	{
		char buffer[64];
		sprintf(buffer, "gpsVP %d.%d.%d%s", m_MainVersion, m_SubVersion, m_SubSubVersion, m_Revision.c_str());
		return buffer;
	}
	std::wstring AswstringWithName() const
	{
		wchar_t buffer[128];
		wsprintf(buffer, L"gpsVP %d.%d.%d%S", m_MainVersion, m_SubVersion, m_SubSubVersion, m_Revision.c_str());
		return buffer;
	}
	// ---------------------------------------------------------------
	bool operator< (const CVersionNumber &other) const
	{
		if (m_MainVersion < other.m_MainVersion) 		return true;
		if (m_MainVersion > other.m_MainVersion)		return false;
		if (m_SubVersion < other.m_SubVersion)			return true;
		if (m_SubVersion > other.m_SubVersion)			return false;
		if (m_SubSubVersion < other.m_SubSubVersion)	return true;
		if (m_SubSubVersion > other.m_SubSubVersion)	return false;
		if (m_Revision < other.m_Revision)	return true;
		return false;
	};

	bool operator== (const CVersionNumber &other) const
	{
		return ((m_MainVersion == other.m_MainVersion) && 
			(m_SubVersion == other.m_SubVersion) && 
			(m_SubSubVersion == other.m_SubSubVersion) &&
			(m_Revision == other.m_Revision));
	};
	// ---------------------------------------------------------------

protected:
	int m_MainVersion, m_SubVersion, m_SubSubVersion;
	std::string m_Revision;
};

#endif // VERSIONNUMBER_H
