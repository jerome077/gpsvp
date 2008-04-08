#include "TypeInfo.h"
#include "Common.h"

byte Char22Byte(char * pos)
{
	char buffer[3];
	buffer[0] = pos[0];
	buffer[1] = pos[1];
	buffer[2] = 0;
	byte res = byte(strtol(buffer, 0, 16));
	return res;
}

char * SkipToNextLine(char * pos)
{
	while (*pos && (*pos != '\r') && (*pos != '\n')) ++pos;
	while (*pos && (*pos == '\r' || *pos == '\n')) ++pos;
	return pos;
}

wstring GetLabel(char * pos)
{
	while (*pos && *pos != '\r' && *pos != '\n' && *pos != '\t') ++pos;
	if (*pos != '\t')
		return L"";
	++pos;
	char * start = pos;
	while (*pos && *pos != '\r' && *pos != '\n') ++pos;
	wchar_t buffer[1000];
	int iRes = MultiByteToWideChar(CP_ACP, MB_USEGLYPHCHARS, start, pos - start, buffer, 1000);
	buffer[iRes] = 0;
	return buffer;
}

void CTypeInfo::Parse(HINSTANCE hInst)
{
	HRSRC hResource = FindResource(hInst, L"Types", RT_RCDATA);
	HGLOBAL hGlobal = LoadResource(hInst, hResource);
	char * data = (char *)LockResource(hGlobal);
	if (data)
	{
		enumObjTypes eType = maskPoints;
		while (*data)
		{
			switch (*data)
			{
			case '[':
				{
					if (!strncmp("[RGN10", data, 6))
						eType = maskPoints;
					else if (!strncmp("[RGN40", data, 6))
						eType = maskPolylines;
					else if (!strncmp("[RGN80", data, 6))
						eType = maskPolygons;
				}
				break;
			case '0':
				{
					if (eType == maskPoints)
					{
						wstring wstrLabel = GetLabel(data);
						byte bClass1 = Char22Byte(data + 2);
						if (data[6] == '-')
						{
							byte bClass2 = Char22Byte(data + 9);
							for (byte b = bClass1; b <= bClass2; ++b)
							{
								m_PointTypes[b].wstrLabel = wstrLabel;
							}
						}
						else
						{
							byte bSubClass1 = Char22Byte(data + 4);
							m_PointTypes[bClass1].subTypes[bSubClass1] = wstrLabel;
						}
					}
					if (eType == maskPolylines)
					{
						wstring wstrLabel = GetLabel(data);
						byte bClass = Char22Byte(data + 2);
						m_PolylineTypes[bClass] = wstrLabel;
					}
					if (eType == maskPolygons)
					{
						wstring wstrLabel = GetLabel(data);
						byte bClass = Char22Byte(data + 2);
						m_PolygonTypes[bClass] = wstrLabel;
					}
				}
				break;
			}
			data = SkipToNextLine(data);
		}
	}
}

wstring CTypeInfo::PointType(int iType)
{
	byte bClass = (iType >> 8) & 0xff;
	byte bSubClass = iType & 0xff;
	PointTypes::iterator it1 = m_PointTypes.find(bClass);
	if (it1 != m_PointTypes.end())
	{
		GenericTypes::iterator it2 = it1->second.subTypes.find(bSubClass);
		if (it2 != it1->second.subTypes.end())
			return it2->second;
		return it1->second.wstrLabel;
	}
	return wstring(L("Unknown type")) + L" (" + IntToText(iType) + L")";
}

wstring CTypeInfo::PolylineType(int iType)
{
	GenericTypes::iterator it = m_PolylineTypes.find(iType);
	if (it != m_PolylineTypes.end())
		return it->second;
	return wstring(L("Unknown type"))+ L" (" + IntToText(iType) + L")";
}

wstring CTypeInfo::PolygonType(int iType)
{
	GenericTypes::iterator it = m_PolygonTypes.find(iType);
	if (it != m_PolygonTypes.end())
		return it->second;
	return wstring(L("Unknown type"))+ L" (" + IntToText(iType) + L")";
}
