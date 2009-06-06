/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef MAPLEVEL_H
#define MAPLEVEL_H

#include <list>

#include "PlatformDef.h"

class CSubdivision;

//! Map level object
class CMapLevel
{
	//! Map zoom is used to select level to display
	Byte m_bZoom;
	//! Shows how many bits are used to represent one coord in the level
	Byte m_bBPC;
	//! Number of subdivisions in the level
	UInt m_uiSubdivisions;
	//! True if the level is last
	bool m_fLast;
	bool m_fEmpty;
public:
	//! Constructor
	CMapLevel() : m_fLast(false), m_fEmpty(true) {}
	//! Object constants
	enum enumConstants {
		cnSize = 4 //!< Object size
	};
	//! Read object from buffer
	void Read(Byte * data);
	//! Debug dump of internal data
	// void Dump();
	//! Tell the level that it's last
	void SetLast() {m_fLast = true;}
	//! Ask the level if it's last
	bool IsLast() {return m_fLast;}
	//! Get-method for m_uiSubdivisions
	UInt GetSubdivisions() {return m_uiSubdivisions;}
	//! Get-method for m_bBPC
	UInt GetBits() {return m_bBPC;}
	void SetEmpty(bool fEmpty) {m_fEmpty = fEmpty;}
	bool IsEmpty() {return m_fEmpty;}
};

#endif // MAPLEVEL_H
