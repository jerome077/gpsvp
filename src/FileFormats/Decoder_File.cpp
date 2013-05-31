/*	
Copyright (c) 2005-2008, Jerome077 and others
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Decoder_File.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef UNDER_CE
#	include <aygshell.h>
#endif // UNDER_CE

HBITMAP CFileTileInfo::LoadTile(HDC dc, CHBitmapBuilder* pHBitmapBuilder)
{
	HBITMAP hbm = NULL;
	#ifndef USE_GDI_PLUS
	#  if defined(BARECE) || defined(UNDER_WINE)
	#  else
		hbm = SHLoadImageFile(m_filename.c_str());
	#  endif
	#else // UNDER_CE
		Gdiplus::Bitmap bm(m_filename.c_str());
		Gdiplus::Color clr;
		bm.GetHBITMAP(clr, &hbm);
	#endif // UNDER_CE

	return hbm;
}

void CFileTileInfo::DeleteTileIfPossible()
{
	DeleteFile(m_filename.c_str());
}

char * CFileTileInfo::OpenTile(int& len)
{
	FILE *f = wfopen(m_filename.c_str(), L"rb");
	fseek(f, 0, SEEK_END);
	len = ftell(f);
	fseek(f, 0, SEEK_SET);
	m_buf = (char *)malloc(len);
	fread(m_buf, len, 1, f);
	fclose(f);
	return m_buf;
}

void CFileTileInfo::CloseTile()
{
	free(m_buf);
	m_buf = NULL;
}
