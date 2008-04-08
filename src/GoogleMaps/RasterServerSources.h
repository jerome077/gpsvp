#pragma once

#include <string>
#include <list>

enum enumGMapType
{
	gtNone,
	gtOsm,
	gtMap,
	gtSatellite,
	gtHybrid,
	gtMSMap,
	gtMSSat,
	gtMSHyb,
	gtCount
};

const long LEVEL_REVERSE_OFFSET = 18;

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

	// Возвращает URL для заданного data
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
public:
	CGMapSource();

	virtual std::string GetRequestURL(const GEOFILE_DATA& data)
	{
		char buffer[256];
		sprintf(buffer, "%sx=%d&y=%d&zoom=%d", GetNextPrefix().c_str(), data.X, data.Y, data.level);
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

protected:
	std::string GetSatelliteBlockName(const GEOFILE_DATA& data) const;
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
	std::string GetBlockName(const GEOFILE_DATA& data) const;
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
};

class COSMSource : public CRasterMapSource
{
public:
	COSMSource();
	virtual std::string GetRequestURL(const GEOFILE_DATA& data)
	{
		char buffer[256];
		sprintf(buffer, "http://tile.openstreetmap.org/%d/%d/%d.png", 17 - data.level, data.X, data.Y);
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

