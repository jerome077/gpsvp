#ifndef SUBDIVISION_H
#define SUBDIVISION_H

#include <list>

using namespace std;

#include "PlatformDef.h"
#include "GeoPoint.h"

class CMapLevel;
class CTreSubfile;
struct IPainter;

#include "Common.h"
class CSubdivision;
class CPolyObject
{
	enumObjTypes m_eType;
	//! List of point deltas in the stream
	list<GeoPoint> m_points;
	//! Object label
	UInt m_uiLabel;
	//! Object owner
	CSubdivision * m_pOwner;
	UInt m_uiType;
	GeoPoint m_gpBase;
	UInt m_uiBits;
	GeoRect m_grBound;
public:
	void Init(enumObjTypes eType, CSubdivision * pOwner, GeoPoint gpBase, UInt uiBits)
	{
		m_eType = eType;
		m_pOwner = pOwner;
		m_gpBase = gpBase;
		m_uiBits = uiBits;
	}
	UInt Parse(Byte * data, UInt uiMaxSize);
	void Paint(IPainter * pPainter, CSubdivision * pOwner);
	static UInt Paint(enumObjTypes eType, CSubdivision * pOwner, const GeoPoint & gpBase, UInt uiBits, Byte * data, UInt uiMaxSize, IPainter * pPainter);
};

class CPoint
{
	UInt m_uiType;
	UInt m_uiLabel;
	GeoPoint m_gpPoint;
public:
	UInt Parse(Byte * data, UInt uiMaxSize, GeoPoint gpBase, UInt uiBits, CSubdivision * pOwner);
	static UInt Paint(Byte * data, UInt uiMaxSize, GeoPoint gpBase, UInt uiBits, CSubdivision * pOwner, IPainter * pPainter);
	void Paint(IPainter * pPainter, CSubdivision * pOwner);
};

//! Subdivision is compact part of map level, which can be painted quickly
class CSubdivision
{
	bool m_fLoaded;
	Byte * m_pData;
	//! Pointer to data in RGN subfile
	UInt m_uiRgnDataPtr;
	//! Size of object data
	UInt m_uiSize;
	//! Set of bits showing which objects are present in the file
	Byte m_bObjTypes;
	//! Area center
	GeoPoint m_gpCenter;
	//! Area 
	GeoRect m_grArea;
	//! Reference to next level subdivisions
	UInt m_uiNextLevelSub;
	//! Cached bits per pixel parameter
	UInt m_uiBits;
	//! It tells that subdivisions for previous level subdivision end here
	bool m_fTerminating;
	//! Our map level
	CMapLevel * m_pMapLevel;
	//! TRE subfile
	CTreSubfile * m_pTreSubfile;
	//! Paint one polygon
	UInt ReadPoly(Byte * data, UInt uiMaxSize, enumObjTypes eType);
	UInt ReadPoint(Byte * data, UInt uiMaxSize);
	UInt PaintPoint(Byte * data, UInt uiMaxSize, IPainter * pPainter);
	//! Paint object to painter
	void Paint(IPainter * pPainter, UInt uiObjects, bool fDirectPaint);
	list<CPolyObject> m_listPolylines;
	list<CPolyObject> m_listPolygons;
	list<CPoint> m_listPoints;
public:
	CSubdivision() : m_fLoaded(false), m_pData(0) {};
	~CSubdivision() { if (m_pData) delete m_pData; }
	UInt m_uiLevel;
	//! Object constants
	enum enumConstants {
		cnSize = 0x10, //! Object data size
		cnLastLevelSize = 0x0E //! Object data size foa last level
	};
	//! Parse data describing the level
	void Parse(Byte * data, bool fIsLast);
	//! Dump all internal data
	// void Dump();
	//! Is the subdivision terminating
	bool IsTerminating() {return m_fTerminating;}
	//! Tell the subdivision it's terminating
	void SetTerminating() {m_fTerminating = true;}
	//! Initialize the object
	void Init(CMapLevel * pLevel, CTreSubfile * pSubFile);
	//! Tell the object where it ends
	void SetNextOffset(UInt uiNextOffset);
	//! Get the object data offset
	UInt GetOffset() {return m_uiRgnDataPtr;}
	//! Paint region painter
	void Paint(IPainter * pPainter, UInt uiBits, UInt uiObjects, bool fDirectPaint);
	CTreSubfile * GetTreSubfile() {return m_pTreSubfile;}
	void Trim(const GeoRect &rect);
	bool IsEmpty() { return !m_bObjTypes; } //  || !m_uiSize;}
};

#endif // SUBDIVISION_H
