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