/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef ATLAS_H
#define ATLAS_H

#include "PlatformDef.h"
#include "Header.h"

inline bool operator == (const std::pair<CIMGFile, bool> & p, int id)
{
	return p.first.GetID() == id;
}

struct IStatusPainter;

class CAtlas
{
private:
	typedef std::list<std::pair<CIMGFile, bool> > FileList;
	FileList m_imgFiles;
#ifndef LINUX
	HKEY m_hRegKey;
#endif
	unsigned int m_uiScale10;
	IPainter * m_pPainter;
	std::list<CIMGFile *> m_listToPaint;
	UInt m_uiBestBits;
public:
#ifndef LINUX
	void Init(HKEY hRegKey);
#endif
	void Add(const tchar_t * wcFilename, IPainter * pPainter);
	void BeginPaint(unsigned int uiScale10, IPainter * pPainter, IStatusPainter * pStatusPainter);
	void PaintMapPlaceholders(IPainter * pPainter);
	void Paint(UInt uiObjectTypes, bool fDirectPaint);
	void Trim(const GeoRect &rect);
	void GetList(IListAcceptor * pAcceptor);
	void GetListUpdateCurrent(int iSelected, IListAcceptor * pAcceptor);
	void CloseMapByID(int iMap);
	void ToggleActiveByID(int iMap);
	void Load();
	void Save();
	const CIMGFile & ById(int id);
	void CloseAll();
	bool IsEmpty();
};

#endif // ATLAS_H
