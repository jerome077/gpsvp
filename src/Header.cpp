/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "DebugOutput.h"
#include "Header.h"
#include "NetSubfile.h"
#include "ipainter.h"

#include <algorithm>
#include <fstream>
#include <windows.h>

struct CIMGFile::Map
{
	Map(CIMGFile * pOwner) : m_pOwner(pOwner){};
	//! Map of subfiles indexed by filenames
	typedef map<string, CSubFile> SubFiles;
	SubFiles m_SubFiles;
	//! TRE subfile object
	CTreSubfile m_TreSubfile;
	//! RGN subfile object
	CRgnSubfile m_RgnSubfile;
	//! LBL subfile object
	CLblSubfile m_LblSubfile;
	//! NET subfile object
	CNetSubfile * m_pNetSubfile;
	CIMGFile * m_pOwner;
	GeoPoint GetCenter() const;
	void Dump(const std::wstring & wstrFilename) const
	{
		wchar_t buff[100];
		for (SubFiles::const_iterator it = m_SubFiles.begin(); it != m_SubFiles.end(); ++it)
		{
			wsprintf(buff, L".%S", it->first.c_str());
			int size = it->second.GetSize();
			auto_ptr<Byte> buffer(new Byte[size]);
			it->second.Read(buffer.get(), 0, size);
			std::basic_ofstream<Byte> of((wstrFilename + buff).c_str());
			of.write(buffer.get(), size);
		}
	}
	void ParseSubfiles()
	{
		// Parse LBL subfile
		m_LblSubfile.Parse(&m_SubFiles["LBL"]);
		// m_LblSubfile.Dump();
		// Parse NET subfile
		if (m_SubFiles.find("NET") != m_SubFiles.end())
		{
			m_pNetSubfile = new CNetSubfile();
			m_pNetSubfile->Parse(&m_SubFiles["NET"]);
			// m_pNetSubfile->Dump();
		}
		else
			m_pNetSubfile = 0;
		// Parse RGN subfile
		m_RgnSubfile.Parse(&m_SubFiles["RGN"]);
		//Set it to TRE subfile
		m_TreSubfile.SetRgnSubfile(&m_RgnSubfile);
		m_TreSubfile.SetLblSubfile(&m_LblSubfile);
		m_TreSubfile.SetNetSubfile(m_pNetSubfile);
		// Parse TRE subfile
		m_TreSubfile.Parse(&m_SubFiles["TRE"]);
	}
	//void Dump()
	//{
	//	for (map<string, CSubFile>::iterator it = m_SubFiles.begin(); it != m_SubFiles.end(); ++it)
	//	{
	//		dout << "File " << it->first << "\n";
	//		it->second.Dump();
	//	}
	//	dout << "TRE subfile\n";
	//	m_TreSubfile.Dump();
	//}
};

void CIMGFile::Parse()
{
	Map * currentMap;
	currentMap = new Map(this);
	m_maps.push_back(currentMap);

	// Read XOR byte & set it to file
	Byte bXOR;
	m_File.Read(&bXOR, 0, 1);
	m_File.SetXOR(bXOR);
	// Buffer for header data
	Byte data[cnHeaderSize];
	// Read header
	m_File.Read(data, 0, cnHeaderSize);
	// Read xor byte
	m_bXOR = data[0x0];
	// Get filesystem block size
	UInt uiBlockSizeExp = data[0x61] + data[0x62];
	m_uiBlockSize = 1 << (uiBlockSizeExp);
	// Get first fs block offset
	m_uiFirstBlockOffset = GetUInt32(data + 0x40C);

	UInt uiFATBlockStart;
	bool fTrueMet = false;
	// FAT blocks start after header and end before first block
	for (uiFATBlockStart = cnHeaderSize; uiFATBlockStart < m_uiFirstBlockOffset || !m_uiFirstBlockOffset; uiFATBlockStart += CFATBlock::cnSize)
	{
		// Add block
		m_FATBlocks.push_back(CFATBlock());
		// Parse its data
		m_FATBlocks.back().Parse(this, uiFATBlockStart);
		// If it is true, add to corresponding subfile
		if (m_FATBlocks.back().IsTrue())
		{
			if (
				m_FATBlocks.back().GetPart() == 0 && 
				currentMap->m_SubFiles.find(m_FATBlocks.back().GetFileName()) 
					!= currentMap->m_SubFiles.end())
			{
				currentMap = new Map(this);
				m_maps.push_back(currentMap);
			}
			currentMap->m_SubFiles[m_FATBlocks.back().GetFileName()].Add(&m_FATBlocks.back());
			fTrueMet = true;
		}
		else 
		{
			if (!m_uiFirstBlockOffset && fTrueMet)
				break;
		}
	}
	for (Maps::iterator it = m_maps.begin(); it != m_maps.end(); ++it)
		(*it)->ParseSubfiles();
};
//void CIMGFile::Dump()
//{
//	// Dump everything
//	dout << "m_bXOR = 0x" << UInt(m_bXOR) << "\n";
//	dout << "m_uiFirstBlockOffset = 0x" << m_uiFirstBlockOffset << "\n";
//	for (Maps::iterator it = m_maps.begin(); it != m_maps.end(); ++it)
//		(*it)->Dump();
//}

bool CIMGFile::Parse(const wchar_t * wcFilename)
{
	m_wstrFilename = wcFilename;
	static int s_iNextID = 0;
	m_iID = s_iNextID ++;
	// Just open and parse file
	try
	{
		m_File.Open(wcFilename);
		if (!m_File)
			return false;
		Parse();
		return true;
	}
	catch (const char *)
	{
		return false;
	}
}

void CIMGFile::Paint(IPainter * pPainter, UInt uiBits, UInt uiObjects, bool fDirectPaint)
{
	// For now just tell TRE subfile to paint
	for (Maps::iterator it = m_maps.begin(); it != m_maps.end(); ++it)
	{
		list<UInt> levels = (*it)->m_TreSubfile.GetLevels();
		if (find(levels.begin(), levels.end(), uiBits) == levels.end())
			continue;
		(*it)->m_TreSubfile.Paint(pPainter, uiBits, uiObjects, fDirectPaint);
	}
}

void CIMGFile::Dump() const
{
	m_maps.front()->Dump(m_wstrFilename);
}

bool CIMGFile::WillRead() const
{
	for (Maps::const_iterator it = m_maps.begin(); it != m_maps.end(); ++it)
	{
		if ((*it)->m_TreSubfile.WillRead())
			return true;
	}
	return false;
}

bool CIMGFile::IsLoaded() const
{
	return !WillRead();
}

void CIMGFile::Trim(const GeoRect &rect)
{
	for (Maps::iterator it = m_maps.begin(); it != m_maps.end(); ++it)
		(*it)->m_TreSubfile.Trim(rect);
}

GeoPoint CIMGFile::GetCenter() const
{
	return GetRect().Center();
}

GeoRect CIMGFile::GetRect() const
{
	GeoRect rect = m_maps.front()->m_TreSubfile.GetRect();
	for (Maps::const_iterator it = m_maps.begin(); it != m_maps.end(); ++it)
		rect.Append((*it)->m_TreSubfile.GetRect());
	return rect;
}

UInt CIMGFile::GetLevelByScale(unsigned int uiScale10, IPainter * pPainter)
{
	UInt res = 100;
	for (Maps::const_iterator it = m_maps.begin(); it != m_maps.end(); ++it)
	{
		if (pPainter->WillPaint((*it)->m_TreSubfile.GetRect()))
		{
			list<UInt> levels = (*it)->m_TreSubfile.GetLevels();

			list<UInt>::iterator it;
			for (it = levels.begin(); it != levels.end(); ++it)
			{
				// Looking for a level with appropriate detail
				UInt uiLevelScale = 10 << (24 - *it);
				if (uiLevelScale <= uiScale10)
					break;
			}
			if (it != levels.begin())
				--it;
			if (it == levels.end())
				continue;
			UInt bits = *it;

			UInt uiBestScale = 10 << (24 - res);
			UInt uiNewScale = 10 << (24 - bits);
			if (res == 100)
				res = bits;
			if (uiBestScale < uiScale10)
			{
				if (bits < res)
					res = bits;
			}
			else if (uiNewScale < uiScale10)
			{
			}
			else if (bits > res)
			{
				res = bits;
			}
		}
	}
	return res;
}

list<UInt> CIMGFile::GetLevels(IPainter * pPainter)
{
	for (Maps::const_iterator it = m_maps.begin(); it != m_maps.end(); ++it)
	{
		if (pPainter->WillPaint((*it)->m_TreSubfile.GetRect()))
			return (*it)->m_TreSubfile.GetLevels();
	}
	return m_maps.front()->m_TreSubfile.GetLevels();
}


void CIMGFile::ParseSubdivisions(IStatusPainter * pStatusPainter, int iLevel)
{
	if (!pStatusPainter)
		return;
	pStatusPainter->SetProgressItems(iLevel, m_maps.size());
	for (Maps::const_iterator it = m_maps.begin(); it != m_maps.end(); ++it)
	{
		
		(*it)->m_TreSubfile.ParseSubdivisions(pStatusPainter, iLevel + 1);
		pStatusPainter->Advance(iLevel);
	}
}
