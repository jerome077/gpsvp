#ifndef FATBLOCK_H
#define FATBLOCK_H

#include <string>
#include <vector>

using namespace std;

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
	string m_strFileName;
	//! Part of file
	UInt m_uiPart;
	//! File size if the block is fist in the file
	UInt m_uiSize;
	//! Vector of filesystem block numbers for the file
	vector<UInt> m_Blocks;
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
	string GetFileName() {return m_strFileName;}
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
