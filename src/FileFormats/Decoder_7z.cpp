/* 7zMain.c - Test application for 7z Decoder
2010-03-12 : Igor Pavlov : Public domain */

#include "Decoder_7z.h"

#include <stdio.h>
#include <string.h>

#include "LZMA_SDK/7z.h"
#include "LZMA_SDK/7zCrc.h"
#include "LZMA_SDK/7zFile.h"
#include "LZMA_SDK/7zVersion.h"

#include "LZMA_SDK/7zAlloc.h"

#ifndef USE_WINDOWS_FILE
/* for mkdir */
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <errno.h>
#endif
#endif

#ifdef _WIN32
#define CHAR_PATH_SEPARATOR '\\'
#else
#define CHAR_PATH_SEPARATOR '/'
#endif


static ISzAlloc g_Alloc = { SzAlloc, SzFree };

static int Buf_EnsureSize(CBuf *dest, size_t size)
{
  if (dest->size >= size)
    return 1;
  Buf_Free(dest, &g_Alloc);
  return Buf_Create(dest, size, &g_Alloc);
}


CDecoder7z::CDecoder7z(const std::wstring& filename)
	: m_filename(filename)
{
	m_ok = true;
	m_allocImp.Alloc = SzAlloc;
	m_allocImp.Free = SzFree;
	m_allocTempImp.Alloc = SzAllocTemp;
	m_allocTempImp.Free = SzFreeTemp;

	if (InFile_OpenW(&m_archiveStream.file, filename.c_str()))
	{
		m_ok = false;
		return;
	}

	FileInStream_CreateVTable(&m_archiveStream);
	LookToRead_CreateVTable(&m_lookStream, False);
  
	m_lookStream.realStream = &m_archiveStream.s;
	LookToRead_Init(&m_lookStream);

	CrcGenerateTable();

	SzArEx_Init(&m_db);
	SRes res = SzArEx_Open(&m_db, &m_lookStream.s, &m_allocImp, &m_allocTempImp);
	if (SZ_OK != res)
	{
		SzArEx_Free(&m_db, &m_allocImp);
		m_ok = false;
		return;
	}
}

CDecoder7z::~CDecoder7z()
{
	if (m_ok)
	{
		SzArEx_Free(&m_db, &m_allocImp);
		File_Close(&m_archiveStream.file);
	}
}

C7zItemInfo* CDecoder7z::FindItem(const std::wstring& itemWithPath)
{
	if (!m_ok) return NULL;
	int result = -1;
	UInt16 *temp = NULL;
	size_t tempSize = 0;
	for(int i = 0; i < m_db.db.NumFiles; i++)
	{
		const CSzFileItem *f = m_db.db.Files + i;
        size_t len;
        len = SzArEx_GetFileNameUtf16(&m_db, i, NULL);

        if (len > tempSize)
        {
          SzFree(NULL, temp);
          tempSize = len;
          temp = (UInt16 *)SzAlloc(NULL, tempSize * sizeof(temp[0]));
          if (temp == 0)
          {
            return NULL;
          }
        }

        SzArEx_GetFileNameUtf16(&m_db, i, temp);
		if (itemWithPath == std::wstring((wchar_t *)temp))
		{
			result = i;
			break;
		}
	}
	SzFree(NULL, temp);
	if (-1 == result)
		return NULL;
	else
		return new C7zItemInfo(m_filename, result);
}

UInt64 CDecoder7z::GetItemSize(int itemIndex)
{
	const CSzFileItem *f = m_db.db.Files + itemIndex;
	return f->Size;
}

bool CDecoder7z::UnzipItem(int itemIndex, void *buffer, UInt64 len)
{
	UInt32 blockIndex = 0xFFFFFFFF; /* it can have any value before first call (if outBuffer = 0) */
	Byte *outBuffer = 0; /* it must be 0 before first call for each new archive. */
	size_t outBufferSize = 0;  /* it can have any value before first call (if outBuffer = 0) */
	size_t offset = 0;
	size_t outSizeProcessed = 0;
	SRes res = SzArEx_Extract(&m_db, &m_lookStream.s, itemIndex,
              &blockIndex, &outBuffer, &outBufferSize,
              &offset, &outSizeProcessed,
              &m_allocImp, &m_allocTempImp);
	if (res != SZ_OK)
		return false;
	if (outSizeProcessed > len)
		outSizeProcessed = len;
	memcpy(buffer, outBuffer + offset, outSizeProcessed); 
	IAlloc_Free(&m_allocImp, outBuffer);
	return true;
}

HBITMAP C7zItemInfo::LoadTile(HDC dc, CHBitmapBuilder* pHBitmapBuilder)
{
	HBITMAP hbm = NULL;

	char *buffer = NULL;
	CDecoder7z* pDec7z = M_Decoder7zPool.GetDecoder(m_zipFilename);
	if (!pDec7z->IsFileOk()) return NULL;
	size_t len = pDec7z->GetItemSize(m_itemIndex);
	buffer = (char *)GlobalAlloc(GMEM_FIXED, len);
	pDec7z->UnzipItem(m_itemIndex, buffer, len);

	hbm = pHBitmapBuilder->BufferToHBitmap(buffer, len, dc);
	GlobalFree(buffer);

	return hbm;
}

void C7zItemInfo::DeleteTileIfPossible()
{
	// Don't try do delete anything out of a 7z file => do nothing
}

char * C7zItemInfo::OpenTile(int& len)
{
	CDecoder7z* pDec7z = M_Decoder7zPool.GetDecoder(m_zipFilename);
	if (!pDec7z->IsFileOk()) return NULL;
	len = pDec7z->GetItemSize(m_itemIndex);
	m_buf = (char *)GlobalAlloc(GMEM_FIXED, len);
	pDec7z->UnzipItem(m_itemIndex, m_buf, len);
	return m_buf;
}

void C7zItemInfo::CloseTile()
{
	GlobalFree(m_buf);
	m_buf = NULL;
}

CDecoder7zPool::CDecoder7zPool(int poolSize)
	: m_poolSize(poolSize)
{
}

CDecoder7zPool::~CDecoder7zPool()
{
	TDecoderList::iterator it;
	for (it = m_pool.begin(); it != m_pool.end(); ++it)
	{
		CDecoder7z* pDecoder = it->second;
		delete pDecoder;
	}
}

CDecoder7z* CDecoder7zPool::GetDecoder(const std::wstring& filename)
{
	TDecoderList::iterator it;
	for (it = m_pool.begin(); it != m_pool.end(); ++it)
	{
		if (it->first == filename)
			return it->second;
	}

	// Not found? => New decoder
	CDecoder7z* pDecoder = new CDecoder7z(filename);
	if (m_pool.size() >= m_poolSize)
	{
		CDecoder7z* pDecoderToRemove = m_pool.back().second;
		m_pool.pop_back();
		delete pDecoderToRemove;
	}
	m_pool.push_front(std::make_pair(filename, pDecoder));
	return pDecoder;
}

CDecoder7zPool M_Decoder7zPool(4);
