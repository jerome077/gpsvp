#include <windows.h>
#include "../PlatformDef.h"
#include "../Lock.h"
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
