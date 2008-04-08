#ifdef USE_STDIO_H
#	include <stdio.h>
#else if USE_IO_H
#	include <io.h>
#	include <fcntl.h>
#endif // USE_STDIO_H

#include "File.h"
#include "Common.h"

void CFile::Open(const wstring & filename)
{
#ifdef USE_STDIO_H
	// Open file
	m_pFile = wfopen(filename.c_str(), L"rb");
	// Check if we succeeded
	Check(0 != m_pFile);
#else USE_IO_H
	// Open file
	m_iFD = _wopen(filename.c_str(), O_BINARY | O_RDONLY);
	// Check if we succeeded
	Check(-1 != m_iFD);
#endif // USE_STDIO_H
}

void CFile::Read(Byte * buffer, UInt nStart, UInt nCount)
{
#ifdef USE_STDIO_H
	// Check if file is open
	Check(0 != m_pFile);
	// Seek
	UInt uiHaveSeeked = fseek(m_pFile, nStart, SEEK_SET);
	Check(0 == uiHaveSeeked);
    // And read
	UInt uiHaveRead = fread(buffer, 1, nCount, m_pFile);
	Check(nCount == uiHaveRead);
#else if USE_IO_H
	// Check if file is open
	Check(-1 != m_iFD);
	// Seek
	Check(nStart == _lseek(m_iFD, nStart, SEEK_SET));
    // And read
	Check(nCount == _read(m_iFD, buffer, nCount));
#endif // USE_STDIO_H
	// XOR data if mask is given
	if (m_bXOR)
	{
		for (UInt u = 0; u < nCount; ++u)
			buffer[u] ^= m_bXOR;
	}
}

CFile::operator bool()
{
#ifdef USE_STDIO_H
	return 0 != m_pFile;
#else if USE_IO_H
	return -1 != m_iFD;
#endif // USE_STDIO_H
}

CFile::~CFile()
{
#ifdef USE_STDIO_H
	if (0 != m_pFile)
		fclose(m_pFile);
#else if USE_IO_H
	if (-1 != m_iFD)
		_close(m_iFD);
#endif // USE_STDIO_H
}
