#include "Common.h"
#include "SubFile.h"
#include "FATBlock.h"

void CSubFile::Add(CFATBlock * pBlock)
{
	// Get part & check if we are expecting iy
	UInt uiPart = pBlock->GetPart();
	// Check(uiPart == m_Blocks.size());
	// Get the block size
	UInt uiBlockSize = pBlock->GetBlockSize();
	// Calculate the block start
	UInt uiBlockStart;
	if (m_Blocks.empty())
		// For the first block it's 0
		uiBlockStart = 0;
	else
		// For any following it's end of previous blolk
		uiBlockStart = m_Blocks.back().m_uiBlockStart + m_Blocks.back().m_uiBlockLen;
	// Add block to our list
	m_Blocks.push_back(pBlock);
	// And record it's params
	m_Blocks.back().m_uiBlockLen = uiBlockSize;
	m_Blocks.back().m_uiBlockStart = uiBlockStart;
}
void CSubFile::Read(Byte * buffer, UInt uiStart, UInt uiCount) const
{
	// We start with no data yet
	UInt uiHaveRead = 0;
	// Iterate through all our blocks
	list<BlockInfo>::const_iterator it;
	for (it = m_Blocks.begin(); it != m_Blocks.end(); ++it)
	{
		// If the end of the block is earlier than start of data we want to read
		UInt uiBlockStart = it->m_uiBlockStart;
		UInt uiBlockLen = it->m_uiBlockLen;
		if ((uiBlockStart + uiBlockLen) > (uiStart + uiHaveRead))
		{
			// Then we should read some data
			// It's not more than amount left to read
			// And not more amount left from reading position to end of block
			UInt uiToRead = mymin(uiCount - uiHaveRead, it->m_uiBlockStart + it->m_uiBlockLen - (uiStart + uiHaveRead));
			// We read to the current position in buffer
			// Starting from current reading position relative to start of file
			// The amount calculated earlier
			it->m_pBlock->Read(buffer + uiHaveRead, uiStart + uiHaveRead - it->m_uiBlockStart, uiToRead);
			// And advance our reading position
			uiHaveRead += uiToRead;
			// Check if reading is complete
			if (uiHaveRead == uiCount)
				break;
		}
	}
	// Check if we've read all
	Check(uiHaveRead == uiCount);
}
	
//void CSubFile::Dump()
//{
//	// Just dump list of blocks
//	for (list<BlockInfo>::iterator it = m_Blocks.begin(); it != m_Blocks.end(); ++it)
//	{
//		it->m_pBlock->Dump();
//	}
//}

UInt CSubFile::GetSize() const
{
	// Subfile size is kept in it's first block
	if (m_Blocks.empty())
		return 0;
	return m_Blocks.front().m_pBlock->GetFileSize();
}
