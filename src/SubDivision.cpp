/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "PlatformDef.h"
#include "DebugOutput.h"
#include "Subdivision.h"
#include "Common.h"
#include "SubFile.h"
#include "TreSubfile.h"
#include "RgnSubfile.h"
#include "IPainter.h"
#include "LblSubfile.h"
#include "BitStream.h"
#include "NetSubfile.h"

void CSubdivision::Parse(Byte * data, bool fIsLast)
{
	// Get pointer to data in RGN file
	m_uiRgnDataPtr = GetUInt24(data + 0x0);
	// Types of objects in this subdivision
	m_bObjTypes = data[0x3];
	// Longitude & latitude
	Int iLongitude = GetInt24(data + 0x4);
	Int iLatitude = GetInt24(data + 0x7);
	m_gpCenter = GeoPoint(iLongitude, iLatitude);
	// Width & height
	UInt uiWidth = GetUInt16(data + 0xA);
	UInt uiHeight = GetUInt16(data + 0xC);
	// Extract terminating flag from width
	m_fTerminating = ((uiWidth & 0x8000) != 0x0000);
	uiWidth &= 0x7FFF;
	// Calculate area
	m_grArea = GeoRect(
		iLongitude - (uiWidth << (GPWIDTH - m_uiBits)), 
		iLongitude + (uiWidth << (GPWIDTH - m_uiBits)), 
		iLatitude - (uiHeight << (GPWIDTH - m_uiBits)), 
		iLatitude + (uiHeight << (GPWIDTH - m_uiBits)));

	// If level is not last, get pointer to next level
	if (!fIsLast)
		m_uiNextLevelSub = GetUInt16(data + 0xE);
	else
		m_uiNextLevelSub = 0;
}

//void CSubdivision::Dump()
//{
//	// Dump everithing we know
//	dout << "\t\t""SubDivision\n";
//	dout << "\t\t\t""m_uiRgnDataPtr = " << m_uiRgnDataPtr << "\n";
//	dout << "\t\t\t""m_bObjTypes = " << UInt(m_bObjTypes) << "\n";
////	dout << "\t\t\t""m_iLongitude = " << m_iLongitude << " (" << Degree(m_iLongitude) << ")\n";
////	dout << "\t\t\t""m_iLatitude = " << m_iLatitude << " (" << Degree(m_iLatitude) << ")\n";
////	dout << "\t\t\t""m_uiWidth = " << m_uiWidth << "\n";
////	dout << "\t\t\t""m_uiHeight = " << m_uiHeight << "\n";
//	dout << "\t\t\t""m_uiNextLevelSub = " << m_uiNextLevelSub << "\n";
//	dout << "\t\t\t""m_fTerminating = " << m_fTerminating << "\n";
//}

void CSubdivision::Paint(IPainter * pPainter, UInt uiObjects, bool fDirectPaint)
{
	if (!pPainter->WillPaint(m_grArea))
		return;

	// If no size then nothing to do
	if (m_uiSize == 0)
		return;

	// Read
	if (!m_fLoaded)
	{
		// Allocate buffer for data
		m_pData = new Byte[m_uiSize];
		// Get pointer to RGN subfileps
		CRgnSubfile * pRGNSubfile = m_pTreSubfile->GetRgnSubfile();
		// And read it from RGN subfile
		pRGNSubfile->Read(m_pData, m_uiRgnDataPtr, m_uiSize);
	}

	if (!m_fLoaded && !fDirectPaint)
	{
		// Init start data
		UInt uiPointerOffset = 0;
		UInt uiSegmentCount = 0;
		// Count different object types
		if (m_bObjTypes & maskPoints)
			++ uiSegmentCount;
		if (m_bObjTypes & maskIndexedPoints)
			++ uiSegmentCount;
		if (m_bObjTypes & maskPolylines)
			++ uiSegmentCount;
		if (m_bObjTypes & maskPolygons)
			++ uiSegmentCount;
		// We expect some type
		Check(uiSegmentCount > 0);
		Byte * data = m_pData;
		// Read pointers
		list<UInt> pointers;
		for (UInt i = 0; i < uiSegmentCount - 1; ++i)
		{
			pointers.push_back(GetUInt16(data + uiPointerOffset));
			uiPointerOffset += 2;
		}
		// First block starts immediately
		pointers.push_front(uiPointerOffset);
		// Last block ends in the end
		pointers.push_back(m_uiSize);
		// If polygons are present, parse them
		if (m_bObjTypes & maskPolygons)
		{
			// Take the last block
			UInt uiEnd = pointers.back();
			pointers.pop_back();
			UInt uiStart = pointers.back();
			// Parse all polygons
			UInt uiPointer = uiStart;
			while(uiPointer < uiEnd)
			{
				// Paint & advance to next polygon
				uiPointer += ReadPoly(data + uiPointer, uiEnd - uiPointer, maskPolygons);
			}
			// There must be neither extra, no missing bytes
			Check(uiPointer == uiEnd);
		}
		// If polylines are present, parse them
		if (m_bObjTypes & maskPolylines)
		{
			// Take the last block
			UInt uiEnd = pointers.back();
			pointers.pop_back();
			UInt uiStart = pointers.back();
			// Parse all polygons
			UInt uiPointer = uiStart;
			while(uiPointer < uiEnd)
			{
				// Paint & advance to next polyline
				uiPointer += ReadPoly(data + uiPointer, uiEnd - uiPointer, maskPolylines);
			}
			// There must be neither extra, no missing bytes
			Check(uiPointer == uiEnd);
		}
		if (m_bObjTypes & maskIndexedPoints)
		{
			UInt uiEnd = pointers.back();
			pointers.pop_back();
			UInt uiStart = pointers.back();
			UInt uiPointer = uiStart;
			while(uiPointer < uiEnd)
			{
				uiPointer += ReadPoint(data + uiPointer, uiEnd - uiPointer);
			}
			Check(uiPointer == uiEnd);
		}
		if (m_bObjTypes & maskPoints)
		{
			UInt uiEnd = pointers.back();
			pointers.pop_back();
			UInt uiStart = pointers.back();
			UInt uiPointer = uiStart;
			while(uiPointer < uiEnd)
			{
				uiPointer += ReadPoint(data + uiPointer, uiEnd - uiPointer);
			}
			Check(uiPointer == uiEnd);
		}
	}
	m_fLoaded = true;
	// Paint
	if (fDirectPaint)
	{
		// Init start data
		UInt uiPointerOffset = 0;
		UInt uiSegmentCount = 0;
		// Count different object types
		if (m_bObjTypes & maskPoints)
			++ uiSegmentCount;
		if (m_bObjTypes & maskIndexedPoints)
			++ uiSegmentCount;
		if (m_bObjTypes & maskPolylines)
			++ uiSegmentCount;
		if (m_bObjTypes & maskPolygons)
			++ uiSegmentCount;
		// We expect some type
		Check(uiSegmentCount > 0);
		Byte * data = m_pData;
		// Read pointers
		list<UInt> pointers;
		for (UInt i = 0; i < uiSegmentCount - 1; ++i)
		{
			pointers.push_back(GetUInt16(data + uiPointerOffset));
			uiPointerOffset += 2;
		}
		// First block starts immediately
		pointers.push_front(uiPointerOffset);
		// Last block ends in the end
		pointers.push_back(m_uiSize);
		// If polygons are present, parse them
		if (m_bObjTypes & maskPolygons)
		{
			if (uiObjects == maskPolygons)
			{
				// Take the last block
				UInt uiEnd = pointers.back();
				pointers.pop_back();
				UInt uiStart = pointers.back();
				// Parse all polygons
				UInt uiPointer = uiStart;
				while(uiPointer < uiEnd)
				{
					// Paint & advance to next polygon
					uiPointer += CPolyObject::Paint(maskPolygons, this, m_gpCenter, m_uiBits, data + uiPointer, uiEnd - uiPointer, pPainter);
				}
				// There must be neither extra, no missing bytes
				Check(uiPointer == uiEnd);
			}
			else
				pointers.pop_back();
		}
		// If polylines are present, parse them
		if (m_bObjTypes & maskPolylines)
		{
			if (uiObjects == maskPolylines)
			{
				// Take the last block
				UInt uiEnd = pointers.back();
				pointers.pop_back();
				UInt uiStart = pointers.back();
				// Parse all polygons
				UInt uiPointer = uiStart;
				while(uiPointer < uiEnd)
				{
					// Paint & advance to next polyline
					uiPointer += CPolyObject::Paint(maskPolylines, this, m_gpCenter, m_uiBits, data + uiPointer, uiEnd - uiPointer, pPainter);
				}
				// There must be neither extra, no missing bytes
				Check(uiPointer == uiEnd);
			}
			else
				pointers.pop_back();
		}
		if (m_bObjTypes & maskIndexedPoints)
		{
			UInt uiEnd = pointers.back();
			pointers.pop_back();
			UInt uiStart = pointers.back();
			UInt uiPointer = uiStart;
			while(uiPointer < uiEnd)
			{
				uiPointer += PaintPoint(data + uiPointer, uiEnd - uiPointer, pPainter);
			}
			Check(uiPointer == uiEnd);
		}
		if (m_bObjTypes & maskPoints)
		{
			UInt uiEnd = pointers.back();
			pointers.pop_back();
			UInt uiStart = pointers.back();
			UInt uiPointer = uiStart;
			while(uiPointer < uiEnd)
			{
				uiPointer += PaintPoint(data + uiPointer, uiEnd - uiPointer, pPainter);
			}
			Check(uiPointer == uiEnd);
		}
	}
	else
	{
		{
			list<CPolyObject>::iterator it;
			if (uiObjects & maskPolylines)
			{
				for (it = m_listPolylines.begin(); it != m_listPolylines.end(); ++it)
					it->Paint(pPainter, this);
			}
			if (uiObjects & maskPolygons)
			{
				for (it = m_listPolygons.begin(); it != m_listPolygons.end(); ++it)
					it->Paint(pPainter, this);
			}	
		}
		{
			list<CPoint>::iterator it;
			if (uiObjects & maskPoints)
			{
				for (it = m_listPoints.begin(); it != m_listPoints.end(); ++it)
					it->Paint(pPainter, this);
			}
		}
	}
}

void CSubdivision::SetNextOffset(UInt uiNextOffset) 
{
	//! Calculate own size as difference between own and next offsets
	m_uiSize = uiNextOffset - m_uiRgnDataPtr;
}


UInt CSubdivision::ReadPoint(Byte * data, UInt uiMaxSize)
{
	m_listPoints.push_back(CPoint());
	return m_listPoints.back().Parse(data, uiMaxSize, m_gpCenter, m_uiBits, this);
}

UInt CSubdivision::PaintPoint(Byte * data, UInt uiMaxSize, IPainter * pPainter)
{
	return CPoint::Paint(data, uiMaxSize, m_gpCenter, m_uiBits, this, pPainter);
}

UInt CSubdivision::ReadPoly(Byte * data, UInt uiMaxSize, enumObjTypes eType)
{
	CPolyObject * pTo;
	if (eType == maskPolygons)
	{
		m_listPolygons.push_back(CPolyObject());
		pTo = &m_listPolygons.back();
	}
	else if (eType == maskPolylines)
	{
		m_listPolylines.push_back(CPolyObject());
		pTo = &m_listPolylines.back();
	}
	CPolyObject & polyObject = *pTo;
	polyObject.Init(eType, this, m_gpCenter, m_uiBits);
	UInt uiSize = polyObject.Parse(data, uiMaxSize);
	// We return polygon data length
	return uiSize;
}

void CSubdivision::Init(CMapLevel * pLevel, CTreSubfile * pSubFile) 
{
	// Record passed parameters
	m_pMapLevel = pLevel; 
	m_pTreSubfile = pSubFile; 
	// And cache some properties
	m_uiBits = pLevel->GetBits();
}

void CSubdivision::Paint(IPainter * pPainter, UInt uiBits, UInt uiObjects, bool fDirectPaint)
{
	if (!pPainter->WillPaint(m_grArea))
		return;
	// If it's the level to paint
	if (uiBits == m_pMapLevel->GetBits())
	{
		// Then paint it to the painter
		Paint(pPainter, uiObjects, fDirectPaint);
	}
	else if (uiBits > m_pMapLevel->GetBits())
	{
		// Else we have to paint corresponding subdivisions in the next level
		if (m_uiNextLevelSub != 0)
			m_pTreSubfile->PaintFrom(pPainter, uiBits, m_uiNextLevelSub - 1, uiObjects, fDirectPaint);
	}
}



UInt CPoint::Parse(Byte * data, UInt uiMaxSize, GeoPoint gpBase, UInt uiBits, CSubdivision * pOwner)
{
	m_uiLabel = 0;
	m_uiType = data[0x0];
	bool fHasSubtype = (m_uiType & 0x80) != 0;
	m_uiType &= 0x7F;
	UInt uiLabelInfo = GetUInt24(data + 0x1);
	bool fFlag = (uiLabelInfo & 0x800000) != 0;
	m_uiLabel = uiLabelInfo &0x7FFFFF;

	Int iLongitude = GetInt16(data + 4);
	Int iLatitude = GetInt16(data + 6);

	m_gpPoint = GeoPoint(
		gpBase.lon + (iLongitude << (GPWIDTH - uiBits)), 
		gpBase.lat + (iLatitude << (GPWIDTH - uiBits)));

	
	m_uiType = m_uiType << 8;
	if (fHasSubtype)
	{
		m_uiType += data[8];
		return 9 + fFlag;
	}
	else
	{
		if (fFlag)
			m_uiType += data[8];
		return 8 + fFlag;
	}
}

UInt CPoint::Paint(Byte * data, UInt uiMaxSize, GeoPoint gpBase, UInt uiBits, CSubdivision * pOwner, IPainter * pPainter)
{
	UInt uiSize = 0;
	UInt uiLabel = 0;
	UInt uiType = data[0x0];
	bool fHasSubtype = (uiType & 0x80) != 0;
	uiType &= 0x7F;
	UInt uiLabelInfo = GetUInt24(data + 0x1);
	bool fFlag = (uiLabelInfo & 0x800000) != 0;
	uiLabelInfo &= 0x7FFFFF;
	if (uiLabelInfo < 0x400000)
		uiLabel = uiLabelInfo;

	Int iLongitude = GetInt16(data + 4);
	Int iLatitude = GetInt16(data + 6);

	const GeoPoint & gpPoint = GeoPoint(
		gpBase.lon + (iLongitude << (GPWIDTH - uiBits)), 
		gpBase.lat + (iLatitude << (GPWIDTH - uiBits)));

	
	uiType = uiType << 8;
	if (fHasSubtype)
	{
		uiType += data[8];
		uiSize = 9 + fFlag;
	}
	else
	{
		if (fFlag)
			uiType += data[8];
		uiSize = 8 + fFlag;
	}
	pPainter->PaintPoint(uiType, gpPoint, pOwner->GetTreSubfile()->GetLblSubfile()->GetLabel(uiLabel));
	return uiSize;
}

void CPoint::Paint(IPainter * pPainter, CSubdivision * pOwner)
{
	UInt labelOffset = m_uiLabel;
	if((labelOffset & 0x400000) != 0){
		labelOffset = pOwner->GetTreSubfile()->GetLblSubfile()->GetLabelOffsetForPoi(labelOffset & 0x3FFFFF);
	}
	const wchar_t *label = NULL;
	if(labelOffset != 0){
		label = pOwner->GetTreSubfile()->GetLblSubfile()->GetLabel(labelOffset);
	}
	pPainter->PaintPoint(m_uiType, m_gpPoint, label);
}

UInt CPolyObject::Parse(Byte * data, UInt uiMaxSize)
{
	m_uiLabel = 0;
	// Get type & info about the length param length
	m_uiType = data[0x0];
	bool fTwoByteLen = ((m_uiType & 0x80) != 0x0);
	if (m_eType == maskPolygons)
		m_uiType &= 0x7F;
	else
		m_uiType &= 0x3F;


	// Get pointer to label and extra bit attribute
	UInt uiLabelInfo = GetUInt24(data + 0x1);
	bool fExtraBit = ((uiLabelInfo & 0x400000) != 0x0);
	bool fLabelInNet = ((uiLabelInfo & 0x800000) != 0x0);
	uiLabelInfo &= 0x3fffff; // TODO: check if bit mask is correct

	if (uiLabelInfo)
	{
		if (fLabelInNet)
		{
			CNetSubfile * pNetSubfile = m_pOwner->GetTreSubfile()->GetNetSubfile();
			if (pNetSubfile)
			{
				CNetSubfile::RoadInfo info = pNetSubfile->GetRoadInfo(uiLabelInfo);
				if (info.uiLabelOffset)
					m_uiLabel = info.uiLabelOffset;
			}
		}
		else
			m_uiLabel = uiLabelInfo;
	}

	// Get longitude & latitude deltas
	Int iLongitude = GetInt16(data + 0x4);
	Int iLatitude = GetInt16(data + 0x6);

	// Get length and determine header size
	UInt uiLen;
	UInt uiHeaderSize;
	if (fTwoByteLen)
	{
		uiLen = GetUInt16(data + 0x8);
		uiHeaderSize = 0xB;
	}
	else
	{
		uiLen = data[0x8];
		uiHeaderSize = 0xA;
	}
	Check(uiHeaderSize + uiLen <= uiMaxSize);

	// Get bitstream info
	Byte bInfo = data[uiHeaderSize - 1];
	UInt bLatBaseBits = bInfo >> 0x4;
	UInt bLonBaseBits = bInfo & 0x0F;

	// Init parsing object
	CBitStream stream(data + uiHeaderSize, uiLen);

	// Extract sign info
	bool fLonSameSigned = stream.GetBit();
	bool fLonNegative = false;
	if (fLonSameSigned)
		fLonNegative = stream.GetBit();
	bool fLatSameSigned = stream.GetBit();
	bool fLatNegative = false;
	if (fLatSameSigned)
		fLatNegative = stream.GetBit();

	// Calculate base bit lengths
	bLatBaseBits = 2 + (bLatBaseBits > 9 ? 2 * bLatBaseBits - 9 : bLatBaseBits) + (fLatSameSigned ? 0 : 1);
	bLonBaseBits = 2 + (bLonBaseBits > 9 ? 2 * bLonBaseBits - 9 : bLonBaseBits) + (fLonSameSigned ? 0 : 1);

	m_points.push_back(GeoPoint(
		m_gpBase.lon + (iLongitude << (GPWIDTH - m_uiBits)), 
		m_gpBase.lat + (iLatitude << (GPWIDTH - m_uiBits))
		));
	m_grBound.Init(m_points.back());
	// Parse all the stream
	while (!stream.AtEnd())
	{
		if (fExtraBit)
			stream.GetBit();
		int iLongitudeDiff = stream.GetInt(bLonBaseBits, fLonSameSigned, fLonNegative);
		int iLatitudeDiff = stream.GetInt(bLatBaseBits, fLatSameSigned, fLatNegative);
		iLongitude += (iLongitudeDiff);
		iLatitude += iLatitudeDiff;
		if (stream.OutOfBound())
			break;
		// Extract two numbers and put them into list
		m_points.push_back(GeoPoint(
			m_gpBase.lon + (iLongitude << (GPWIDTH - m_uiBits)), 
			m_gpBase.lat + (iLatitude << (GPWIDTH - m_uiBits))));
		m_grBound.Append(m_points.back());
	}

	return uiHeaderSize + uiLen;
}

UInt CPolyObject::Paint(enumObjTypes eType, CSubdivision * pOwner, const GeoPoint & gpBase, UInt uiBits, Byte * data, UInt uiMaxSize, IPainter * pPainter)
{
	// Get type & info about the length param length
	UInt uiType = data[0x0];
	bool fTwoByteLen = ((uiType & 0x80) != 0x0);
	if (eType == maskPolygons)
		uiType &= 0x7F;
	else
		uiType &= 0x3F;

	// Get length and determine header size
	UInt uiLen;
	UInt uiHeaderSize;
	if (fTwoByteLen)
	{
		uiLen = GetUInt16(data + 0x8);
		uiHeaderSize = 0xB;
	}
	else
	{
		uiLen = data[0x8];
		uiHeaderSize = 0xA;
	}
	Check(uiHeaderSize + uiLen <= uiMaxSize);

	if ((eType == maskPolygons) && (uiType == 0x4a))
		return uiHeaderSize + uiLen;

	// Get pointer to label and extra bit attribute
	UInt uiLabelInfo = GetUInt24(data + 0x1);
	bool fExtraBit = ((uiLabelInfo & 0x400000) != 0x0);
	bool fLabelInNet = ((uiLabelInfo & 0x800000) != 0x0);
	uiLabelInfo &= 0x3fffff; // TODO: check if bit mask is correct

	const wchar_t * wstrLabel = 0;
	if (uiLabelInfo)
	{
		if (fLabelInNet)
		{
			CNetSubfile * pNetSubfile = pOwner->GetTreSubfile()->GetNetSubfile();
			if (pNetSubfile)
			{
				CNetSubfile::RoadInfo info = pNetSubfile->GetRoadInfo(uiLabelInfo);
				if (info.uiLabelOffset)
					wstrLabel = pOwner->GetTreSubfile()->GetLblSubfile()->GetLabel(info.uiLabelOffset);
			}
		}
		else
			wstrLabel = pOwner->GetTreSubfile()->GetLblSubfile()->GetLabel(uiLabelInfo);
	}

	if (eType == maskPolygons)
		pPainter->StartPolygon(uiType, wstrLabel);
	else
		pPainter->StartPolyline(uiType, wstrLabel);

	// Get longitude & latitude deltas
	Int iLongitude = GetInt16(data + 0x4);
	Int iLatitude = GetInt16(data + 0x6);

	// Get bitstream info
	Byte bInfo = data[uiHeaderSize - 1];
	UInt bLatBaseBits = bInfo >> 0x4;
	UInt bLonBaseBits = bInfo & 0x0F;

	// Init parsing object
	CBitStream stream(data + uiHeaderSize, uiLen);

	// Extract sign info
	bool fLonSameSigned = stream.GetBit();
	bool fLonNegative = false;
	if (fLonSameSigned)
		fLonNegative = stream.GetBit();
	bool fLatSameSigned = stream.GetBit();
	bool fLatNegative = false;
	if (fLatSameSigned)
		fLatNegative = stream.GetBit();

	// Calculate base bit lengths
	bLatBaseBits = 2 + (bLatBaseBits > 9 ? 2 * bLatBaseBits - 9 : bLatBaseBits) + (fLatSameSigned ? 0 : 1);
	bLonBaseBits = 2 + (bLonBaseBits > 9 ? 2 * bLonBaseBits - 9 : bLonBaseBits) + (fLonSameSigned ? 0 : 1);

	pPainter->AddPoint(GeoPoint(
		gpBase.lon + (iLongitude << (GPWIDTH - uiBits)), 
		gpBase.lat + (iLatitude << (GPWIDTH - uiBits))
		));
	// Parse all the stream
	while (!stream.AtEnd())
	{
		if (fExtraBit)
			stream.GetBit();
		int iLongitudeDiff = stream.GetInt(bLonBaseBits, fLonSameSigned, fLonNegative);
		int iLatitudeDiff = stream.GetInt(bLatBaseBits, fLatSameSigned, fLatNegative);
		iLongitude += (iLongitudeDiff);
		iLatitude += iLatitudeDiff;
		if (stream.OutOfBound())
			break;
		// Extract two numbers and put them into list
		pPainter->AddPoint(GeoPoint(
			gpBase.lon + (iLongitude << (GPWIDTH - uiBits)), 
			gpBase.lat + (iLatitude << (GPWIDTH - uiBits))));
	}

	// Finally paint the polygon
	pPainter->FinishObject();

	return uiHeaderSize + uiLen;
}

void CPolyObject::Paint(IPainter * pPainter, CSubdivision * pOwner)
{
	if ((m_eType == maskPolygons) && (m_uiType == 0x4a))
		return;
	if (!pPainter->WillPaint(m_grBound))
		return;
	// Start painting polygon
	if (m_eType == maskPolygons)
		pPainter->StartPolygon(m_uiType, m_uiLabel ? pOwner->GetTreSubfile()->GetLblSubfile()->GetLabel(m_uiLabel) : 0);
	else
		pPainter->StartPolyline(m_uiType, m_uiLabel ? pOwner->GetTreSubfile()->GetLblSubfile()->GetLabel(m_uiLabel) : 0);

	// Iterate through point list
	for (list<GeoPoint>::iterator it = m_points.begin(); it != m_points.end(); ++it)
	{
		// Add next point
		pPainter->AddPoint(*it);
	}

	// Finally paint the polygon
	pPainter->FinishObject();
}

void CSubdivision::Trim(const GeoRect &rect)
{
	if (!rect.Intersect(m_grArea))
	{
		m_listPolylines.clear();
		m_listPoints.clear();
		m_listPolygons.clear();
		m_fLoaded = false;
	}
}