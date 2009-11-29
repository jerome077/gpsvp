/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef TRESUBFILE_H
#define TRESUBFILE_H

#include <string>
#include <list>
#include <vector>

#include "PlatformDef.h"
#include "MapLevel.h"
#include "SubDivision.h"
#include "GeoPoint.h"

class CSubFile;
class CRgnSubfile;
class CLblSubfile;
class CNetSubfile;
struct IStatusPainter;

//! TRE subfile - all the structure of the file
class CTreSubfile
{
	//! TRE subfile header data length
	UInt m_uiHeaderLength;
	//! Some unnecessary info about file type
	std::string m_strType;
	//! Boundary of map area
	GeoRect m_grBound;
	//! Offset of levels section
	UInt m_uiMapLevelsOffset;
	//! Size of levels section
	UInt m_uiMapLevelsLen;

	UInt m_uiSubdivisionOffset;
	UInt m_uiSubdivisionLen;

	UInt m_uiPolylineOffset;
	UInt m_uiPolylineLen;
	UInt m_uiPolylineRecSize;

	UInt m_uiPolygonOffset;
	UInt m_uiPolygonLen;
	UInt m_uiPolygonRecSize;

	UInt m_uiPointOffset;
	UInt m_uiPointLen;
	UInt m_uiPointRecSize;

	//! Type for list of levels in the map
	typedef std::list<CMapLevel> Levels;
	//! List of levels in the map
	std::list<CMapLevel> m_Levels;
	//! List of subdivisions in the map
	std::list<CSubdivision> m_Subdivisions;
	//! Index of subdivisions
	std::vector<CSubdivision *> m_SubdivisionsIndex;

	//! Subfile with object shapes
	CRgnSubfile * m_pRgnSubfile;
	
	CLblSubfile * m_pLblSubfile;
	CNetSubfile * m_pNetSubfile;
	CSubFile * m_pSubFile;

	void ParseLevels();
	bool m_fSubdivisionsParsed;
	bool m_fLevelsParsed;
public:
	//! Object constants
	enum enumConstants {
		cnHeaderSize = 0x78 //!< Object header data size
	};
	//! Read subfile data
	void Parse(CSubFile * pSubFile);
	//! Debug dump of object data
	// void Dump() const;
	//! Get-method for m_pRgnSubfile
	CRgnSubfile * GetRgnSubfile() {return m_pRgnSubfile;}
	CLblSubfile * GetLblSubfile() {return m_pLblSubfile;}
	CNetSubfile * GetNetSubfile() {return m_pNetSubfile;}
	//! Set-method for m_pRgnSubfile
	void SetRgnSubfile(CRgnSubfile * pSubFile) {m_pRgnSubfile = pSubFile;}
	void SetLblSubfile(CLblSubfile * pSubFile) {m_pLblSubfile = pSubFile;}
	void SetNetSubfile(CNetSubfile * pSubFile) {m_pNetSubfile = pSubFile;}
	//! Paint map to painter
	void Paint(IPainter * pPainter, UInt uiBits, UInt uiObjects, bool fDirectPaint);
	//! Paint subdivisions to painter starting from uiSubdivision
	void PaintFrom(IPainter * pPainter, UInt uiBits, UInt uiSubdivision, UInt uiObjects, bool fDirectPaint);
	//! Get level to show with given scale
	UInt GetLevelByScale(unsigned int uiScale10);
	//! Get center longitude (average of east & west)
	GeoPoint GetCenter() const {return m_grBound.Center();}
	GeoRect GetRect() {return m_grBound;}
	void Trim(const GeoRect &rect);
	bool WillRead() {return !m_fSubdivisionsParsed || !m_fLevelsParsed;}
	std::list<UInt> GetLevels();
	void ParseSubdivisions(IStatusPainter * pStatusPainter = 0, int iLevel = 0);
};

#endif // TRESUBFILE_H
