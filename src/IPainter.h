#ifndef IPAINTER_H
#define IPAINTER_H

#include "PlatformDef.h"
#include "GeoPoint.h"

//! Abstract class for painting map objects 
struct IPainter
{
	//! Start painting polygon
	virtual void StartPolygon(UInt uiType, const wchar_t * wcName) = 0;
	//! Start painting polyline
	virtual void StartPolyline(UInt uiType, const wchar_t * wcName) = 0;
	//! Finish painting polygon
	virtual void FinishObject() = 0;
	//! Add point to currently painted object
	virtual void AddPoint(const GeoPoint & gp) = 0;
	//! Check if will paint object
	virtual bool WillPaint(const GeoRect & rect) = 0;
	//! Set view center coordinates
	virtual void SetView(const GeoPoint & gp, bool fManual) = 0;
	//! Paint one point
	virtual void PaintPoint(UInt uiType, const GeoPoint & gp, const wchar_t * wcName) = 0;
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
