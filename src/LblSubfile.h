#ifndef LBLSUBFILE_H
#define LBLSUBFILE_H

#include "PlatformDef.h"
#include <string>
#include <hash_map>

using namespace std;

class CSubFile;

//! Subfile containing all labels
class CLblSubfile
{
	//! Subfile with data
	CSubFile * m_pSubFile;
	Byte m_bCoding;
	UInt m_uiCodepage;
	UInt m_uiDataOffset;
	UInt m_uiDataLength;
	typedef stdext::hash_map<UInt, std::wstring> LabelCache;
	LabelCache m_cache;
public:
	//! Object constants
	enum enumConstants {
		cnHeaderSize = 0xD0, //!< File header size
		cnMaxLabel = 0x30
	};
	//! Parse undelying subfile
	void Parse(CSubFile * pSubFile);
	const wchar_t * GetLabel(UInt uiOffset);
	// void Dump();
};

#endif // LBLSUBFILE_H
