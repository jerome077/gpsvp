#include "DebugOutput.h"
#include "Common.h"
#include "TreSubfile.h"
#include "SubFile.h"
#include "RgnSubfile.h"
#include <windows.h>
#include "IPainter.h"

void CTreSubfile::Parse(CSubFile * pSubFile)
{
	m_pSubFile = pSubFile;
	m_fSubdivisionsParsed = false;
	m_fLevelsParsed = false;
	// Read header with constant size
	{
		// Read header data
		Byte data[cnHeaderSize];
		pSubFile->Read(data, 0x0, cnHeaderSize);
		// Get header length
		m_uiHeaderLength = GetUInt16(data + 0x0);
		// Read file type
		m_strType.assign((char*)(data + 0x2), 10);

		// General data
		// Read boundaries
		Int iNorthBoundary = GetInt24(data + 0x15);
		Int iEastBoundary = GetInt24(data + 0x18);
		Int iSouthBoundary = GetInt24(data + 0x1B);
		Int iWestBoundary = GetInt24(data + 0x1E);
		if (iEastBoundary <= iWestBoundary)
			iEastBoundary += (1 << 24);
		m_grBound = GeoRect(iWestBoundary, iEastBoundary, iSouthBoundary, iNorthBoundary);

		// Get levels block info
		m_uiMapLevelsOffset = GetUInt32(data + 0x21);
		m_uiMapLevelsLen = GetUInt32(data + 0x25);

		// Get subdivisions block info
		m_uiSubdivisionOffset = GetUInt32(data + 0x29);
		m_uiSubdivisionLen = GetUInt32(data + 0x2D);

		// Get polyline block info
		m_uiPolylineOffset = GetUInt32(data + 0x4A);
		m_uiPolylineLen = GetUInt32(data + 0x4E);
		m_uiPolylineRecSize = GetUInt16(data + 0x52);

		// Get polygons block info
		m_uiPolygonOffset = GetUInt32(data + 0x58);
		m_uiPolygonLen = GetUInt32(data + 0x5C);
		m_uiPolygonRecSize = GetUInt16(data + 0x60);

		// Get points block info
		m_uiPointOffset = GetUInt32(data + 0x66);
		m_uiPointLen = GetUInt32(data + 0x6A);
		m_uiPointRecSize = GetUInt16(data + 0x6E);
	}
}

void CTreSubfile::ParseLevels()
{
	if (m_fLevelsParsed || !m_pSubFile)
		return;
	// Levels
	// Read levels data
	auto_ptr<Byte> pData(new Byte[m_uiMapLevelsLen]);
	m_pSubFile->Read(pData.get(), m_uiMapLevelsOffset, m_uiMapLevelsLen);
	// Parse the data from the beginning
	UInt uiLevelOffset;
	for (uiLevelOffset = 0; uiLevelOffset < m_uiMapLevelsLen; uiLevelOffset += CMapLevel::cnSize)
	{
		// Add level & make it read its data
		m_Levels.push_back(CMapLevel());
		m_Levels.back().Read(pData.get() + uiLevelOffset);
	}
	// Tell the last level it's last
	m_Levels.back().SetLast();
	m_fLevelsParsed = true;
}

void CTreSubfile::ParseSubdivisions(IStatusPainter * pStatusPainter, int iLevel)
{
	if (m_fSubdivisionsParsed || !m_pSubFile)
		return;
	ParseLevels();
	// Subdivisions are to be read by each level, because they need to be attached to
	// level and they have different size depending on level
	// Read subdivisions data
	auto_ptr<Byte> pData(new Byte[m_uiSubdivisionLen]);
	if (pStatusPainter)
		pStatusPainter->SetProgressItems(iLevel, m_uiSubdivisionLen);
	m_pSubFile->Read(pData.get(), m_uiSubdivisionOffset, m_uiSubdivisionLen);
	// Start from the first level
	list<CMapLevel>::iterator itLevel = m_Levels.begin();
	UInt uiPos = 0;
	UInt uiInCurrentLevel = 0;
	UInt uiLevel = 0;
	// We always need to know previous subdivision
	CSubdivision * pPrevSubdivision = 0;
	int iNotEmpty = 0;
	while ((uiPos < m_uiSubdivisionLen) && (itLevel != m_Levels.end()))
	{
		if (pStatusPainter)
			pStatusPainter->SetProgress(iLevel, uiPos);
		// Append subdivision to list
		m_Subdivisions.push_back(CSubdivision());
		// And to index vector
		if (m_SubdivisionsIndex.capacity() == m_SubdivisionsIndex.size())
			m_SubdivisionsIndex.reserve(m_SubdivisionsIndex.capacity() * 2);
		m_SubdivisionsIndex.push_back(&m_Subdivisions.back());
		// Init it with its level and TRE subfile reference
		m_Subdivisions.back().Init(&(*itLevel), this);
		// Make it parse its data
		m_Subdivisions.back().Parse(pData.get() + uiPos, itLevel->IsLast());
		m_Subdivisions.back().m_uiLevel = uiLevel;
		if (!m_Subdivisions.back().IsEmpty())
		{
			itLevel->SetEmpty(false);
			++iNotEmpty;
		}
		// Subdivision size depends on whether the level is last
		if (itLevel->IsLast())
			uiPos += CSubdivision::cnLastLevelSize;
		else
			uiPos += CSubdivision::cnSize;
		// Count subdivisions for level
		++ uiInCurrentLevel;
		// If it's enough for current level we should advance to next
		if (uiInCurrentLevel == itLevel->GetSubdivisions())
		{
			uiInCurrentLevel = 0;
			++itLevel;
			++uiLevel;
			m_Subdivisions.back().SetTerminating();
		}
		// Tell the previous level our offset to calculate its size
		if (pPrevSubdivision)
			pPrevSubdivision->SetNextOffset(m_Subdivisions.back().GetOffset());
		// This level is previous
		pPrevSubdivision = &m_Subdivisions.back();
	}
	// The size of the last subdivision is limited by the whole block size
	if(pPrevSubdivision != 0)
		pPrevSubdivision->SetNextOffset(m_pRgnSubfile->GetSize());
	// Check correctness of decoding
	Check(itLevel == m_Levels.end());
	Check(iNotEmpty != 0);
	Check(uiPos + 0x4 == m_uiSubdivisionLen);
	m_fSubdivisionsParsed = true;
}


//void CTreSubfile::Dump()
//{
//	// Dump everything
//	dout << "\t\t""m_uiHeaderLength = " << m_uiHeaderLength << "\n";
//	dout << "\t\t""m_strType = " << m_strType.c_str() << "\n";
////	dout << "\t\t""m_iNorthBoundary = " << m_iNorthBoundary << " (" << Degree(m_iNorthBoundary) << ")\n";
////	dout << "\t\t""m_iSouthBoundary = " << m_iSouthBoundary << " (" << Degree(m_iSouthBoundary) << ")\n";
////	dout << "\t\t""m_iEastBoundary = " << m_iEastBoundary << " (" << Degree(m_iEastBoundary) << ")\n";
////	dout << "\t\t""m_iWestBoundary = " << m_iWestBoundary << " (" << Degree(m_iWestBoundary) << ")\n";
//
//	dout << "\t\t""m_uiMapLevelsOffset = " << m_uiMapLevelsOffset << "\n";
//	dout << "\t\t""m_uiMapLevelsLen = " << m_uiMapLevelsLen << "\n";
//	dout << "\t\t""m_uiSubdivisionOffset = " << m_uiSubdivisionOffset << "\n";
//	dout << "\t\t""m_uiSubdivisionLen = " << m_uiSubdivisionLen << "\n";
//	dout << "\t\t""m_uiPolylineOffset = " << m_uiPolylineOffset << "\n";
//	dout << "\t\t""m_uiPolylineLen = " << m_uiPolylineLen << "\n";
//	dout << "\t\t""m_uiPolylineRecSize = " << m_uiPolylineRecSize << "\n";
//	dout << "\t\t""m_uiPolygonOffset = " << m_uiPolygonOffset << "\n";
//	dout << "\t\t""m_uiPolygonLen = " << m_uiPolygonLen << "\n";
//	dout << "\t\t""m_uiPolygonRecSize = " << m_uiPolygonRecSize << "\n";
//	dout << "\t\t""m_uiPointOffset = " << m_uiPointOffset << "\n";
//	dout << "\t\t""m_uiPointLen = " << m_uiPointLen << "\n";
//	dout << "\t\t""m_uiPointRecSize = " << m_uiPointRecSize << "\n";
//
//	for (list<CMapLevel>::iterator itLevel = m_Levels.begin(); itLevel != m_Levels.end(); ++itLevel)
//		itLevel->Dump();
//	
//	for (list<CSubdivision>::iterator itSD = m_Subdivisions.begin(); itSD != m_Subdivisions.end(); ++itSD)
//		itSD->Dump();
//}

void CTreSubfile::Paint(IPainter * pPainter, UInt uiBits, UInt uiObjects, bool fDirectPaint)
{
	// All we have to do is to paint our first block of subdivisions
	PaintFrom(pPainter, uiBits, 0, uiObjects, fDirectPaint);
}

void CTreSubfile::PaintFrom(IPainter * pPainter, UInt uiBits, UInt uiSubdivision, UInt uiObjects, bool fDirectPaint)
{
	ParseSubdivisions();
	// We paint subdivisions till we meet terminating subdivision
	while (uiSubdivision < m_SubdivisionsIndex.size())
	{
		// Paint subdivision
		m_SubdivisionsIndex[uiSubdivision]->Paint(pPainter, uiBits, uiObjects, fDirectPaint);
		// Break if it's terminating
		if (m_SubdivisionsIndex[uiSubdivision]->IsTerminating())
			break;
		// Else advance to next
		++ uiSubdivision;
	}
}

UInt CTreSubfile::GetLevelByScale(unsigned int uiScale10)
{
	ParseLevels();
	ParseSubdivisions();
	UInt uiRes = 0;
	// We iterate through all levels
	list<UInt> levels = GetLevels();
	list<UInt>::iterator it;
	for (it = levels.begin(); it != levels.end(); ++it)
	{
		// Looking for a level with appropriate detail
		UInt uiLevelScale = 10 << (24 - *it);
		if (uiLevelScale <= uiScale10)
			break;
	}
	if (it != levels.begin())
		--it;
	return *it;
}

void CTreSubfile::Trim(const GeoRect &rect)
{
	std::list<CSubdivision>::iterator it;
	for (it = m_Subdivisions.begin(); it != m_Subdivisions.end(); ++it)
		it->Trim(rect);
}

list<UInt> CTreSubfile::GetLevels() 
{
	ParseLevels();
	ParseSubdivisions();
	list<UInt> result;
	for (list<CMapLevel>::iterator it = m_Levels.begin(); it != m_Levels.end(); ++it)
	{
		if (!it->IsEmpty())
			result.push_back(it->GetBits());
	}
	return result;
}
