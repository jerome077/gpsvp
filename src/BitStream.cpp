#include "BitStream.h"

//! Get one bit from stream
bool CBitStream::GetBit()
{
	if (m_uiByte >= m_uiSize)
		return false;
	// Get bit
	bool fRes = (m_data[m_uiByte] >> m_uiBit) & 0x01;
	// Advance to next bit
	++ m_uiBit;
	if (m_uiBit & 0x08)
	{
		// And byte if needed
		m_uiBit = 0;
		++ m_uiByte;
	}
	// Return bit
	return fRes;
}

Int CBitStream::GetInt(UInt uiSize, bool fSameSigned, bool fNegative)
{
	// No bits are known yet
	Int res = 0;
	// We have read nothing
	UInt uiHaveRead = 0;
	// And have to read everything
	UInt uiToRead = uiSize;
	// We read step by step
	while (uiToRead > 0)
	{
		if (m_uiByte >= m_uiSize)
			return 0;
		// If we have to read all the rest of the byte
		if (uiToRead + m_uiBit >= 8)
		{
			// Read these bytes to their position
			res |= (Int(m_data[m_uiByte] >> m_uiBit) << uiHaveRead);
			// Decrement remaining bits
			uiHaveRead += (8 - m_uiBit);
			// Increment ready bits
			uiToRead -= (8 - m_uiBit);
			// Go to the next byte
			++ m_uiByte;
			m_uiBit = 0;
		}
		else
		{
			// Read remaining bytes
			res |= (Int((m_data[m_uiByte] >> m_uiBit) & ((1 << uiToRead) -1))) << uiHaveRead;
			// Advance buffer bit pointer
			m_uiBit += uiToRead;
			// Adjust number pointers
			uiHaveRead += uiToRead;
			uiToRead -= uiToRead;
		}
	}
	if (fSameSigned)
	{
		// If the sign is known assign it
		if (fNegative)
			return -res;
		else
			return res;
	}
	else
	{
		// If unknown then it's the highest bit
		// TODO: make simpler
		Int iHigestBit = 1 << (uiSize - 1);
		if (res & iHigestBit)
		{
			res &= (iHigestBit - 1);
			if (res != 0)
				return res - iHigestBit;
			else
			{
				// Int res2 = GetInt(fSameSigned ? uiSize : uiSize - 1, false, false);
				Int res2 = GetInt(uiSize, false, false);
				// Int res2 = GetChar(uiSize); return 0;
				if (res2 < 0)
					return 1 - iHigestBit + res2;
				else
					return iHigestBit - 1 + res2;
			}
		}
		else
			return res;
	}
}

Int CBitStream::GetChar(UInt uiSize)
{
	if (m_uiByte >= m_uiSize)
		return 0xffff;
	// No bits are known yet
	Int res = 0;
	// We have read nothing
	UInt uiHaveRead = 0;
	// And have to read everything
	UInt uiToRead = uiSize;
	// We read step by step
	while (uiToRead > 0)
	{
		// If we have to read all the rest of the byte
		if (uiToRead + m_uiBit >= 8)
		{
			res <<= (8 - m_uiBit);
			res |= Int(m_data[m_uiByte] & ((1 << (8 - m_uiBit)) - 1) );
			// Decrement remaining bits
			uiHaveRead += (8 - m_uiBit);
			// Increment ready bits
			uiToRead -= (8 - m_uiBit);
			// Go to the next byte
			++ m_uiByte;
			if (m_uiByte >= m_uiSize)
				return 0xffff;
			m_uiBit = 0;
		}
		else
		{
			res <<= uiToRead;
			res |= Int(m_data[m_uiByte] >> (8 - m_uiBit - uiToRead) & ((1 << uiToRead) - 1) );
			// Advance buffer bit pointer
			m_uiBit += uiToRead;
			// Adjust number pointers
			uiHaveRead += uiToRead;
			uiToRead -= uiToRead;
		}
	}
	return res;
}