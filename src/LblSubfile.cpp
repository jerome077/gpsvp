#include "LblSubfile.h"
#include "SubFile.h"
#include "DebugOutput.h"
#include "Windows.h"
#include "Winnls.h"
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

}

const wchar_t * CLblSubfile::GetLabel(UInt uiOffset)
{
	LabelCache::iterator it = m_cache.find(uiOffset);
	if (it != m_cache.end())
		return it->second.c_str();
	wstring res;
	// Now for 8-bit coding only
	if (m_bCoding == 9)
	{
		// Read data
		Byte data[cnMaxLabel];
		m_pSubFile->Read(data, m_uiDataOffset + uiOffset, cnMaxLabel);
		// Cut it off with zero
		data[cnMaxLabel - 1] = 0;

		// Convert to wide char according to codepage
		wchar_t wcRes[cnMaxLabel];
		MultiByteToWideChar(m_uiCodepage, 0, (char *)data, -1, wcRes, cnMaxLabel);
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
//				// ���� �� ����� ����, ��� ������ �����
//				break;
				// �� �� ������������ ���� ���:
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
	
	for (unsigned int i = 0; i < res.length(); ++i)
	{
		while ((i < res.length()) && !iswprint(res[i]))
			res.erase(i, 1);
	}

	return (m_cache[uiOffset] = res).c_str();
}

//void CLblSubfile::Dump()
//{
//	// Just dump everything
//	dout << "m_bCoding = " << m_bCoding << "\n";
//	dout << "m_uiCodepage = " << m_uiCodepage << "\n";
//	dout << "m_uiDataOffset = " << m_uiDataOffset << "\n";
//	dout << "m_uiDataLength = " << m_uiDataLength << "\n";
//}