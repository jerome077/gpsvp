﻿/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef LBLSUBFILE_H
#define LBLSUBFILE_H

#include "PlatformDef.h"
#include <string>
#include <hash_map>

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
	UInt m_uiPoiOffset;
	UInt m_uiPoiLength;
	typedef stdext::hash_map<UInt, std::wstring> LabelCache;
	LabelCache m_cache;
	typedef stdext::hash_map<UInt, UInt> PoiCache;
	PoiCache m_poiCache;
public:
	//! Object constants
	enum enumConstants {
		cnHeaderSize = 0xD0, //!< File header size
		cnMaxLabel = 0x30
	};
	//! Parse undelying subfile
	void Parse(CSubFile * pSubFile);
	const wchar_t * GetLabel(UInt uiOffset);
	UInt GetLabelOffsetForPoi(UInt uiOffset);
	// void Dump();
};

#endif // LBLSUBFILE_H
