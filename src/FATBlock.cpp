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
