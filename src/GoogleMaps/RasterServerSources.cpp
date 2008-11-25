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
#include "../PlatformDef.h"
#include "../Lock.h"
#include "../SimpleIniExt.h"
#include "RasterServerSources.h"

#ifndef UNDER_CE
#	define WMKDIR(x) _wmkdir(x)
#else // UNDER_CE
#	define WMKDIR(x) CreateDirectory(x, NULL)
#endif // UNDER_CE

bool CRasterMapSource::GetDiskGenericFileName(const GEOFILE_DATA& gfdata, const std::wstring& root, 
	std::wstring &path, const wchar_t *pwszMapType)
{
	wchar_t dir[MAX_PATH];
	wchar_t zoomname[16];
	wsprintf(zoomname, L"level=%d", LEVEL_REVERSE_OFFSET - gfdata.level);

	if (root != L"") {
		wsprintf(dir, L"%s/%s", root.c_str(), pwszMapType);
		WMKDIR(dir);
		wsprintf(dir, L"%s/%s/%s", root.c_str(), pwszMapType, zoomname);
		WMKDIR(dir);
		wsprintf(dir, L"%s/%s/%s/x=%d", root.c_str(), pwszMapType, zoomname, gfdata.X);
		WMKDIR(dir);
	} else {
		wsprintf(dir, L"%s/%s/x=%d", pwszMapType, zoomname, gfdata.X);
	}
	path = dir;

	return true;
}

CGMapSource::CGMapSource()
{
	std::list<std::string> lst;
	lst.push_back("http://mt0.google.com/mt?");
	lst.push_back("http://mt1.google.com/mt?");
	lst.push_back("http://mt2.google.com/mt?");
	lst.push_back("http://mt3.google.com/mt?");
	SetServerPrefixes(lst);

	SetType(gtMap);
}

CGTopoSource::CGTopoSource()
{
	std::list<std::string> lst;
	lst.push_back("http://mt0.google.com/mt?n=404&v=w2p.81&hl=en&");
	lst.push_back("http://mt1.google.com/mt?n=404&v=w2p.81&hl=en&");
	lst.push_back("http://mt2.google.com/mt?n=404&v=w2p.81&hl=en&");
	lst.push_back("http://mt3.google.com/mt?n=404&v=w2p.81&hl=en&");
	SetServerPrefixes(lst);

	SetType(gtTopo);
}

CGSatSource::CGSatSource()
{
	std::list<std::string> lst;
	lst.push_back("http://kh0.google.com/kh?n=404&v=20&");
	lst.push_back("http://kh1.google.com/kh?n=404&v=20&");
	lst.push_back("http://kh2.google.com/kh?n=404&v=20&");
	lst.push_back("http://kh3.google.com/kh?n=404&v=20&");
	SetServerPrefixes(lst);

	SetType(gtSatellite);
}

CMSMapSource::CMSMapSource()
{
	std::list<std::string> lst;
	char buf[256];
	for (int i=0; i<4; i++) {
		sprintf(buf, "http://%S%d.ortho.tiles.virtualearth.net/tiles/", GetMapType().c_str(), i);
		lst.push_back(buf);
	}
	SetServerPrefixes(lst);

	SetType(gtMSMap);
}

CMSSatSource::CMSSatSource()
{
	std::list<std::string> lst;
	char buf[256];
	for (int i=0; i<4; i++) {
		sprintf(buf, "http://%S%d.ortho.tiles.virtualearth.net/tiles/", GetMapType().c_str(), i);
		lst.push_back(buf);
	}
	SetServerPrefixes(lst);

	SetType(gtMSSat);
}

CMSHybSource::CMSHybSource()
{
	std::list<std::string> lst;
	char buf[256];
	for (int i=0; i<4; i++) {
		sprintf(buf, "http://%S%d.ortho.tiles.virtualearth.net/tiles/", GetMapType().c_str(), i);
		lst.push_back(buf);
	}
	SetServerPrefixes(lst);

	SetType(gtMSHyb);
}

bool CGMapSource::IsGoodFileName(GEOFILE_DATA &data, const std::wstring &name) const
{
	std::string::size_type q = name.find(L".png");
	if (q == std::string::npos)
		return false;
	if (name.length() > (q + 4)) {
		return false;
	}

	long x, y, zoom;
	int nReadFields = swscanf(name.c_str(), L"x=%d&y=%d&zoom=%d.png", &x, &y, &zoom);
	if (nReadFields == 3) {
		data.level = (unsigned char) zoom;
		data.X = x;
		data.Y = y;
		data.type = gtMap;
		return true;
	} else {
		return false;
	}
}

bool CGTopoSource::IsGoodFileName(GEOFILE_DATA &data, const std::wstring &name) const
{
	std::string::size_type q = name.find(L".jpg");
	if (q == std::string::npos)
		return false;
	if (name.length() > (q + 4)) {
		return false;
	}

	long x, y, zoom;
	int nReadFields = swscanf(name.c_str(), L"x=%d&y=%d&zoom=%d.jpg", &x, &y, &zoom);
	if (nReadFields == 3) {
		data.level = (unsigned char) zoom;
		data.X = x;
		data.Y = y;
		data.type = gtTopo;
		return true;
	} else {
		return false;
	}
}

bool CGSatSource::IsGoodFileName(GEOFILE_DATA &data, const std::wstring &name) const
{
	long nLevel = 0;

	std::string::size_type q = name.find(L".jpg");
	if (q == std::string::npos)
		return false;
	if (name.length() > (q + 4)) {
		return false;
	}

	long X = 0, Y = 0;
	for (std::string::size_type i=1; i<q; i++) {
		X <<= 1;
		Y <<= 1;
		switch (name[i]) {
			case L'q':
				break;
			case L'r':
				X++;
				break;
			case L't':
				Y++;
				break;
			case L's':
				X++;
				Y++;
				break;
			default:
				return false;
				break;
		}
	}

	data.X = X;
	data.Y = Y;
	data.level = (unsigned char) (LEVEL_REVERSE_OFFSET - q);
	data.type = gtSatellite;

	return true;
};

std::string CGSatSource::GetSatelliteBlockName(const GEOFILE_DATA& data) const
{
	long NumX = data.X;
	long NumY = data.Y;
	long level = LEVEL_REVERSE_OFFSET - data.level;
	long d = 1 << (level - 1);

	if ((NumX < 0) || (NumX > (d-1))) {
		NumX = NumX % d;
		if (NumX < 0) {
			NumX += d;
		}
	}

	char buf[24];
	buf[0] = 't';

	for (long nPos = 1; nPos < level; nPos++) {
	    d >>= 1;
		if (NumY < d) {
			if (NumX < d) {
				buf[nPos] = 'q';
			} else {
				buf[nPos] = 'r';
				NumX -= d;
			}
		} else {
			if (NumX < d) {
				buf[nPos] = 't';
			} else { 
				buf[nPos] = 's';
				NumX -= d;
			}
			NumY -= d;
		}
	}
	buf[level] = '\0';
	return buf;
}

bool CMSSource::IsGoodFileName(GEOFILE_DATA &data, const std::wstring &name) const
{
	long nLevel = 0;

	const std::wstring &type = GetMapType();
	if (wcsncmp(type.c_str(), name.c_str(), type.length()) != 0) {
		return false;
	}

	std::wstring n = name.c_str() + 1;

	std::string::size_type q = n.find(L".");
	if (q == std::string::npos)
		return false;
	if (n.length() > (q + GetFileExtension().length())) {
		return false;
	}

	long X = 0, Y = 0;
	for (std::string::size_type i=0; i<q; i++) {
		X <<= 1;
		Y <<= 1;
		switch (n[i]) {
			case L'0':
				break;
			case L'1':
				X++;
				break;
			case L'2':
				Y++;
				break;
			case L'3':
				X++;
				Y++;
				break;
			default:
				return false;
				break;
		}
	}

	data.X = X;
	data.Y = Y;
	data.level = (unsigned char) (LEVEL_REVERSE_OFFSET - q - 1);
	data.type = GetType();

	return true;
};

std::string CMSSource::GetBlockName(const GEOFILE_DATA& data) const
{
	long NumX = data.X;
	long NumY = data.Y;
	long level = LEVEL_REVERSE_OFFSET - data.level;
	long d = 1 << (level - 1);

	if ((NumX < 0) || (NumX > (d-1))) {
		NumX = NumX % d;
		if (NumX < 0) {
			NumX += d;
		}
	}

	char buf[24];

	for (long nPos = 0; nPos < (level-1); nPos++) {
	    d >>= 1;
		if (NumY < d) {
			if (NumX < d) {
				buf[nPos] = '0';
			} else {
				buf[nPos] = '1';
				NumX -= d;
			}
		} else {
			if (NumX < d) {
				buf[nPos] = '2';
			} else { 
				buf[nPos] = '3';
				NumX -= d;
			}
			NumY -= d;
		}
	}
	buf[level-1] = '\0';
	return buf;
}


COSMSource::COSMSource()
{
	SetType(gtOsm);
}

bool COSMSource::IsGoodFileName(GEOFILE_DATA &data, const std::wstring &name) const
{
	std::string::size_type q = name.find(L".png");
	if (q == std::string::npos)
		return false;
	if (name.length() > (q + 4)) {
		return false;
	}

	long x, y, zoom;
	int nReadFields = swscanf(name.c_str(), L"osm-x=%d&y=%d&zoom=%d.png", &x, &y, &zoom);
	if (nReadFields == 3) {
		data.level = (unsigned char) zoom;
		data.X = x;
		data.Y = y;
		data.type = gtMap;
		return true;
	} else {
		return false;
	}
}

CUserWMSMapSource::CUserWMSMapSource(long iMapType,
					                 const std::wstring& mapName,
					                 const std::wstring& configFile,
									 const std::wstring& cacheRoot,
									 const CVersionNumber& gpsVPVersion)
	: m_MapName(mapName),
	  m_CacheRoot(cacheRoot),
	  m_ConfigErrorCode(cecOK),
	  m_DemoPoint(0, 0),
	  m_DemoPointZoomOne(14)
{
	SetType(enumGMapType(iMapType));

    CSimpleIniExtW iniFile;
    iniFile.LoadFile(configFile.c_str());

	if (iniFile.GetSectionSize(L"Tiled MAP") <= 0)
	{
		m_ConfigErrorCode = cecError;
		return;
	}
	CVersionNumber mapMinVersion(iniFile.GetValue(L"Tiled MAP", L"gpsVPVersionMin", L"" /*default*/));
	if (gpsVPVersion < mapMinVersion)
		m_ConfigErrorCode = cecMapVersionNewerAsGpsVP;

	m_DefaultProps.URLSchema = iniFile.GetValue(L"Tiled MAP", L"URL", L"" /*default*/);
	m_DefaultProps.FilenameSchema = iniFile.GetValue(L"Tiled MAP", L"Filename", L"" /*default*/);
	m_DefaultProps.SubpathSchema = iniFile.GetValue(L"Tiled MAP", L"Subpath", L"" /*default*/);
	for(int zoomOne=1;zoomOne<=LEVEL_REVERSE_OFFSET;zoomOne++)
	{
		wchar_t valueName[32];
		swprintf(valueName, 32, L"ZoomOne%d", zoomOne);
		std::wstring sMapForZoom = iniFile.GetValue(L"Tiled MAP", valueName, L"" /*default*/);
		if (sMapForZoom.empty())
		{
			m_ZoomProps[zoomOne-1] = &m_DefaultProps;
		}
		else
		{
			CUserMapZoomProp_MAP::iterator it = m_MapZoomProps.find(sMapForZoom);
		    if (it != m_MapZoomProps.end())
			{
				m_ZoomProps[zoomOne-1] = &(it->second);
			}
			else
			{
				CUserMapZoomProp& zoomProps = m_MapZoomProps[sMapForZoom];
				zoomProps.URLSchema = iniFile.GetValue(sMapForZoom.c_str(), L"URL", L"" /*default*/);
				zoomProps.FilenameSchema = iniFile.GetValue(sMapForZoom.c_str(), L"Filename", L"" /*default*/);
				zoomProps.SubpathSchema = iniFile.GetValue(sMapForZoom.c_str(), L"Subpath", L"" /*default*/);
				m_ZoomProps[zoomOne-1] = &zoomProps;
			}
		}
	}//for zoomOne

	double DemoPointLon = iniFile.GetDoubleValue(L"Tiled MAP", L"DemoPointLon", L"0" /*default*/);
	double DemoPointLat = iniFile.GetDoubleValue(L"Tiled MAP", L"DemoPointLat", L"0" /*default*/);
	m_DemoPointZoomOne = iniFile.GetIntValue(L"Tiled MAP", L"DemoPointZoomOne", L"14" /*default*/);
	m_DemoPoint = GeoPoint(DemoPointLon, DemoPointLat);
}

bool CUserWMSMapSource::IsGoodFileName(GEOFILE_DATA &data, const std::wstring &name) const
{
    return false;
}

void ReplaceSubStr(std::wstring& AString,
				   const std::wstring& oldSub,
				   const std::wstring& newSub)
{
	size_t found = AString.find(oldSub);
	while (std::wstring::npos != found)
	{
		AString.replace(found, oldSub.length(), newSub);
		found = AString.find(oldSub, found);
	}
}

void CUserWMSMapSource::ReplaceSchemaSubStrings(const GEOFILE_DATA& data, std::wstring& AString)
{
	wchar_t buffer[32];
	double Long1 = XtoLong(data.X,data.level);
	double Lat1 = YtoLat(data.Y+1,data.level);
	double Long2 = XtoLong(data.X+1,data.level);
	double Lat2 = YtoLat(data.Y,data.level);

	swprintf(buffer, 32, L"%.14f", Long1);
	ReplaceSubStr(AString, L"%LONG1", buffer);

	swprintf(buffer, 32, L"%.14f", Lat1);
	ReplaceSubStr(AString, L"%LAT1", buffer);

	swprintf(buffer, 32, L"%.14f", Long2);
	ReplaceSubStr(AString, L"%LONG2", buffer);

	swprintf(buffer, 32, L"%.14f", Lat2);
	ReplaceSubStr(AString, L"%LAT2", buffer);

	swprintf(buffer, 32, L"%d", data.X);
	ReplaceSubStr(AString, L"%X", buffer);

	swprintf(buffer, 32, L"%d", data.Y);
	ReplaceSubStr(AString, L"%Y", buffer);

	swprintf(buffer, 32, L"%d", data.level);
	ReplaceSubStr(AString, L"%ZOOM_17", buffer);  // 17 = Whole Earth in 1 tile

	swprintf(buffer, 32, L"%d", LEVEL_REVERSE_OFFSET-data.level);
	ReplaceSubStr(AString, L"%ZOOM_01", buffer);  //  1 = Whole Earth in 1 tile

	swprintf(buffer, 32, L"%d", 17-data.level);
	ReplaceSubStr(AString, L"%ZOOM_00", buffer);  //  0 = Whole Earth in 1 tile
}


std::string CUserWMSMapSource::GetRequestURL(const GEOFILE_DATA& data)
{
	CUserMapZoomProp& zoomProps = GetZoomProps(data.level);
	std::wstring wstrUrl = zoomProps.URLSchema;
    ReplaceSchemaSubStrings(data, wstrUrl);
    std::string strUrl;
    strUrl.assign(wstrUrl.begin(), wstrUrl.end());
	//sprintf(buffer, "http://tile.openstreetmap.org/%d/%d/%d.png", 17 - data.level, data.X, data.Y);
	return strUrl;
}

bool CUserWMSMapSource::GetDiskFileName(
		const GEOFILE_DATA& gfdata, std::wstring &path, std::wstring &name, const std::wstring root
	)
{
	CUserMapZoomProp& zoomProps = GetZoomProps(gfdata.level);
	name = zoomProps.FilenameSchema;
	ReplaceSchemaSubStrings(gfdata, name);

	if (!zoomProps.SubpathSchema.empty())
	{
		path = zoomProps.SubpathSchema;
        ReplaceSchemaSubStrings(gfdata, path);
		path = m_MapName + L"/" + path;
		if (!root.empty())
			path = root + L"/" + path;
		return true;
	}
	else
		return GetDiskGenericFileName(gfdata, root, path, m_MapName.c_str());
}

GeoPoint CUserWMSMapSource::GetDemoPoint(double &scale) const
{
	// I don't exactely how to calculate the "scale". I've just tried and
	// found out that the following works:
	scale = pow(2.0, 16.0-m_DemoPointZoomOne);
	return m_DemoPoint;
};
