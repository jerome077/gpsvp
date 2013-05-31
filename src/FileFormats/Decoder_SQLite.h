/*	
Copyright (c) 2005-2008, Jerome077 and others
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DECODER_SQLITE_H
#define DECODER_SQLITE_H

#include <string>
#include <list>
#include <map>
#include "Decoder_Common.h"
#include "sqlite/sqlite3.h"
#include "../GoogleMaps/RasterServerSources.h"

class CSQLiteItemInfo: public CDecoderTileInfo
{
protected:
	int m_X;
	int m_Y;
	int m_Z0;
	std::wstring m_sqliteFilename;
	char* m_buf;
public:
	CSQLiteItemInfo(const std::wstring& sqliteFilename, int X, int Y, int Z0) : m_sqliteFilename(sqliteFilename), m_X(X), m_Y(Y), m_Z0(Z0),
	                                                                            m_buf(NULL) {};
	virtual HBITMAP LoadTile(HDC dc, CHBitmapBuilder* pHBitmapBuilder);
	virtual void DeleteTileIfPossible();
	virtual char * OpenTile(int& len);
	virtual void CloseTile();
};


class CDecoderSQLite
{
public:
	CDecoderSQLite(const std::wstring& filename);
	~CDecoderSQLite();
	bool IsFileOk() { return m_ok; };
	CSQLiteItemInfo* FindItem(int X, int Y, int Z0); // returns -1 when not found
	const void * OpenItem(int X, int Y, int Z0, size_t& len, sqlite3_stmt*& stmt);
	void CloseItem(sqlite3_stmt* stmt);

private:
	std::wstring m_sqliteFilename;
	sqlite3 *db;
	bool m_ok;
};

class CDecoderSQLitePool
{
public:
	CDecoderSQLitePool(int poolSize);
	~CDecoderSQLitePool();
	CDecoderSQLite* GetDecoder(const std::wstring& filename);

protected:
	typedef std::list<std::pair<std::wstring, CDecoderSQLite*> > TDecoderList;
	TDecoderList m_pool;
	int m_poolSize;
};

extern CDecoderSQLitePool M_DecoderSQLitePool;

#endif
