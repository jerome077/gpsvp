#ifndef BITSTREAM_H
#define BITSTREAM_H

#include "PlatformDef.h"

//! Bit stream parser
class CBitStream
{
	//! Stream size
	UInt m_uiSize;
	//! Current bit
	UInt m_uiBit;
	//! Current byte
	UInt m_uiByte;
	Byte * m_data;
public:
	//! Constructor
	CBitStream(Byte * data, UInt uiSize);
	//! Decode one bit
	bool GetBit();
	//! Decode integer
	Int GetInt(UInt uiSize, bool fSameSigned, bool fNegative);
	Int GetChar(UInt uiSize);
	bool OutOfBound();
	bool AtEnd();
};

inline CBitStream::CBitStream(Byte * data, UInt uiSize) : m_data(data), m_uiSize(uiSize), m_uiBit(0), m_uiByte(0)
{
}

inline bool CBitStream::OutOfBound()
{
	return m_uiByte * 8 + m_uiBit > m_uiSize * 8;
}

inline bool CBitStream::AtEnd()
{
	return m_uiByte >= m_uiSize;
}


#endif // BITSTREAM_H