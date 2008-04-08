#ifndef IMGFILE_H
#define IMGFILE_H

#include <list>
#include <map>
#include <string>

using namespace std;

#include "PlatformDef.h"
#include "FATBlock.h"
#include "File.h"
#include "SubFile.h"
#include "RgnSubfile.h"
#include "LblSubfile.h"

class CNetSubfile;
struct IStatusPainter;

//! Main library object - the whole IMG file
class CIMGFile
{
	//! All the file may be xored with this byte
	Byte m_bXOR;
	//! Offset of the first filesystem data block
	UInt m_uiFirstBlockOffset;
	//! List of FAT blocks
	list<CFATBlock> m_FATBlocks;
	//! Size of a block in filesystem
	UInt m_uiBlockSize;
	//! OS file with map data
	CFile m_File;

	//! Parse opened OS file
	void Parse();
	wstring m_wstrFilename;
	int m_iID;

	struct Map;
	typedef std::list<Map *> Maps;
	Maps m_maps;
public:
	//! Object costants
	enum enumConstants {
		cnHeaderSize = 0x600 //!< Size of file header
	};
	//! Parse file with given name
	bool Parse(const wchar_t * wcFilename);
	//! Debug dump of internal data
	// void Dump();
	//! Get-method for m_uiBlockSize
	UInt GetBlockSize() {return m_uiBlockSize;}
	//! Read from the file
	void Read(Byte * buffer, UInt uiStart, UInt uiCount) {m_File.Read(buffer, uiStart, uiCount);};
	//! Paint the file to painter object
	void Paint(IPainter * pPainter, UInt uiBits, UInt uiObjects, bool fDirectPaint);
	//! Get center from TRE subfile
	GeoPoint GetCenter() const;
	GeoRect GetRect() const;
	UInt GetLevelByScale(unsigned int uiScale10, IPainter * pPainter);
	list<UInt> GetLevels(IPainter * pPainter);
	wstring GetFilename() const {return m_wstrFilename;}
	int GetID() const {return m_iID;};
	void Trim(const GeoRect & rect);
	bool WillRead() const;
	void ParseSubdivisions(IStatusPainter * pStatusPainter, int iLevel);
	bool IsLoaded() const;
	void Dump() const;
};

#endif // IMGFILE_H
