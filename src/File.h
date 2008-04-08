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
