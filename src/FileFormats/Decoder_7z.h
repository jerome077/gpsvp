/*	
Copyright (c) 2005-2008, Jerome077 and others
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DECODER_7Z_H
#define DECODER_7Z_H

#include <string>
#include <list>
#include "LZMA_SDK/types.h"
#include "LZMA_SDK/7zFile.h"
#include "LZMA_SDK/7z.h"

class CDecoder7z
{
public:
	CDecoder7z(const std::wstring& filename);
	~CDecoder7z();
	bool IsFileOk() { return m_ok; };
	int FindItem(const std::wstring& itemWithPath); // returns -1 when not found
	UInt64 GetItemSize(int itemIndex);
	bool UnzipItem(int itemIndex, void *buffer, UInt64 len);

private:
	CFileInStream m_archiveStream;
	CLookToRead m_lookStream;
	CSzArEx m_db;
	ISzAlloc m_allocImp;
	ISzAlloc m_allocTempImp;
	bool m_ok;
};

class CDecoder7zPool
{
public:
	CDecoder7zPool(int poolSize);
	~CDecoder7zPool();
	CDecoder7z* GetDecoder(const std::wstring& filename);

protected:
	typedef std::list<std::pair<std::wstring, CDecoder7z*> > TDecoderList;
	TDecoderList m_pool;
	int m_poolSize;
};

extern CDecoder7zPool M_Decoder7zPool;

#endif
