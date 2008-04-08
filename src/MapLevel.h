#ifndef MAPLEVEL_H
#define MAPLEVEL_H

#include <list>

using namespace std;

#include "PlatformDef.h"

class CSubdivision;

//! Map level object
class CMapLevel
{
	//! Map zoom is used to select level to display
	Byte m_bZoom;
	//! Shows how many bits are used to represent one coord in the level
	Byte m_bBPC;
	//! Number of subdivisions in the level
	UInt m_uiSubdivisions;
	//! True if the level is last
	bool m_fLast;
	bool m_fEmpty;
public:
	//! Constructor
	CMapLevel() : m_fLast(false), m_fEmpty(true) {}
	//! Object constants
	enum enumConstants {
		cnSize = 4 //!< Object size
	};
	//! Read object from buffer
	void Read(Byte * data);
	//! Debug dump of internal data
	// void Dump();
	//! Tell the level that it's last
	void SetLast() {m_fLast = true;}
	//! Ask the level if it's last
	bool IsLast() {return m_fLast;}
	//! Get-method for m_uiSubdivisions
	UInt GetSubdivisions() {return m_uiSubdivisions;}
	//! Get-method for m_bBPC
	UInt GetBits() {return m_bBPC;}
	void SetEmpty(bool fEmpty) {m_fEmpty = fEmpty;}
	bool IsEmpty() {return m_fEmpty;}
};

#endif // MAPLEVEL_H
