/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "NetSubfile.h"
#include "SubFile.h"

static const int cnHeaderSize = 55;

void CNetSubfile::Parse(CSubFile * pSubFile)
{
	m_pSubFile = pSubFile;
	Byte data[cnHeaderSize];
	pSubFile->Read(data, 0, cnHeaderSize);
	m_uiRoadsOffset = GetUInt32(data + 0x15);
	m_uiRoadsLength = GetUInt32(data + 0x19);
	m_uiRoadsOffsetMultiplier = data[0x1d];
	m_uiSegmentedRoadsOffset = GetUInt32(data + 0x1e);
	m_uiSegmentedRoadsLength = GetUInt32(data + 0x22);
	m_uiSegmentedRoadsOffsetMultiplier = data[0x26];
	m_uiSortedRoadsOffset = GetUInt32(data + 0x27);
	m_uiSortedRoadsLength = GetUInt32(data + 0x2a);
	m_uiSortedRoadsRecordSize = GetUInt16(data + 0x2f);
}

//void CNetSubfile::Dump()
//{
//}

CNetSubfile::RoadInfo CNetSubfile::GetRoadInfo(UInt uiOffset)
{
	RoadInfo res;

	Byte buffer[3];
	m_pSubFile->Read(buffer, m_uiRoadsOffset + uiOffset, 3);
	UInt uiLabelInfo = GetUInt24(buffer);
	bool fLast = (uiLabelInfo & 0x800000) != 0;
	bool fSegmented = (uiLabelInfo & 0x400000) != 0;
	bool fUnknown = (uiLabelInfo & 0x200000) != 0;
	uiLabelInfo &= 0x3fffff;
	res.uiLabelOffset = uiLabelInfo;

	return res;
}

CNetSubfile::RoadInfo CNetSubfile::GetSortedRoadInfo(int index)
{
	Byte buffer[3];
	m_pSubFile->Read(buffer, m_uiSortedRoadsOffset + index * 3, 3);
	UInt uiOffset = GetUInt24(buffer);
	UInt idx = uiOffset >> 22;
	uiOffset &= 0x3fffff;
	return GetRoadInfo(uiOffset + idx * 3);
}