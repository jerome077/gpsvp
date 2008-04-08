#include "DebugOutput.h"
#include "MapLevel.h"
#include "SubDivision.h"

void CMapLevel::Read(Byte * data)
{
	// Determine zoom level
	// Actually we don't need it
	m_bZoom = data[0] & 0x0F;
	// Determine bits per pixel, real detail level
	m_bBPC = data[1];
	// Get subdivision count
	m_uiSubdivisions = GetUInt16(data + 2);
}

//void CMapLevel::Dump()
//{
//	// Just dump everything
//	dout << "\t\t""Map level\n";
//	dout << "\t\t\t""m_bZoom = " << UInt(m_bZoom) << "\n";
//	dout << "\t\t\t""m_bBPC = " << UInt(m_bBPC) << "\n";
//	dout << "\t\t\t""m_uiSubdivisions = " << UInt(m_uiSubdivisions) << "\n";
//}
//
