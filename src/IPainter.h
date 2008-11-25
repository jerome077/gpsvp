/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef IPAINTER_H
#define IPAINTER_H

#include "PlatformDef.h"
#include "GeoPoint.h"

//! Abstract class for painting map objects 
struct IPainter
{
	//! Start painting polygon
	virtual void StartPolygon(UInt uiType, const tchar_t * wcName) = 0;
	//! Start painting polyline
	virtual void StartPolyline(UInt uiType, const tchar_t * wcName) = 0;
	//! Finish painting polygon
	virtual void FinishObject() = 0;
	//! Add point to currently painted object
	virtual void AddPoint(const GeoPoint & gp) = 0;
	//! Check if will paint object
	virtual bool WillPaint(const GeoRect & rect) = 0;
	//! Set view center coordinates
	virtual void SetView(const GeoPoint & gp, bool fManual) = 0;
	//! Paint one point
	virtual void PaintPoint(UInt uiType, const GeoPoint & gp, const tchar_t * wcName) = 0;
	virtual void SetLabelMandatory() = 0;
	virtual GeoRect GetRect() = 0;
};

struct IButtonPainter
{
	virtual void AddButton(const wchar_t * wcLabel, int iCommand, bool fSelected) = 0;
};

struct IStatusPainter
{
	virtual void PaintText(const wchar_t * wcText) = 0;
	virtual void SetProgressItems(int iLevel, int iCount) = 0;
	virtual void SetProgress(int iLevel, int iProgress) = 0;
	virtual void Advance(int iLevel) = 0;
};

#endif // IPAINTER_H
