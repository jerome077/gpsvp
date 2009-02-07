/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "DebugOutput.h"
#include "FATBlock.h"
#include "Common.h"
#include "Header.h"

void CFATBlock::Parse(CIMGFile * pIMGFile, UInt uiBlockStart)
{
	// Remember params
	m_pIMGFile = pIMGFile;
	// Cache block size
	m_uiBlockSize = pIMGFile->GetBlockSize();
	// Buffer for block data
	Byte data[cnSize];
	// Read block
	m_pIMGFile->Read(data, uiBlockStart, cnSize);
	// Byte 0 tells, if the block is real
	m_fIsTrue = (data[0x0] != 0x00 && data[0x0] != 0xff);
	if (!m_fIsTrue)
		return;

	// Bytes 0x9-0xB contain filename
	m_strFileName.assign((char*)(data + 0x9), 3);
	// File part sequence number
	m_uiPart = data[0x11];
	// First part contain file size
	m_uiSize = GetUInt32(data + 0xC);

	// The rest of file contain list of blocks
	UInt uiPos;
	for (uiPos = 0x20; uiPos < cnSize; uiPos += 2)
	{
		// They are 16-bit numbers from 0x0000 to 0xFFFE
		UInt uiSeqNumber = GetUInt16(data + uiPos);
		if (uiSeqNumber != 0xFFFF)
		{
			if (m_Blocks.capacity() == m_Blocks.size())
				m_Blocks.reserve(m_Blocks.capacity() * 2);
			m_Blocks.push_back(uiSeqNumber);
		}
	}
}

UInt CFATBlock::CalculateStart()
{
	UInt res = m_Blocks[0];
	for (UInt i = 1; i < m_Blocks.size(); ++i)
	{
		if (m_Blocks[i] < res)
			res = m_Blocks[i];
	}
	return res * m_uiBlockSize;
}

void CFATBlock::Read(Byte * buffer, UInt uiStart, UInt uiCount)
{
	// Checking state
	Check(m_pIMGFile != 0);
	// Reading parts
	UInt uiHaveRead = 0;
	while (uiHaveRead < uiCount)
	{
		// Calculate block
		UInt uiBlock = (uiStart + uiHaveRead) / m_uiBlockSize;
		// Calculate size to read
		// We read min of (we need, we can)
		UInt uiToRead = mymin(uiCount - uiHaveRead, (uiBlock + 1) * m_uiBlockSize - (uiStart + uiHaveRead));
		// Calculate block start in file
		UInt uiBlockStart = m_Blocks[uiBlock] * m_uiBlockSize;
		// Read file
		m_pIMGFile->Read(buffer + uiHaveRead, uiBlockStart + ((uiStart + uiHaveRead) - uiBlock * m_uiBlockSize), uiToRead);
		// Advance to next part
		uiHaveRead += uiToRead;
	}
}
//void CFATBlock::Dump()
//{
//	// Just dump all we know
//	dout << "\tFATBlock\n";
//	dout << "\t\tm_fIsTrue = 0x" << m_fIsTrue << "\n";
//	if (!m_fIsTrue)
//		return;
//
//	dout << "\t\tm_strFileName = " << m_strFileName.c_str() << "\n";
//	dout << "\t\tm_uiSize = 0x" << m_uiSize << "\n";
//	dout << "\t\t""m_uiPart = 0x" << m_uiPart << "\n";
//	
//	dout << "\t\tBlocks: ";
//	for (vector<UInt>::iterator it = m_Blocks.begin(); it != m_Blocks.end(); ++it)
//		dout << "0x" << *it << " ";
//	dout << "\n";
//}


UInt CFATBlock::GetFileSize() 
{
	//! File size is contained only in first block of the file
	Check(m_uiPart == 0); 
	return m_uiSize;
}
