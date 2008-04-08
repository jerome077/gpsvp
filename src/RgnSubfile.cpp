#include "RgnSubfile.h"
#include "SubFile.h"
#include "DebugOutput.h"

void CRgnSubfile::Read(Byte * buffer, UInt uiStart, UInt uiCount)
{
	// We read from the corresponding subfile
	// skipping our header
	m_pSubFile->Read(buffer, uiStart + m_uiStart, uiCount);
}

void CRgnSubfile::Parse(CSubFile * pSubFile)
{
	// Remember our subfile
	m_pSubFile = pSubFile;
	// Read our header
	Byte data[cnHeaderSize];
	pSubFile->Read(data, 0, cnHeaderSize);
	// Data block start
	m_uiStart = GetUInt32(data + 0x15);
	// And length
	m_uiLen = GetUInt32(data + 0x19);
}
