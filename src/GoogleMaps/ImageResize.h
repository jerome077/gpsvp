/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#pragma once

#include <windows.h>
#include "GMCommon.h"
#ifdef USE_GDI_PLUS
#include <gdiplus.h>
#endif // UNDER_CE

//
// Functions for smooth bitmap resize
//
// Improvement: float calculations changed to int.
//
// Ivaylo Byalkov, January 24, 2000
// e-mail: ivob@i-n.net
//

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

struct BGRColor
{
	BGRColor() {}
	BGRColor(byte R, byte G, byte B) : m_R(R), m_G(G), m_B(B) {}
	byte m_B;
	byte m_G;
	byte m_R;
};

///////////////////////////////////////////////////////////

// helper function prototypes
static BITMAPINFO *PrepareRGBBitmapInfo(WORD wWidth,
                                        WORD wHeight);

static void ShrinkDataInt(BYTE *pInBuff,
                          WORD wWidth,
                          WORD wHeight,
                          BYTE *pOutBuff,
                          WORD wNewWidth,
                          WORD wNewHeight);

static void EnlargeDataInt(BYTE *pInBuff,
                           WORD wWidth,
                           WORD wHeight,
                           BYTE *pOutBuff,
                           WORD wNewWidth,
                           WORD wNewHeight);

///////////////////////////////////////////////////////////
// Main resize function

HBITMAP ScaleBitmapInt(HDC dc, HBITMAP hBmp,
                       WORD wNewWidth,
                       WORD wNewHeight)
{
 BITMAP bmp;
 ::GetObject(hBmp, sizeof(BITMAP), &bmp);

 // check for valid size
 if((bmp.bmWidth > wNewWidth
   && bmp.bmHeight < wNewHeight)
 || (bmp.bmWidth < wNewWidth
   && bmp.bmHeight > wNewHeight))
  return NULL;

 HDC hDC = dc;
 BITMAPINFO *pbi = PrepareRGBBitmapInfo((WORD)bmp.bmWidth,
                                        (WORD)bmp.bmHeight);
 BYTE *pData = new BYTE[pbi->bmiHeader.biSizeImage];

#ifdef UNDER_CE
/*	VOID *pBits;
	long nCorrectedWidth = ((bmp.bmWidth + 3) / 4) * 4;
	HBITMAP h = CreateDIBSection(hDC, pbi, DIB_RGB_COLORS, &pBits, NULL, 0);
	HANDLE hDC1 = CreateCompatibleDC(dc);
	HANDLE hDC2 = CreateCompatibleDC(dc);
	HBITMAP hOldBitmap1 = SelectObject(hDC1, hBmp);
	HBITMAP hOldBitmap2 = SelectObject(hDC2, h);
	BitBlt(hDC2, 0, 0, bmp.bmWidth, bmp.bmHeight, hDC1, 0, 0, SRCCOPY);
	// Теперь из pBits аккуратно скопировать данные про картинку...

	byte *pByteData = (byte*)pData;
	int nPosition = 0;
	int nDataPosition = 0;

	for (int y=0; y<nHeight; y++) {
		nPosition = m_nCorrectedWidth*(m_nHeight-y-1);
		nDataPosition = nWidth*3*y;
		for (int x=0; x<nWidth; x++) {
			m_pBuffer[nPosition].m_R = pByteData[nDataPosition++];
			m_pBuffer[nPosition].m_G = pByteData[nDataPosition++];
			m_pBuffer[nPosition].m_B = pByteData[nDataPosition++];
			nPosition++;
		}
	}

	SelectObject(hDC1, hOldBitmap1);
	SelectObject(hDC2, hOldBitmap2);
	DeleteDC(hDC1);
	DeleteDC(hDC2);	*/
#else // UNDER_CE
	GetDIBits(hDC, hBmp, 0, bmp.bmHeight, pData, pbi, DIB_RGB_COLORS);
#endif // UNDER_CE

 delete pbi;
 pbi = PrepareRGBBitmapInfo(wNewWidth, wNewHeight);
 BYTE *pData2 = new BYTE[pbi->bmiHeader.biSizeImage];

 if(bmp.bmWidth >= wNewWidth
 && bmp.bmHeight >= wNewHeight)
  ShrinkDataInt(pData,
                (WORD)bmp.bmWidth,
                (WORD)bmp.bmHeight,
                pData2,
                wNewWidth,
                wNewHeight);
 else
  EnlargeDataInt(pData,
                 (WORD)bmp.bmWidth,
                 (WORD)bmp.bmHeight,
                 pData2,
                 wNewWidth,
                 wNewHeight);

 delete pData;

 HBITMAP hResBmp = ::CreateCompatibleBitmap(hDC,
                                            wNewWidth,
                                            wNewHeight);
#ifdef UNDER_CE
#else // UNDER_CE
 ::SetDIBits(hDC,
             hResBmp,
             0,
             wNewHeight,
             pData2,
             pbi,
             DIB_RGB_COLORS);
#endif // UNDER_CE

delete pbi;
delete pData2;

return hResBmp;
}

///////////////////////////////////////////////////////////

BITMAPINFO *PrepareRGBBitmapInfo(WORD wWidth, WORD wHeight)
{
 BITMAPINFO *pRes = new BITMAPINFO;
 ::ZeroMemory(pRes, sizeof(BITMAPINFO));
 pRes->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
 pRes->bmiHeader.biWidth = wWidth;
 pRes->bmiHeader.biHeight = wHeight;
 pRes->bmiHeader.biPlanes = 1;
 pRes->bmiHeader.biBitCount = 24;

 pRes->bmiHeader.biSizeImage =
  ((3 * wWidth + 3) & ~3) * wHeight;

 return pRes;
}

///////////////////////////////////////////////////////////

static int *CreateCoeffInt(int nLen, int nNewLen, BOOL bShrink)
{
 int nSum = 0, nSum2;
 int *pRes = new int[2 * nLen];
 int *pCoeff = pRes;
 int nNorm = (bShrink)
           ? (nNewLen << 12) / nLen : 0x1000;
 int	nDenom = (bShrink)? nLen : nNewLen;

 ::ZeroMemory(pRes, 2 * nLen * sizeof(int));
 for(int i = 0; i < nLen; i++, pCoeff += 2)
 {
  nSum2 = nSum + nNewLen;
  if(nSum2 > nLen)
  {
   *pCoeff = ((nLen - nSum) << 12) / nDenom;
   pCoeff[1] = ((nSum2 - nLen) << 12) / nDenom;
   nSum2 -= nLen;
  }
  else
  {
   *pCoeff = nNorm;
   if(nSum2 == nLen)
   {
    pCoeff[1] = -1;
    nSum2 = 0;
   }
  }
  nSum = nSum2;
 }

 return pRes;
}

///////////////////////////////////////////////////////////

void ShrinkDataInt(BYTE *pInBuff,
                   WORD wWidth,
                   WORD wHeight,
                   BYTE *pOutBuff,
                   WORD wNewWidth,
                   WORD wNewHeight)
{
 BYTE  *pLine = pInBuff, *pPix;
 BYTE  *pOutLine = pOutBuff;
 DWORD dwInLn = (3 * wWidth + 3) & ~3;
 DWORD dwOutLn = (3 * wNewWidth + 3) & ~3;
 int   x, y, i, ii;
 BOOL  bCrossRow, bCrossCol;
 int   *pRowCoeff = CreateCoeffInt(wWidth,
                                   wNewWidth,
                                   TRUE);
 int   *pColCoeff = CreateCoeffInt(wHeight,
                                   wNewHeight,
                                   TRUE);
 int   *pXCoeff, *pYCoeff = pColCoeff;
 DWORD dwBuffLn = 3 * wNewWidth * sizeof(DWORD);
 DWORD *pdwBuff = new DWORD[6 * wNewWidth];
 DWORD *pdwCurrLn = pdwBuff,
       *pdwCurrPix,
       *pdwNextLn = pdwBuff + 3 * wNewWidth;
 DWORD dwTmp, *pdwNextPix;

 ::ZeroMemory(pdwBuff, 2 * dwBuffLn);

 y = 0;
 while(y < wNewHeight)
 {
  pPix = pLine;
  pLine += dwInLn;

  pdwCurrPix = pdwCurrLn;
  pdwNextPix = pdwNextLn;

  x = 0;
  pXCoeff = pRowCoeff;
  bCrossRow = pYCoeff[1] > 0;
  while(x < wNewWidth)
  {
   dwTmp = *pXCoeff * *pYCoeff;
   for(i = 0; i < 3; i++)
    pdwCurrPix[i] += dwTmp * pPix[i];
   bCrossCol = pXCoeff[1] > 0;
   if(bCrossCol)
   {
    dwTmp = pXCoeff[1] * *pYCoeff;
    for(i = 0, ii = 3; i < 3; i++, ii++)
     pdwCurrPix[ii] += dwTmp * pPix[i];
   }
   if(bCrossRow)
   {
    dwTmp = *pXCoeff * pYCoeff[1];
    for(i = 0; i < 3; i++)
     pdwNextPix[i] += dwTmp * pPix[i];
    if(bCrossCol)
    {
     dwTmp = pXCoeff[1] * pYCoeff[1];
     for(i = 0, ii = 3; i < 3; i++, ii++)
      pdwNextPix[ii] += dwTmp * pPix[i];
    }
   }
   if(pXCoeff[1])
   {
    x++;
    pdwCurrPix += 3;
    pdwNextPix += 3;
   }
   pXCoeff += 2;
   pPix += 3;
  }
  if(pYCoeff[1])
  {
   // set result line
   pdwCurrPix = pdwCurrLn;
   pPix = pOutLine;
   for(i = 3 * wNewWidth; i > 0; i--, pdwCurrPix++, pPix++)
    *pPix = ((LPBYTE)pdwCurrPix)[3];

   // prepare line buffers
   pdwCurrPix = pdwNextLn;
   pdwNextLn = pdwCurrLn;
   pdwCurrLn = pdwCurrPix;
   ::ZeroMemory(pdwNextLn, dwBuffLn);

   y++;
   pOutLine += dwOutLn;
  }
  pYCoeff += 2;
 }

 delete [] pRowCoeff;
 delete [] pColCoeff;
 delete [] pdwBuff;
}

///////////////////////////////////////////////////////////

void EnlargeDataInt(BYTE *pInBuff,
                    WORD wWidth,
                    WORD wHeight,
                    BYTE *pOutBuff,
                    WORD wNewWidth,
                    WORD wNewHeight)
{
 BYTE  *pLine = pInBuff,
       *pPix = pLine,
       *pPixOld,
       *pUpPix,
       *pUpPixOld;
 BYTE  *pOutLine = pOutBuff, *pOutPix;
 DWORD dwInLn = (3 * wWidth + 3) & ~3;
 DWORD dwOutLn = (3 * wNewWidth + 3) & ~3;
 int   x, y, i;
 BOOL  bCrossRow, bCrossCol;
 int   *pRowCoeff = CreateCoeffInt(wNewWidth,
                                   wWidth,
                                   FALSE);
 int   *pColCoeff = CreateCoeffInt(wNewHeight,
                                   wHeight,
                                   FALSE);
 int   *pXCoeff, *pYCoeff = pColCoeff;
 DWORD dwTmp, dwPtTmp[3];

 y = 0;
 while(y < wHeight)
 {
  bCrossRow = pYCoeff[1] > 0;
  x = 0;
  pXCoeff = pRowCoeff;
  pOutPix = pOutLine;
  pOutLine += dwOutLn;
  pUpPix = pLine;
  if(pYCoeff[1])
  {
   y++;
   pLine += dwInLn;
   pPix = pLine;
  }

  while(x < wWidth)
  {
   bCrossCol = pXCoeff[1] > 0;
   pUpPixOld = pUpPix;
   pPixOld = pPix;
   if(pXCoeff[1])
   {
    x++;
    pUpPix += 3;
    pPix += 3;
   }

   dwTmp = *pXCoeff * *pYCoeff;

   for(i = 0; i < 3; i++)
    dwPtTmp[i] = dwTmp * pUpPixOld[i];

   if(bCrossCol)
   {
    dwTmp = pXCoeff[1] * *pYCoeff;
    for(i = 0; i < 3; i++)
    dwPtTmp[i] += dwTmp * pUpPix[i];
   }

   if(bCrossRow)
   {
    dwTmp = *pXCoeff * pYCoeff[1];
    for(i = 0; i < 3; i++)
    dwPtTmp[i] += dwTmp * pPixOld[i];
    if(bCrossCol)
    {
     dwTmp = pXCoeff[1] * pYCoeff[1];
     for(i = 0; i < 3; i++)
     dwPtTmp[i] += dwTmp * pPix[i];
    }
   }

   for(i = 0; i < 3; i++, pOutPix++)
    *pOutPix = ((LPBYTE)(dwPtTmp + i))[3];

   pXCoeff += 2;
  }
  pYCoeff += 2;
 }

 delete [] pRowCoeff;
 delete [] pColCoeff;
}

// end src
