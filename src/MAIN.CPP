#include "Header.h"
#include "DebugOutput.h"
#include "Common.h"
#include "IPainter.h"
#include <windows.h>


// TODO: move into separate file
//! Painter object for testing purposes
class CTestPainter : public IPainter
{
	virtual void StartPolygon(Byte bType, const wchar_t * wcName)
	{
		dout << "Polygon started [" << UInt(bType) << "] {\n";
	}
	virtual void FinishObject()
	{
		dout << "}\n";
	}
	virtual void AddPoint(GeoPoint gp) 
	{
		dout << "\t" << gp << "\n";
	}
	virtual void StartPolyline(Byte bType, const wchar_t * wcName)
	{
		char buff[1000] = {0};
		if (wcName)
			WideCharToMultiByte(CP_OEMCP, 0, wcName, -1, buff, 1000, 0, 0);
		dout << "Polyline started [" << UInt(bType) << ", " << buff << "] {\n";
	}
	virtual bool WillPaint(GeoRect gr)
	{
		return true;
	}
	virtual void SetView(GeoPoint gp) {}
	virtual void SetCursor(GeoPoint gp) {}
	virtual void SetCursor(bool fVisible) {}
	virtual void PaintPoint(UInt uiType, GeoPoint gp, const wchar_t * wcName)
	{
		char buff[1000] = {0};
		if (wcName)
			WideCharToMultiByte(CP_OEMCP, 0, wcName, -1, buff, 1000, 0, 0);
		dout << "Point [" << uiType << ", " << buff << "]: " << gp << "\n";
	}
	virtual void SetLabelMandatory() {};
	virtual GeoRect GetRect()
	{
		return GeoRect(0, 0, 0, 0);
	};
};


int main()
{
	{
		CIMGFile imgfile;
		imgfile.Parse(L"cyprus_by_alex.img");
//		imgfile.Dump();
		CTestPainter painter;
		imgfile.Paint(&painter, 24, maskPoints | maskIndexedPoints);
	}
	/*
	{
		CIMGFile imgfile;
		imgfile.Parse(L"ярославль.img");
//		imgfile.Dump();
		CTestPainter painter;
		imgfile.Paint(&painter, 1, maskPoints | maskIndexedPoints);
	}
	{
		CIMGFile imgfile;
		imgfile.Parse(L"Polyline.img");
//		imgfile.Dump();
//		CTestPainter painter;
//		imgfile.Paint(&painter, 1, maskPoints | maskIndexedPoints);
	}
	{
		CIMGFile imgfile;
		imgfile.Parse(L"Polyline2.img");
//		imgfile.Dump();
//		CTestPainter painter;
//		imgfile.Paint(&painter, 1);
	}
	{
		CIMGFile imgfile;
		imgfile.Parse(L"Polyline3.img");
//		imgfile.Dump();
//		CTestPainter painter;
//		imgfile.Paint(&painter, 1);
	}
	*/
	return 0;
}
