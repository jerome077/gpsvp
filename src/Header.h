/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef IMGFILE_H
#define IMGFILE_H

#include <list>
#include <map>
#include <string>

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
	std::list<CFATBlock> m_FATBlocks;
	//! Size of a block in filesystem
	UInt m_uiBlockSize;
	//! OS file with map data
	CFile m_File;

	//! Parse opened OS file
	void Parse();
	std::wstring m_wstrFilename;
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
	std::list<UInt> GetLevels(IPainter * pPainter);
	std::wstring GetFilename() const {return m_wstrFilename;}
	int GetID() const {return m_iID;};
	void Trim(const GeoRect & rect);
	bool WillRead() const;
	void ParseSubdivisions(IStatusPainter * pStatusPainter, int iLevel);
	bool IsLoaded() const;
	void Dump() const;
};

#endif // IMGFILE_H
