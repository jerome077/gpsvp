#ifndef RGNSUBFILE_H
#define RGNSUBFILE_H

#include "PlatformDef.h"

class CSubFile;

//! Subfile containing all the structure information
class CRgnSubfile
{
	//! Subfile with data
	CSubFile * m_pSubFile;
	//! Data block length
	UInt m_uiLen;
	//! Data block start
	UInt m_uiStart;
public:
	//! Object constants
	enum enumConstants {
		cnHeaderSize = 0x1D //!< File header size
	};
	//! Get-method for m_uiLen
	UInt GetSize() {return m_uiLen;};
	//! Read data from data block
	void Read(Byte * buffer, UInt uiStart, UInt uiCount);
	//! Parse undelying subfile
	void Parse(CSubFile * pSubFile);
};

#endif // RGNSUBFILE_H
