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
	// Метод для получения запроса для отрисовщика, которому не хватило фрагментов
	std::string GetRequestURL(const GEOFILE_DATA& data);
	// Получить запрос
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
	// Здесь - общий префикс для всех директорий
	std::wstring m_strMapsRoot;
	// Inited
	bool m_bInitialized;

	std::set< GEOFILE_DATA > m_setToDownload;

	// Имя файла, которое возвращается в случае отсутствия в базе необходимого фрагмента
	std::wstring m_strDefaultFileName;

	// Минимальный и максимальный доступный уровень (0..18 для карт)
	long m_nMinLevel, m_nMaxLevel; 

	// Текущий номер сервера
	CRasterMapSource * m_vecRMS[gtCount];

	unsigned char m_nMTServerId;
	unsigned char m_nKHServerId;

	DWORD m_wdLastRequestTicks;
};
