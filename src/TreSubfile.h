#ifndef TRESUBFILE_H
#define TRESUBFILE_H

#include <string>
#include <list>
#include <vector>

using namespace std;

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
	string m_strType;
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
	typedef list<CMapLevel> Levels;
	//! List of levels in the map
	list<CMapLevel> m_Levels;
	//! List of subdivisions in the map
	list<CSubdivision> m_Subdivisions;
	//! Index of subdivisions
	vector<CSubdivision *> m_SubdivisionsIndex;

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
	// void Dump();
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
	list<UInt> GetLevels();
	void ParseSubdivisions(IStatusPainter * pStatusPainter = 0, int iLevel = 0);
};

#endif // TRESUBFILE_H
