/*
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
#include <list>
#include "GMCommon.h"
#include "../VersionNumber.h"
#include "../GeoPoint.h"
#include "VariableInterpreter.h"

enum enumGMapType
{
	gtNone,
	gtOsm,
	gtMap,
	gtSatellite,
	gtTopo,
	gtMSMap,
	gtMSSat,
	gtMSHyb,
	gtNYandex, // Former gtHybrid
	//gtHybrid = gtNYandex,
	gtFirstWMSMapType,
	gtLastGMapType = 0x1000,
};

struct GEOFILE_DATA {
	GEOFILE_DATA(unsigned char t, unsigned char l, unsigned long x, unsigned long y) 
		: type(t), level(l), X(x), Y(y) {};
	GEOFILE_DATA() : type(gtNone) {};

	unsigned char level;
	unsigned char type; // it is enumGMapType
	long X : 24;
	long Y : 24;

	bool operator< (const GEOFILE_DATA &other) const
	{
		if (level < other.level)
			return true;
		if (level > other.level)
			return false;
		if (X < other.X)
			return true;
		if (X > other.X)
			return false;
		if (type < other.type)
			return true;
		if (type > other.type)
			return false;
		if (Y < other.Y)
			return true;
		return false;
	};

	bool operator== (const GEOFILE_DATA &other) const
	{
		return ((type == other.type) && (X == other.X) && (Y == other.Y) && (level == other.level));
	};
};

class CRasterMapSource
{
public:
	CRasterMapSource() : m_enMyType(gtNone) {};
	virtual ~CRasterMapSource() {};

	enumGMapType GetType() const { return m_enMyType; };
	void SetType(enumGMapType en) { m_enMyType = en; };

	// Return URL for given data
	virtual std::string GetRequestURL(const GEOFILE_DATA& data) = 0;

	virtual bool GetDiskFileName(
			const GEOFILE_DATA& gfdata, std::wstring &path, std::wstring &name, const std::wstring root
		) = 0;

	virtual bool IsGoodFileName(GEOFILE_DATA &data, const std::wstring &name) const = 0;

	virtual void SetServerPrefixes(std::list<std::string> &lstPrefixes)
	{
		m_lstPrefixes.erase(m_lstPrefixes.begin(), m_lstPrefixes.end());
		m_lstPrefixes.swap(lstPrefixes);
		m_it = m_lstPrefixes.begin();
	};

	virtual bool IsSatellite() const
	{
		return false;
	};

	virtual GeoPoint GetDemoPoint(double &scale) const
	{
		return GeoPoint(0, 0);
	};

protected:
	virtual std::string GetNextPrefix()
	{
		if (m_it == m_lstPrefixes.end()) {
			m_it = m_lstPrefixes.begin();
		}
		return *m_it++;
	};

	bool GetDiskGenericFileName(const GEOFILE_DATA& gfdata, const std::wstring& root, 
		std::wstring &path, const wchar_t *pwszMapType);

private:
	enumGMapType m_enMyType;
	std::list<std::string> m_lstPrefixes;
	std::list<std::string>::iterator m_it;
};

typedef CRasterMapSource* PRasterMapSource;

class CNullSource : public CRasterMapSource
{
	virtual std::string GetRequestURL(const GEOFILE_DATA& data) { return ""; };

	virtual bool GetDiskFileName(
			const GEOFILE_DATA& gfdata, std::wstring &path, std::wstring &name, const std::wstring root
		) 
	{ return false;	};

	virtual bool IsGoodFileName(GEOFILE_DATA &data, const std::wstring &name) const { return false; };
};

class CGMapSource : public CRasterMapSource
{
	char m_szHl[16];
public:
	CGMapSource();

	virtual std::string GetRequestURL(const GEOFILE_DATA& data)
	{
		char buffer[256];
		sprintf(buffer, "%sx=%ld&y=%ld&zoom=%d&hl=%s", GetNextPrefix().c_str(), data.X, data.Y, data.level, m_szHl);
		return buffer;
	};

	virtual bool GetDiskFileName(
			const GEOFILE_DATA& gfdata, std::wstring &path, std::wstring &name, const std::wstring root
		)
	{
		wchar_t filename[MAX_PATH];

		wsprintf(filename, L"x=%d&y=%d&zoom=%d.png", gfdata.X, gfdata.Y, gfdata.level);
		name = filename;

		return GetDiskGenericFileName(gfdata, root, path, L"gmap");
	};

	virtual bool IsGoodFileName(GEOFILE_DATA &data, const std::wstring &name) const;
};

class CGTopoSource : public CRasterMapSource
{
public:
	CGTopoSource();

	virtual std::string GetRequestURL(const GEOFILE_DATA& data)
	{
		char buffer[256];
		sprintf(buffer, "%sx=%ld&y=%ld&zoom=%d", GetNextPrefix().c_str(), data.X, data.Y, data.level);
		return buffer;
	};

	virtual bool GetDiskFileName(
			const GEOFILE_DATA& gfdata, std::wstring &path, std::wstring &name, const std::wstring root
		)
	{
		wchar_t filename[MAX_PATH];

		wsprintf(filename, L"x=%d&y=%d&zoom=%d.jpg", gfdata.X, gfdata.Y, gfdata.level);
		name = filename;

		return GetDiskGenericFileName(gfdata, root, path, L"gtopo");
	};

	virtual bool IsGoodFileName(GEOFILE_DATA &data, const std::wstring &name) const;
};

class CGSatSource : public CRasterMapSource
{
public:
	CGSatSource();

	virtual std::string GetRequestURL(const GEOFILE_DATA& data)
	{
		char buffer[256];
		sprintf(buffer, "%st=%s", GetNextPrefix().c_str(), GetSatelliteBlockName(data).c_str());
		return buffer;
	};

	virtual bool GetDiskFileName(
			const GEOFILE_DATA& gfdata, std::wstring &path, std::wstring &name, const std::wstring root
		)
	{
		wchar_t filename[MAX_PATH];

		wsprintf(filename, L"%S.jpg", GetSatelliteBlockName(gfdata).c_str());
		name = filename;
		// wsprintf(zoomname, L"level=%d", name.length() - 4); // 4 = length(.jpg)

		return GetDiskGenericFileName(gfdata, root, path, L"gsat");
	};

	virtual bool IsGoodFileName(GEOFILE_DATA &data, const std::wstring &name) const;

	virtual bool IsSatellite() const { return true; }

protected:
	std::string GetSatelliteBlockName(const GEOFILE_DATA& data) const
	{	return GoogleXYZ17toQRST(data.X, data.Y, data.level); };
};

class CMSSource : public CRasterMapSource
{
public:
	const virtual std::wstring& GetFileExtension() const = 0;
	const virtual std::wstring& GetMapType() const = 0;
	const virtual std::wstring& GetFilePrefix() const = 0;

	virtual std::string GetRequestURL(const GEOFILE_DATA& data)
	{
		char buffer[256];
		sprintf(buffer, "%s%S%s%S?g=45", GetNextPrefix().c_str(), GetMapType().c_str(), 
			GetBlockName(data).c_str(), GetFileExtension().c_str());
		return buffer;
	};

	virtual bool GetDiskFileName(
			const GEOFILE_DATA& gfdata, std::wstring &path, std::wstring &name, const std::wstring root
		)
	{
		wchar_t filename[MAX_PATH];

		wsprintf(filename, L"%s%S%s", GetMapType().c_str(), GetBlockName(gfdata).c_str(), GetFileExtension().c_str());
		name = filename;

		return GetDiskGenericFileName(gfdata, root, path, GetFilePrefix().c_str());
	};

	virtual bool IsGoodFileName(GEOFILE_DATA &data, const std::wstring &name) const;

protected:
	std::string GetBlockName(const GEOFILE_DATA& data) const
	{	return GoogleXYZ17toQKey(data.X, data.Y, data.level); };
};

class CMSMapSource : public CMSSource
{
public:
	CMSMapSource();

	const virtual std::wstring& GetFileExtension() const
	{
		const static std::wstring ext = L".png"; return ext;
	}
	const virtual std::wstring& GetMapType() const
	{
		const static std::wstring type = L"r"; return type;
	}
	const virtual std::wstring& GetFilePrefix() const
	{
		const static std::wstring prefix = L"msmap"; return prefix;
	}
};

class CMSSatSource : public CMSSource
{
public:
	CMSSatSource();

	const virtual std::wstring& GetFileExtension() const
	{
		const static std::wstring ext = L".jpeg"; return ext;
	}
	const virtual std::wstring& GetMapType() const
	{
		const static std::wstring type = L"a"; return type;
	}
	const virtual std::wstring& GetFilePrefix() const
	{
		const static std::wstring prefix = L"mssat"; return prefix;
	}
	virtual bool IsSatellite() const { return true; }
};

class CMSHybSource : public CMSSource
{
public:
	CMSHybSource();

	const virtual std::wstring& GetFileExtension() const
	{
		const static std::wstring ext = L".jpeg"; return ext;
	}
	const virtual std::wstring& GetMapType() const
	{
		const static std::wstring type = L"h"; return type;
	}
	const virtual std::wstring& GetFilePrefix() const
	{
		const static std::wstring prefix = L"mshyb"; return prefix;
	}
	virtual bool IsSatellite() const { return true; }
};

class COSMSource : public CRasterMapSource
{
public:
	COSMSource();
	virtual std::string GetRequestURL(const GEOFILE_DATA& data)
	{
		char buffer[256];
		sprintf(buffer, "http://tile.openstreetmap.org/%d/%ld/%ld.png", 17 - data.level, data.X, data.Y);
		return buffer;
	};

	virtual bool GetDiskFileName(
			const GEOFILE_DATA& gfdata, std::wstring &path, std::wstring &name, const std::wstring root
		)
	{
		wchar_t filename[MAX_PATH];

		wsprintf(filename, L"osm-x=%d&y=%d&zoom=%d.png", gfdata.X, gfdata.Y, gfdata.level);
		name = filename;

		return GetDiskGenericFileName(gfdata, root, path, L"osm");
	};

	virtual bool IsGoodFileName(GEOFILE_DATA &data, const std::wstring &name) const;
};

class CNYaSource : public CRasterMapSource
{
public:
	CNYaSource();
	virtual std::string GetRequestURL(const GEOFILE_DATA& data)
	{
		char buffer[256];
		sprintf(buffer, "http://wvec.maps.yandex.net/?x=%ld&y=%ld&z=%d", data.X, data.Y, 17 - data.level);
		return buffer;
	};

	virtual bool GetDiskFileName(
			const GEOFILE_DATA& gfdata, std::wstring &path, std::wstring &name, const std::wstring root
		)
	{
		wchar_t filename[MAX_PATH];

		wsprintf(filename, L"x=%d&y=%d&zoom=%d.png", gfdata.X, gfdata.Y, gfdata.level);
		name = filename;

		return GetDiskGenericFileName(gfdata, root, path, L"nya");
	};

	virtual bool IsGoodFileName(GEOFILE_DATA &data, const std::wstring &name) const;
};

// Properties for one level of zoom of a user defined Map:
class CUserMapZoomProp
{
public:
	CStringSchema URLSchema;
	CStringSchema FilenameSchema;
	CStringSchema SubpathSchema;
};
typedef std::map<std::wstring, CUserMapZoomProp> CUserMapZoomProp_MAP;


// User defined map type (mapcfg.ini)
class CUserWMSMapSource : public CRasterMapSource
{
private:
    std::wstring m_MapName;
    std::wstring m_CacheRoot;
	GeoPoint m_DemoPoint; // A point where the map should be visible
	int m_DemoPointZoomOne;

public:
	CUserWMSMapSource(long iMapType,
					  const std::wstring& mapName,
					  const std::wstring& configFile,
					  const std::wstring& cacheRoot,
					  const CVersionNumber& gpsVPVersion);
	virtual std::string GetRequestURL(const GEOFILE_DATA& data);
	std::wstring GetName() { return m_MapName; };

	virtual bool GetDiskFileName(
			const GEOFILE_DATA& gfdata, std::wstring &path, std::wstring &name, const std::wstring root
		);

	virtual bool IsGoodFileName(GEOFILE_DATA &data, const std::wstring &name) const;

	virtual GeoPoint GetDemoPoint(double &scale) const;

	enum enumConfigErrorType { cecOK,
		                       cecMapVersionNewerAsGpsVP,
						       cecError };
	enumConfigErrorType GetConfigError() { return m_ConfigErrorCode; };

private:
	// Default map properties:
	CUserMapZoomProp m_DefaultProps;
	// Complementary map properties (Key = Name as in the ini file, Value = corresponding CUserMapZoomProp):
	CUserMapZoomProp_MAP m_MapZoomProps;
	// Array of pointers on the map properties for each zoom level:
	CUserMapZoomProp* m_ZoomProps[LEVEL_REVERSE_OFFSET];
	// Access to the map properties:
	CUserMapZoomProp& GetZoomProps(long zoomSeventeen) // zoomSeventeen corresponds to 'data.level'
	{
		return *m_ZoomProps[17-zoomSeventeen];
	};
	enumConfigErrorType m_ConfigErrorCode;

};

