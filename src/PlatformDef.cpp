#include "PlatformDef.h"

#ifdef DO_NOT_ALIGN_DATA

UInt GetUInt32(Byte * pbStart)
{
	return *(UInt*)pbStart;
}

UInt GetUInt16(Byte * pbStart)
{
	return *(unsigned short*)pbStart;
}

UInt GetInt16(Byte * pbStart)
{
	return *(short*)pbStart;
}

Int GetInt24(Byte * pbStart)
{
	return *(int*)pbStart & 0x00ffffff;
}

UInt GetUInt24(Byte * pbStart)
{
	return *(unsigned int*)pbStart & 0x00ffffff;
}

#else // ALIGN_DATA

UInt GetUInt32(Byte * pbStart)
{
	return UInt(pbStart[0]) + (UInt(pbStart[1]) << 8) + (UInt(pbStart[2]) << 16)+ (UInt(pbStart[3]) << 24);
}

UInt GetUInt16(Byte * pbStart)
{
	return UInt(pbStart[0]) + (UInt(pbStart[1]) << 8);
}

UInt GetInt16(Byte * pbStart)
{
	if (pbStart[1] & 0x80)
		return Int(pbStart[0]) | (Int(pbStart[1]) << 8) | 0xFFFF0000;
	else
		return Int(pbStart[0]) | (Int(pbStart[1]) << 8);
}

Int GetInt24(Byte * pbStart)
{
	if (pbStart[2] & 0x80)
		return Int(pbStart[0]) | (Int(pbStart[1]) << 8) | (Int(pbStart[2]) << 16) | 0xFF000000;
	else
		return Int(pbStart[0]) | (Int(pbStart[1]) << 8) | (Int(pbStart[2]) << 16);
}

UInt GetUInt24(Byte * pbStart)
{
	return UInt(pbStart[0]) + (UInt(pbStart[1]) << 8) + (UInt(pbStart[2]) << 16);
}

#endif