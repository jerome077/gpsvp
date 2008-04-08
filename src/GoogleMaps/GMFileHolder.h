#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include "../Track.h"
#include "RasterServerSources.h"

class CGMFileHolder
{
public:
	CGMFileHolder(void);
	virtual ~CGMFileHolder(void);

	void SetDefaultFileName(const wchar_t * pszDefaultFile)
	{
		m_strDefaultFileName = pszDefaultFile;
	};

	long InitFromDir(const wchar_t *pszRoot, bool bCreateIndexIfNeeded = true);
	void Deinit();

	const long GetFileName(std::wstring& name, const GEOFILE_DATA& data) const;
	// ����� ��� ��������� ������� ��� �����������, �������� �� ������� ����������
	std::string GetRequestURL(const GEOFILE_DATA& data);
	// �������� ������
	bool GetQueuedData(GEOFILE_DATA* pData);
	long OnRequestProcessed(const std::string request, GEOFILE_DATA& gfdata, const char * data, int size);
	long ProcessPrefixes(const std::string &s);

	size_t GetDownloadQueueSize();
	long AddFileToDownload(const GEOFILE_DATA& data);
	bool IsFileInCache(const GEOFILE_DATA& data);

	long GetMaxLevel() const { return m_nMaxLevel; };
	long GetMinLevel() const { return m_nMinLevel; };
	bool empty() { return false; };

	HANDLE RelocateFiles(HANDLE h, long nMaxMSec = INFINITE); 
	bool NeedRelocateFiles(); 

protected:
	long BuildInternalIndex();

	bool RelocateFilesInDir(std::wstring wstrCurPath, std::wstring wstrPartPath);
	bool DeleteDirIfEmpty(std::wstring sDir, bool bDeleteThis = true);

	bool GetDiskFileName(const GEOFILE_DATA& gfdata, std::wstring &path, std::wstring &name, const std::wstring root = L"") const;

private:
	// ����� - ����� ������� ��� ���� ����������
	std::wstring m_strMapsRoot;
	// Inited
	bool m_bInitialized;

	std::set< GEOFILE_DATA > m_setToDownload;

	// ��� �����, ������� ������������ � ������ ���������� � ���� ������������ ���������
	std::wstring m_strDefaultFileName;

	// ����������� � ������������ ��������� ������� (0..18 ��� ����)
	long m_nMinLevel, m_nMaxLevel; 

	// ������� ����� �������
	CRasterMapSource * m_vecRMS[gtCount];

	unsigned char m_nMTServerId;
	unsigned char m_nKHServerId;

	DWORD m_wdLastRequestTicks;
};
