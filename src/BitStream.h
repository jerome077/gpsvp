/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


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