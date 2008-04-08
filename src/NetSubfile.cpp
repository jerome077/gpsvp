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