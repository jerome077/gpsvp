#ifndef TYPEINFO_H
#define TYPEINFO_H

#include <Windows.h>
#include <map>
#include <string>
#include "Common.h"

using namespace std;

class CTypeInfo
{
	typedef map<int, wstring> GenericTypes;
	struct PointTypeElement
	{
		PointTypeElement(){}
		wstring wstrLabel; 
		GenericTypes subTypes; 
	};
	typedef map<byte, PointTypeElement> PointTypes;
	PointTypes m_PointTypes;
	GenericTypes m_PolylineTypes;
	GenericTypes m_PolygonTypes;
public:
	void Parse(HINSTANCE hInst);
	wstring PointType(int iType);
	wstring PolylineType(int iType);
	wstring PolygonType(int iType);
};

#endif // TYPEINFO_H