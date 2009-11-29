/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "DebugOutput.h"
#include "Common.h"
#include "TreSubfile.h"
#include "SubFile.h"
#include "RgnSubfile.h"
#include "IPainter.h"
#include "PlatformDef.h"
#include <memory>

#include <memory>
#include <iostream>

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
		Int igNorthBoundary = GetInt24(data + 0x15) << (GPWIDTH - 24);
		Int igEastBoundary = GetInt24(data + 0x18) << (GPWIDTH - 24);
		Int igSouthBoundary = GetInt24(data + 0x1B) << (GPWIDTH - 24);
		Int igWestBoundary = GetInt24(data + 0x1E) << (GPWIDTH - 24);
		if (igEastBoundary <= igWestBoundary)
			igEastBoundary += (1 << GPWIDTH);
		m_grBound = GeoRect(igWestBoundary, igEastBoundary, igSouthBoundary, igNorthBoundary);

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
	std::auto_ptr<Byte> pData(new Byte[m_uiMapLevelsLen]);
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
	std::auto_ptr<Byte> pData(new Byte[m_uiSubdivisionLen]);
	if (pStatusPainter)
		pStatusPainter->SetProgressItems(iLevel, m_uiSubdivisionLen);
	m_pSubFile->Read(pData.get(), m_uiSubdivisionOffset, m_uiSubdivisionLen);
	// Start from the first level
	std::list<CMapLevel>::iterator itLevel = m_Levels.begin();
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
	std::list<UInt> levels = GetLevels();
	std::list<UInt>::iterator it;
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

std::list<UInt> CTreSubfile::GetLevels() 
{
	ParseLevels();
	ParseSubdivisions();
	std::list<UInt> result;
	for (std::list<CMapLevel>::iterator it = m_Levels.begin(); it != m_Levels.end(); ++it)
	{
		if (!it->IsEmpty())
			result.push_back(it->GetBits());
	}
	return result;
}
