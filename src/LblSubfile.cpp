/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "LblSubfile.h"
#include "SubFile.h"
#include "DebugOutput.h"
#include "PlatformDef.h"
#ifndef LINUX
#	include "Winnls.h"
#else
#	include <iconv.h>
#	include <string.h>
#endif
#include "BitStream.h"

void CLblSubfile::Parse(CSubFile * pSubFile)
{
	// Remember our subfile
	m_pSubFile = pSubFile;
	// Read our header
	Byte data[cnHeaderSize];
	// Read header
	pSubFile->Read(data, 0, cnHeaderSize);
	// File coding (6, 8 or 10-bit)
	m_bCoding = data[0x1e];
	// Codepage (for 8-bit coding)
	m_uiCodepage = GetUInt16(data + 0xAA);
	// Label data offset & length
	m_uiDataOffset = GetUInt32(data + 0x15);
	m_uiDataLength = GetUInt32(data + 0x19);

	m_uiPoiOffset = GetUInt32(data + 0x57);
	m_uiPoiLength = GetUInt32(data + 0x5B);	
}

const tchar_t * CLblSubfile::GetLabel(UInt uiOffset)
{
	LabelCache::iterator it = m_cache.find(uiOffset);
	if (it != m_cache.end())
		return it->second.c_str();
	tstring res;
	// Now for 8-bit coding only
	if (m_bCoding == 9)
	{
		// Read data
		Byte data[cnMaxLabel];
		m_pSubFile->Read(data, m_uiDataOffset + uiOffset, cnMaxLabel);
		// Cut it off with zero
		data[cnMaxLabel - 1] = 0;

		// Convert to wide char according to codepage
		tchar_t wcRes[cnMaxLabel];
#ifndef LINUX
		MultiByteToWideChar(m_uiCodepage, 0, (char *)data, -1, wcRes, cnMaxLabel);
#else
		char encoding[100];
		sprintf(encoding, "cp%d", m_uiCodepage);
		iconv_t cd = iconv_open("utf8", encoding);
		size_t inbytesleft = strlen((char*)data);
		size_t outbytesleft = cnMaxLabel;
		char * inbuf = (char*)&data[0];
		char * outbuf = (char*)&wcRes[0];
		size_t converted = iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
		iconv_close(cd);
		outbuf[converted] = 0;
#endif
		// Return result
		res = wcRes;
	}
	else if (m_bCoding == 6)
	{
		// Read data
		Byte data[cnMaxLabel];
		res.reserve(cnMaxLabel * 8 / 6 + 2);
		m_pSubFile->Read(data, m_uiDataOffset + uiOffset, cnMaxLabel);
		CBitStream bs(data, cnMaxLabel);
		bool fAfterAlpha = false;
		while (!bs.AtEnd())
		{
			wchar_t wChar = 0xffff;
			int iChar = bs.GetChar(6);
			if (iChar >= 0x01 && iChar <= 0x1A)
				wChar = L'A' + iChar - 0x01;
			else if (iChar == 0x00)
				wChar = L' ';
			else if (iChar >= 0x20 && iChar <= 0x29)
				wChar = L'0' + iChar - 0x20;
			else if (iChar == 0x1C)
			{
				int iChar2 = bs.GetChar(6);
				switch (iChar2)
				{
				case 0x00: wChar = L'@'; break;
				case 0x01: wChar = L'!'; break;
				case 0x02: wChar = L'"'; break;
				case 0x03: wChar = L'#'; break;
				case 0x04: wChar = L'$'; break;
				case 0x05: wChar = L'%'; break;
				case 0x06: wChar = L'&'; break;
				case 0x07: wChar = L'\''; break;
				case 0x08: wChar = L'('; break;
				case 0x09: wChar = L')'; break;
				case 0x0A: wChar = L'*'; break;
				case 0x0B: wChar = L'+'; break;
				case 0x0C: wChar = L','; break;
				case 0x0D: wChar = L'-'; break;
				case 0x0E: wChar = L'.'; break;
				case 0x0F: wChar = L'/'; break;

				case 0x1A: wChar = L':'; break;
				case 0x1B: wChar = L';'; break;
				case 0x1C: wChar = L'<'; break;
				case 0x1D: wChar = L'='; break;
				case 0x1E: wChar = L'>'; break;
				case 0x1F: wChar = L'?'; break;

				case 0x2B: wChar = L'['; break;
				case 0x2C: wChar = L'\\'; break;
				case 0x2D: wChar = L']'; break;
				case 0x2E: wChar = L'^'; break;
				case 0x2F: wChar = L'_'; break;
				}
			}
			else if (iChar == 0x1B)
			{
//				// Судя по карте мира, это просто конец
//				break;
				// Но по документации надо так:
				iChar = bs.GetChar(6);
				if (iChar == 0)
					wChar = L'`';
				else if (iChar <= 0x1A)
					wChar = L'a' + iChar - 0x01;
			}
			else if (iChar > 0x2F)
				break;
			if (wChar != 0xffff)
			{
				if (isalpha(wChar))
				{
					if (fAfterAlpha)
						wChar = tolower(wChar);
					else
						fAfterAlpha = true;
				}
				else
					fAfterAlpha = false;
				res += wChar;
			}
		}
	}
	
#ifndef LINUX
// LINUXTODO:
	for (unsigned int i = 0; i < res.length(); ++i)
	{
		while ((i < res.length()) && !iswprint(res[i]))
			res.erase(i, 1);
	}
#endif

	return (m_cache[uiOffset] = res).c_str();
}

UInt CLblSubfile::GetLabelOffsetForPoi(UInt uiOffset){
	PoiCache::iterator it = m_poiCache.find(uiOffset);
	if (it != m_poiCache.end())
		return it->second;

	Byte data[3];
	m_pSubFile->Read(data, m_uiPoiOffset + uiOffset, 3);
	UInt labelOffset=GetUInt24(data);

	return (m_poiCache[uiOffset] = labelOffset);
}

//void CLblSubfile::Dump()
//{
//	// Just dump everything
//	dout << "m_bCoding = " << m_bCoding << "\n";
//	dout << "m_uiCodepage = " << m_uiCodepage << "\n";
//	dout << "m_uiDataOffset = " << m_uiDataOffset << "\n";
//	dout << "m_uiDataLength = " << m_uiDataLength << "\n";
//}
