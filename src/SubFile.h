#ifndef SUBFILE_H
#define SUBFILE_H

#include <list>

using namespace std;

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
	list<BlockInfo> m_Blocks;
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
