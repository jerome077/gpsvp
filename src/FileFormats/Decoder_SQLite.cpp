/*	
Copyright (c) 2005-2008, Jerome077 and others
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Decoder_SQLite.h"

#include <stdio.h>
#include <string.h>

#include "sqlite/sqlite3.h"


// ---------------------------------------------------------------------------------------

CSimpleDecoderSQLite::CSimpleDecoderSQLite(const std::wstring& filename)
	: m_sqliteFilename(filename)
{
	m_ok = true;
	int rc;
	rc = sqlite3_open16(filename.c_str(), &db);
	if (rc)
	{
		m_ok = false;
		return;
	}
}

CSimpleDecoderSQLite::~CSimpleDecoderSQLite()
{
	if (m_ok)
	{
		sqlite3_close(db);
	}
}

CSQLiteItemInfo* CSimpleDecoderSQLite::FindItem(int X, int Y, int Z17)
{
	if (!m_ok) return NULL;
	char sqlstr[100];
	sprintf(sqlstr, "SELECT image FROM tiles WHERE x = %d AND y = %d AND z = %d", X, Y, Z17);
	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(db, sqlstr, -1, &stmt, NULL);
    bool found = (sqlite3_step(stmt) == SQLITE_ROW);
	sqlite3_finalize(stmt);
	if (found)
		return new CSQLiteItemInfo(m_sqliteFilename, X, Y, Z17);
	else
		return NULL;
}

const void * CSimpleDecoderSQLite::OpenItem(int X, int Y, int Z17, size_t& len, sqlite3_stmt*& stmt)
{
	char sqlstr[100];
	sprintf(sqlstr, "SELECT image FROM tiles WHERE x = %d AND y = %d AND z = %d", X, Y, Z17);
	sqlite3_prepare_v2(db, sqlstr, -1, &stmt, NULL);
    if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		len = sqlite3_column_bytes(stmt, 0);
		return sqlite3_column_blob(stmt, 0);
	}
	else
		return NULL;
}

void CSimpleDecoderSQLite::CloseItem(sqlite3_stmt* stmt)
{
	sqlite3_finalize(stmt);
}

// ---------------------------------------------------------------------------------------

CMultiDecoderSQLite::CSQLiteFileItem::CSQLiteFileItem(const std::wstring& strFileName)
	: filename(strFileName),
	  XminAtZmax(INT_MAX), XmaxAtZmax(INT_MIN),
	  YminAtZmax(INT_MAX), YmaxAtZmax(INT_MIN),
	  Z17min(INT_MAX), Z17max(INT_MIN)
{
	int rc;
	sqlite3 *db;
	rc = sqlite3_open16(filename.c_str(), &db);
	if (!rc)
	{
		sqlite3_stmt *stmt;
		sqlite3_prepare_v2(db, "SELECT MIN(z), MAX(z) FROM tiles", -1, &stmt, NULL);
		if (sqlite3_step(stmt) == SQLITE_ROW)
		{
			Z17min = sqlite3_column_int(stmt, 0);
			Z17max = sqlite3_column_int(stmt, 1);
		}
		sqlite3_finalize(stmt);
		for(int Z17=Z17min; Z17<=Z17max; Z17++)
		{
			char sqlstr[100];
			sprintf(sqlstr, "SELECT MIN(x), MAX(x), MIN(y), MAX(y) FROM tiles WHERE z=%d", Z17);
			sqlite3_prepare_v2(db, sqlstr, -1, &stmt, NULL);
			if (sqlite3_step(stmt) == SQLITE_ROW)
			{
				XminAtZmax = mymin(XminAtZmax, sqlite3_column_int(stmt, 0) >> (Z17max-Z17));
				XmaxAtZmax = mymax(XmaxAtZmax, sqlite3_column_int(stmt, 1) >> (Z17max-Z17));
				YminAtZmax = mymin(YminAtZmax, sqlite3_column_int(stmt, 2) >> (Z17max-Z17));
				YmaxAtZmax = mymax(YmaxAtZmax, sqlite3_column_int(stmt, 3) >> (Z17max-Z17));
			}
			sqlite3_finalize(stmt);
		}
	}
	sqlite3_close(db);
}

bool CMultiDecoderSQLite::CSQLiteFileItem::IsInRange(int X, int Y, int Z17)
{
	if ((Z17 < Z17min) || (Z17max < Z17))
		return false;
	int XAtZmax = X >> (Z17max-Z17);
	int YAtZmax = Y >> (Z17max-Z17);
	return ((XminAtZmax <= XAtZmax) && (XAtZmax <= XmaxAtZmax) && (YminAtZmax <= YAtZmax) && (YAtZmax <= YmaxAtZmax));
}

CMultiDecoderSQLite::CMultiDecoderSQLite(const std::wstring& filenameWithStar)
	: m_list()
{
	m_ok = true;
	WIN32_FIND_DATA FindFileData;

	// Find all .sqlitedb files collections in subfolfers of the cache folder:
	std::wstring strFoldername = filenameWithStar.substr(0, filenameWithStar.length()-10);
	std::wstring strSearch = filenameWithStar;
	HANDLE hFind = FindFirstFile(strSearch.c_str(), &FindFileData);
	if (INVALID_HANDLE_VALUE != hFind)
	{
		do
		{
			m_list.push_back(CSQLiteFileItem(strFoldername + FindFileData.cFileName));
		}
		while (FindNextFile(hFind, &FindFileData));
		FindClose(hFind);
	}
}

CMultiDecoderSQLite::~CMultiDecoderSQLite()
{
}

CSQLiteItemInfo* CMultiDecoderSQLite::FindItem(int X, int Y, int Z17)
{
	if (!m_ok) return NULL;
	CSQLiteItemInfo* pResult = NULL;
	for (SQLiteFile_iterator iter = m_list.begin(); iter != m_list.end(); ++iter)
	{
		if (iter->IsInRange(X, Y, Z17))
		{
			CDecoderSQLite* pDecSQlite = M_DecoderSQLitePool.GetDecoder(iter->filename);
			pResult = pDecSQlite->FindItem(X, Y, Z17);
			if (NULL != pResult) break;
		}
	}
	return pResult;
}

const void * CMultiDecoderSQLite::OpenItem(int X, int Y, int Z17, size_t& len, sqlite3_stmt*& stmt)
{
	if (!m_ok) return NULL;
	const void* pResult = NULL;
	for (SQLiteFile_iterator iter = m_list.begin(); iter != m_list.end(); ++iter)
	{
		if (iter->IsInRange(X, Y, Z17))
		{
			CDecoderSQLite* pDecSQlite = M_DecoderSQLitePool.GetDecoder(iter->filename);
			pResult = pDecSQlite->OpenItem(X, Y, Z17, len, stmt);
			if (NULL != pResult) break;
		}
	}
	return pResult;
}

void CMultiDecoderSQLite::CloseItem(sqlite3_stmt* stmt)
{
	sqlite3_finalize(stmt);
}

// ---------------------------------------------------------------------------------------

HBITMAP CSQLiteItemInfo::LoadTile(HDC dc, CHBitmapBuilder* pHBitmapBuilder)
{
	HBITMAP hbm = NULL;

	CDecoderSQLite* pDec = M_DecoderSQLitePool.GetDecoder(m_sqliteFilename);
	if (!pDec->IsFileOk()) return NULL;
	sqlite3_stmt *stmt = NULL;
	size_t len = 0;
	char *buf1 = (char *)pDec->OpenItem(m_X, m_Y, m_Z17, len, stmt);
	char *buffer = (char *)GlobalAlloc(GMEM_FIXED, len);
	memcpy(buffer, buf1, len);
	pDec->CloseItem(stmt);

	hbm = pHBitmapBuilder->BufferToHBitmap(buffer, len, dc);
	GlobalFree(buffer);

	return hbm;
}

void CSQLiteItemInfo::DeleteTileIfPossible()
{
	// Don't try do delete anything out of a sqlite file => do nothing
}

char * CSQLiteItemInfo::OpenTile(int& len)
{
	CDecoderSQLite *pDec = M_DecoderSQLitePool.GetDecoder(m_sqliteFilename);
	if (!pDec->IsFileOk()) return NULL;
	sqlite3_stmt *stmt = NULL;
	size_t size = 0;
	char * buf1 = (char *)pDec->OpenItem(m_X, m_Y, m_Z17, size, stmt);
	m_buf = (char *)GlobalAlloc(GMEM_FIXED, size);
	len = size;
	memcpy(m_buf, buf1, len);
	pDec->CloseItem(stmt);
	return m_buf;
}

void CSQLiteItemInfo::CloseTile()
{
	GlobalFree(m_buf);
	m_buf = NULL;
}

// ---------------------------------------------------------------------------------------

CDecoderSQLitePool::CDecoderSQLitePool(int poolSize)
	: m_poolSize(poolSize),
	  m_cachedMultiFilename(L""),
	  m_cachedMultiDecoder(NULL)
{
}

CDecoderSQLitePool::~CDecoderSQLitePool()
{
	TDecoderList::iterator it;
	for (it = m_pool.begin(); it != m_pool.end(); ++it)
	{
		CDecoderSQLite* pDecoder = it->second;
		delete pDecoder;
	}
	if (NULL != m_cachedMultiDecoder) delete m_cachedMultiDecoder;
}

CDecoderSQLite* CDecoderSQLitePool::GetDecoder(const std::wstring& filename)
{
	CDecoderSQLite* pDecoder;
	if (std::wstring::npos != filename.find(L"*.sqlitedb")) // With "*" => MultiDecoder
	{
		if (filename == m_cachedMultiFilename)
		{
			pDecoder = m_cachedMultiDecoder;
		}
		else
		{
			pDecoder = new CMultiDecoderSQLite(filename);
			if (NULL != m_cachedMultiDecoder) delete m_cachedMultiDecoder;
			m_cachedMultiDecoder = pDecoder;
			m_cachedMultiFilename = filename;
		}
	}
	else
	{
		TDecoderList::iterator it;
		for (it = m_pool.begin(); it != m_pool.end(); ++it)
		{
			if (it->first == filename)
				return it->second;
		}

		// Not found? => New decoder
		pDecoder = new CSimpleDecoderSQLite(filename);
		if (m_pool.size() >= m_poolSize)
		{
			CDecoderSQLite* pDecoderToRemove = m_pool.back().second;
			m_pool.pop_back();
			delete pDecoderToRemove;
		}
		m_pool.push_front(std::make_pair(filename, pDecoder));
	}
	return pDecoder;
}

CDecoderSQLitePool M_DecoderSQLitePool(4);

// ---------------------------------------------------------------------------------------

#ifndef UNDER_CE
CEncoderSQLite::CEncoderSQLite(const std::wstring& filename)
	: m_sqliteFilename(filename),
	  m_stmt(NULL)
{
	m_ok = true;
	int rc;
	rc = sqlite3_open16(filename.c_str(), &db);
	if (rc)
	{
		m_ok = false;
		return;
	}

    sqlite3_exec(db, "BEGIN", NULL, NULL, NULL);
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS tiles (x int, y int, z int, s int, image blob, PRIMARY KEY (x,y,z,s))", NULL, NULL, NULL);
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS info (minzoom int, maxzoom int, url text)", NULL, NULL, NULL);
	sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS android_metadata (locale TEXT)", NULL, NULL, NULL);
	sqlite3_exec(db, "INSERT OR IGNORE INTO android_metadata (rowid, locale) VALUES (1, 'en_US')", NULL, NULL, NULL);
	// Drop index to be able to insert new tiles quickly:
    sqlite3_exec(db, "drop index IND if exists", NULL, NULL, NULL);
}

CEncoderSQLite::~CEncoderSQLite()
{
	if (m_ok)
	{
	    sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);
		sqlite3_close(db);
	}
}

bool CEncoderSQLite::AddTile(const GEOFILE_DATA& data, CDecoderTileInfo*& itemInfo)
{
	int rc;
	char sqlstr[100];
	sprintf(sqlstr, "INSERT INTO tiles (x, y, z, s, image) VALUES (%d, %d, %d, 0, ?)", data.X, data.Y, data.level);
    rc = sqlite3_prepare_v2(db, sqlstr, -1, &m_stmt, NULL);
    bool ok = (rc == SQLITE_OK);
	if (ok)
	{
		int len;
		char * buf = itemInfo->OpenTile(len);
		rc = sqlite3_bind_blob(m_stmt, 1, buf, len, SQLITE_TRANSIENT);
		bool ok = ((rc = sqlite3_step(m_stmt)) == SQLITE_DONE);
		sqlite3_finalize(m_stmt);
		itemInfo->CloseTile();
    }
	return ok;
}

void CEncoderSQLite::UpdateGlobalInfo()
{
	// Regenerate Index to optimize access on big tables: 
    sqlite3_exec(db, "create index IND on tiles (x,y,z,s)", NULL, NULL, NULL);

	// Get current info:
	int Zmin = 0;
	int Zmax = 0;
	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(db, "SELECT MIN(z), MAX(z) FROM tiles", -1, &stmt, NULL);
	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		Zmin = sqlite3_column_int(stmt, 0);
		Zmax = sqlite3_column_int(stmt, 1);
	}
	sqlite3_finalize(stmt);

	// Write current info:
	char sqlstr[100];
	sprintf(sqlstr, "INSERT OR REPLACE INTO info (rowid, minzoom, maxzoom) VALUES (1, %d, %d)", Zmin, Zmax);
    sqlite3_exec(db, sqlstr, NULL, NULL, NULL);
}
#endif

// ---------------------------------------------------------------------------------------
