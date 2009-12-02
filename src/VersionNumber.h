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
#ifndef LINUX
#	include <windows.h>
#endif
#include "PlatformDef.h"


class CVersionNumber
{
public:
    // ---------------------------------------------------------------
	CVersionNumber(int MainVersion, int SubVersion, int SubSubVersion)
		: m_MainVersion(MainVersion), m_SubVersion(SubVersion), m_SubSubVersion(SubSubVersion) 
	{};
    // ---------------------------------------------------------------
	CVersionNumber(const CVersionNumber& source)
		: m_MainVersion(source.m_MainVersion), m_SubVersion(source.m_SubVersion), m_SubSubVersion(source.m_SubSubVersion) 
	{};
    // ---------------------------------------------------------------
#ifndef LINUX
	CVersionNumber(const std::string& sVersion)
		: m_MainVersion(0), m_SubVersion(0), m_SubSubVersion(0) 
	{
		size_t found1 = sVersion.find(".");
		if (std::string::npos != found1)
		{
			m_MainVersion = atoi(sVersion.substr(0, found1).c_str());
			size_t found2 = sVersion.find(".", found1+1);
			if (std::string::npos != found2)
			{
				m_SubVersion = atoi(sVersion.substr(found1+1, found2-found1-1).c_str());
				m_SubSubVersion = atoi(sVersion.substr(found2+1, sVersion.length()).c_str());
			}
			else
				m_SubVersion = atoi(sVersion.substr(found1+1, sVersion.length()).c_str());
		}
		else
			m_MainVersion = atoi(sVersion.c_str());
	};
#endif
    // ---------------------------------------------------------------
	CVersionNumber(const std::tstring& sVersion)
		: m_MainVersion(0), m_SubVersion(0), m_SubSubVersion(0) 
	{
		size_t found1 = sVersion.find(L("."));
		if (std::string::npos != found1)
		{
			m_MainVersion = _wtoi(sVersion.substr(0, found1).c_str());
			size_t found2 = sVersion.find(L("."), found1+1);
			if (std::string::npos != found2)
			{
				m_SubVersion = _wtoi(sVersion.substr(found1+1, found2-found1-1).c_str());
				m_SubSubVersion = _wtoi(sVersion.substr(found2+1, sVersion.length()).c_str());
			}
			else
				m_SubVersion = _wtoi(sVersion.substr(found1+1, sVersion.length()).c_str());
		}
		else
			m_MainVersion = _wtoi(sVersion.c_str());
	};
    // ---------------------------------------------------------------
	int GetMainVersion()   const { return m_MainVersion;   };
	int GetSubVersion()    const { return m_SubVersion;    };
	int GetSubSubVersion() const { return m_SubSubVersion; };

	std::string AsString() const
	{
		char buffer[64];
	    sprintf(buffer, "%d.%d.%d", m_MainVersion, m_SubVersion, m_SubSubVersion);
		return buffer;
    }

	std::string AsStringWithName() const
	{
		char buffer[64];
	    sprintf(buffer, "gpsVP %d.%d.%d", m_MainVersion, m_SubVersion, m_SubSubVersion);
		return buffer;
    }
	std::tstring AswstringWithName() const
	{
		tchar_t buffer[128];
	    stprintf(buffer, 128, L("gpsVP %d.%d.%d"), m_MainVersion, m_SubVersion, m_SubSubVersion);
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
		return false;
	};

	bool operator== (const CVersionNumber &other) const
	{
		return ((m_MainVersion == other.m_MainVersion) && (m_SubVersion == other.m_SubVersion) && (m_SubSubVersion == other.m_SubSubVersion));
	};
    // ---------------------------------------------------------------

protected:
	int m_MainVersion, m_SubVersion, m_SubSubVersion;
};

#endif // VERSIONNUMBER_H
