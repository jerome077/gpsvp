/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef FATBLOCK_H
#define FATBLOCK_H

#include <string>
#include <vector>

#include "PlatformDef.h"
#include "TreSubfile.h"

class CIMGFile;
class CFile;

//! FAT block of Garmin filesystem
class CFATBlock
{
	//! Is it true block
	bool m_fIsTrue;
	//! Name of file the block belong to
	std::string m_strFileName;
	//! Part of file
	UInt m_uiPart;
	//! File size if the block is fist in the file
	UInt m_uiSize;
	//! Vector of filesystem block numbers for the file
	std::vector<UInt> m_Blocks;
	//! Main map object
	CIMGFile * m_pIMGFile;
	//! Size of block in Garmin filesystem
	UInt m_uiBlockSize;
public:
	//! Object constants
	enum enumConstants {
		cnSize = 0x200 //!< FAT block size
	};
	//! Parse block data from given position in IMG file
	void Parse(CIMGFile * pIMGFile, UInt uiBlockStart);
	//! Read from data described by the block
	void Read(Byte * buffer, UInt uiStart, UInt uiCount);
	UInt CalculateStart();
	//! Dump the block info
	// void Dump();
	//! Get name of file to which the block belongs
	std::string GetFileName() {return m_strFileName;}
	//! Get sequence number of part in the file
	UInt GetPart() {return m_uiPart;}
	//! Get file size; valid for first block only
	UInt GetFileSize();
	//! Check if the block is really a block
	bool IsTrue() {return m_fIsTrue;}
	//! Get size of data described by the block
	UInt GetBlockSize() {return m_uiBlockSize * m_Blocks.size();}
};

#endif // FATBLOCK_H
