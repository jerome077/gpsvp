/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifdef USE_STDIO_H
#	include <stdio.h>
#else // USE_IO_H
#	include <io.h>
#	include <fcntl.h>
#endif // USE_STDIO_H

#include "File.h"
#include "Common.h"

void CFile::Open(const std::wstring & filename)
{
#ifdef USE_STDIO_H
	// Open file
	m_pFile = wfopen(filename.c_str(), L"rb");
	// Check if we succeeded
	Check(0 != m_pFile);
#else // USE_IO_H
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
#else // USE_IO_H
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
#else // USE_IO_H
	return -1 != m_iFD;
#endif // USE_STDIO_H
}

CFile::~CFile()
{
#ifdef USE_STDIO_H
	if (0 != m_pFile)
		fclose(m_pFile);
#else // USE_IO_H
	if (-1 != m_iFD)
		_close(m_iFD);
#endif // USE_STDIO_H
}
