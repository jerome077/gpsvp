/*	
Copyright (c) 2005-2008, Jerome077 and others
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DECODER_COMMON_H
#define DECODER_COMMON_H

#include "../GoogleMaps/GMCommon.h"
#include <windows.h>

#ifdef USE_GDI_PLUS
#	include <gdiplus.h>
#endif // USE_GDI_PLUS
#ifdef UNDER_CE
#  if UNDER_CE && _WIN32_WCE < 0x500
#  else
#    include <imaging.h> 
#  endif // _WIN32_WCE
#endif // UNDER_CE

// Callback taht will be implemented in GMPainter
class CHBitmapBuilder
{
public:
	virtual HBITMAP BufferToHBitmap(char *buffer, size_t len, HDC dc) = 0;
};

// Result of a tile search, can decode the tile
class CDecoderTileInfo
{
public:
	virtual HBITMAP LoadTile(HDC dc, CHBitmapBuilder* pHBitmapBuilder) = 0;
	virtual void DeleteTileIfPossible() = 0;
	virtual bool Is7z() { return false; };
	virtual std::wstring Get7zFilename() { return L""; };
	virtual char * OpenTile(int& len) = 0;
	virtual void CloseTile() = 0;
};

#endif
