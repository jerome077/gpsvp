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


CDecoderSQLite::CDecoderSQLite(const std::wstring& filename)
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

CDecoderSQLite::~CDecoderSQLite()
{
	if (m_ok)
	{
		sqlite3_close(db);
	}
}

CSQLiteItemInfo* CDecoderSQLite::FindItem(int X, int Y, int Z0) // Finally not Z0 but Z17
{
	char sqlstr[100];
	sprintf(sqlstr, "SELECT image FROM tiles WHERE x = %d AND y = %d AND z = %d", X, Y, Z0);
	sqlite3_stmt *stmt;
	sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
    bool found = (sqlite3_step(stmt) == SQLITE_ROW);
	sqlite3_finalize(stmt);
	if (found)
		return new CSQLiteItemInfo(m_sqliteFilename, X, Y, Z0);
	else
		return NULL;
}

const void * CDecoderSQLite::OpenItem(int X, int Y, int Z0, size_t& len, sqlite3_stmt*& stmt)
{
	char sqlstr[100];
	sprintf(sqlstr, "SELECT image FROM tiles WHERE x = %d AND y = %d AND z = %d", X, Y, Z0);
	sqlite3_prepare(db, sqlstr, strlen(sqlstr), &stmt, NULL);
    if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		len = sqlite3_column_bytes(stmt, 0);
		return sqlite3_column_blob(stmt, 0);
	}
	else
		return NULL;
}

void CDecoderSQLite::CloseItem(sqlite3_stmt* stmt)
{
	sqlite3_finalize(stmt);
}

HBITMAP CSQLiteItemInfo::LoadTile(HDC dc, CHBitmapBuilder* pHBitmapBuilder)
{
	HBITMAP hbm = NULL;

	CDecoderSQLite* pDec = M_DecoderSQLitePool.GetDecoder(m_sqliteFilename);
	if (!pDec->IsFileOk()) return NULL;
	sqlite3_stmt *stmt = NULL;
	size_t len = 0;
	char *buf1 = (char *)pDec->OpenItem(m_X, m_Y, m_Z0, len, stmt);
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
	char * buf1 = (char *)pDec->OpenItem(m_X, m_Y, m_Z0, size, stmt);
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

CDecoderSQLitePool::CDecoderSQLitePool(int poolSize)
	: m_poolSize(poolSize)
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
}

CDecoderSQLite* CDecoderSQLitePool::GetDecoder(const std::wstring& filename)
{
	TDecoderList::iterator it;
	for (it = m_pool.begin(); it != m_pool.end(); ++it)
	{
		if (it->first == filename)
			return it->second;
	}

	// Not found? => New decoder
	CDecoderSQLite* pDecoder = new CDecoderSQLite(filename);
	if (m_pool.size() >= m_poolSize)
	{
		CDecoderSQLite* pDecoderToRemove = m_pool.back().second;
		m_pool.pop_back();
		delete pDecoderToRemove;
	}
	m_pool.push_front(std::make_pair(filename, pDecoder));
	return pDecoder;
}

CDecoderSQLitePool M_DecoderSQLitePool(4);
