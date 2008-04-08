#ifndef NETSUBFILE_H
#define NETSuBFILE_H

#include "PlatformDef.h"
#include <string>

using namespace std;

class CSubFile;

class CNetSubfile
{
	//! Subfile with data
	CSubFile * m_pSubFile;
	UInt m_uiRoadsOffset;
	UInt m_uiRoadsLength;
	UInt m_uiRoadsOffsetMultiplier;
	UInt m_uiSegmentedRoadsOffset;
	UInt m_uiSegmentedRoadsLength;
	UInt m_uiSegmentedRoadsOffsetMultiplier;
	UInt m_uiSortedRoadsOffset;
	UInt m_uiSortedRoadsLength;
	UInt m_uiSortedRoadsRecordSize;
public:
	struct RoadInfo
	{
		RoadInfo() : uiLabelOffset(0) {}
		UInt uiLabelOffset;
	};
	//! Object constants
	enum enumConstants {
		cnHeaderSize = 0xD0, //!< File header size
		cnMaxLabel = 0x30
	};
	//! Parse undelying subfile
	void Parse(CSubFile * pSubFile);
	RoadInfo GetRoadInfo(UInt uiOffset);
	RoadInfo GetSortedRoadInfo(int index);
	// void Dump();
};

#endif NETSUBFILE_H