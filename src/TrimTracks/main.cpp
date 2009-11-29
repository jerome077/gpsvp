/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "../Track.h"
#include "../Header.h"
#include <string.h>

class CTrimmer : public IPainter
{
private:
	CTrack & m_result;
	GeoRect m_grBound;
public:
	CTrimmer(CTrack &result, GeoRect grBound) : m_result(result), m_grBound(grBound) {}
	virtual void StartPolygon(Byte bType, const wchar_t * wcName) {}
	virtual void StartPolyline(Byte bType, const wchar_t * wcName) {m_result.Break();}
	virtual void FinishObject() {}
	virtual void AddPoint(const GeoPoint & gp) {if (m_grBound.Contain(gp)) m_result.AddPoint(gp);}
	virtual bool WillPaint(const GeoRect & rect) {return true;}
	virtual void SetView(const GeoPoint & gp) {}
	virtual void PaintPoint(UInt uiType, const GeoPoint & gp, const wchar_t * wcName) {}
	virtual void SetLabelMandatory() {}
	virtual GeoRect GetRect() {return m_grBound;}

};

int wmain(int argc, wchar_t ** argv)
{
	wchar_t ** arg = argv + 1;
	CIMGFile imgFile;
	printf("%S\n", *arg); fflush(stdout);
	imgFile.Parse(*arg);
	++ arg;
	CTrack trResult;
	trResult.SetFileName(L"Result.plt");
	trResult.SetColor(0x007FFF);
	CTrimmer trimmer(trResult, imgFile.GetRect());
	for (; *arg; ++arg)
	{
		wprintf(L"%s\n", *arg); fflush(stdout);
		CTrack track;
		track.Read(*arg);
		track.Paint(&trimmer);
	}
	return true;
}

int GetTrackStep()
{
	return 1;
}

#include "../Common.cpp"
#include "../Header.cpp"
#include "../File.cpp"
#include "../TreSubfile.cpp"
#include "../RgnSubfile.cpp"
#include "../LblSubfile.cpp"
#include "../DebugOutput.cpp"
#include "../SubFile.cpp"
#include "../FatBlock.cpp"
#include "../SubDivision.cpp"
#include "../PlatformDef.cpp"
#include "../MapLevel.cpp"
#include "../Track.cpp"