/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef SUBFILE_H
#define SUBFILE_H

#include <list>

#include "PlatformDef.h"

class CFATBlock;

//! File of Garmin filesystem
class CSubFile
{
	//! Info about one block of the file
	struct BlockInfo
	{
		//! Constructor
		BlockInfo(CFATBlock * pBlock) : m_pBlock(pBlock) {};
		//! Pointer to block object
		CFATBlock * m_pBlock;
		//! Start of area in the file represented with the block
		UInt m_uiBlockStart;
		//! Length of area in the file represented with the block
		UInt m_uiBlockLen;
	};
	//! List of blocks with info
	std::list<BlockInfo> m_Blocks;
public:
	//! Add FAT block with file data
	void Add(CFATBlock * pBlock);
	//! Debug dump data
	// void Dump();
	//! Read data from the subfile
	void Read(Byte * buffer, UInt uiStart, UInt uiCount) const;
	//! Get subfile size
	UInt GetSize() const;
};

#endif // SUBFILE_H
