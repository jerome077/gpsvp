/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef FILE_H
#define FILE_H

#include "PlatformDef.h"

#ifdef USE_STDIO_H
#	include <stdio.h>
#endif // USE_STDIO_H

#include <string>
#include "PlatformDef.h"

using namespace std;

//! OS file abstraction
class CFile
{

#ifdef USE_STDIO_H
	//! File pointer
	FILE * m_pFile;
#else // USE_STDIO_H
#ifdef USE_IO_H
	//! File descriptor
	int m_iFD;
#endif // USE_IO_H
#endif // USE_STDIO_H
	//! Mask for XORing file data
	Byte m_bXOR;

public:
	CFile() : m_bXOR(0)
#ifdef USE_STDIO_H
	, m_pFile(0)
#elif defined(USE_IO_H)
	, m_iFD(-1)
#endif
	{}
	~CFile();
	//! Open file
	void Open(const wstring & filename);
	//! Read data from file
	void Read(Byte * buffer, UInt nStart, UInt nCount);
	//! Set	mask for XORing file data
	void SetXOR(Byte bXOR) {m_bXOR = bXOR;}
	operator bool();
};

#endif // FILE_H
