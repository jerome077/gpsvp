/*
Copyright (c) 2009, Jerome077
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GPX.h"
#include "..\Common.h"
#include <vector>
#include <sstream>

// To identify a GPX file as one written by gpsVP and containung only waypoints (can be overwritten)
static const std::wstring g_wstrWaypointFileMark = L"gpsVP file only for waypoints";

// ---------------------------------------------------------------
// CDOMGPXWaypoint
// ---------------------------------------------------------------

bool CDOMGPXWaypoint::operator==(const CDOMGPXWaypoint& wpt2) const
{
	return (m_pXMLNode.GetInterfacePtr() == wpt2.m_pXMLNode.GetInterfacePtr());
}

// ---------------------------------------------------------------

double CDOMGPXWaypoint::getLongitude()
{
	try
	{
		XmlNamedNodeMap pAttrList = m_pXMLNode->attributes;
		XmlNode pAttr = pAttrList->getNamedItem(L"lon");
		std::string sVal = (const char*)pAttr->text;
		return atof(sVal.c_str());
	}
	catch (_com_error e)
	{
		return 0.0;
	}
}

// ---------------------------------------------------------------

double CDOMGPXWaypoint::getLatitude()
{
	try
	{
		XmlNamedNodeMap pAttrList = m_pXMLNode->attributes;
		XmlNode pAttr = pAttrList->getNamedItem(L"lat");
		std::string sVal = (const char*)pAttr->text;
		return atof(sVal.c_str());
	}
	catch (_com_error e)
	{
		return 0.0;
	}
}

// ---------------------------------------------------------------

bool CDOMGPXWaypoint::hasField(const std::wstring& FieldName)
{
	XmlNode pNode = m_pXMLNode->selectSingleNode(FieldName.c_str());
	return (pNode);
}

// ---------------------------------------------------------------

std::wstring CDOMGPXWaypoint::getField(const std::wstring& FieldName)
{
	XmlNode pNode = m_pXMLNode->selectSingleNode(FieldName.c_str());
	if (pNode)
		return (const wchar_t*)pNode->text;
	else
		return L"";
}

// ---------------------------------------------------------------

bool CDOMGPXWaypoint::hasExtensionField(const std::wstring& FieldName)
{
	XmlNode pExtNode = m_pXMLNode->selectSingleNode(L"extensions");
	if (pExtNode)
	{
		XmlNode pNode = pExtNode->selectSingleNode(FieldName.c_str());
		return (pNode);
	}
	else
		return false;
}

// ---------------------------------------------------------------

std::wstring CDOMGPXWaypoint::getExtensionField(const std::wstring& FieldName)
{
	XmlNode pExtNode = m_pXMLNode->selectSingleNode(L"extensions");
	if (pExtNode)
	{
		XmlNode pNode = pExtNode->selectSingleNode(FieldName.c_str());
		if (pNode)
			return (const wchar_t*)pNode->text;
		else
			return L"";
	}
	else
		return L"";
}

// ---------------------------------------------------------------

std::wstring CGPXWaypoint::getName() // "" when not available
{
	return getField(L"name");
}

// ---------------------------------------------------------------

double CDOMGPXWaypoint::getAltitude()   // -777 when not available
{
	XmlNode pNode = m_pXMLNode->selectSingleNode(L"ele");
	if (pNode)
	{
		std::string sVal = (const char*)pNode->text;
		if (!sVal.empty())
			return atof(sVal.c_str());
		else
			return -777.0;
	}
	else
	{
		return -777.0;
	}
}

// ---------------------------------------------------------------

int CGPXWaypoint::getRadius()        // 0 when not available
{
	return _wtoi(getExtensionField(L"gpsVP:radius").c_str());
}

// ---------------------------------------------------------------

std::auto_ptr<CGPXField> CDOMGPXWaypoint::firstField()
{
	m_pXMLNodeCurrentField = m_pXMLNode->firstChild;
	return std::auto_ptr<CGPXField>(new CDOMGPXField(m_pXMLNodeCurrentField));
}

// ---------------------------------------------------------------

std::auto_ptr<CGPXField> CDOMGPXWaypoint::nextField()
{
	m_pXMLNodeCurrentField = m_pXMLNodeCurrentField->nextSibling;
	if (m_pXMLNodeCurrentField)
	{
		std::wstring sNodeName = (const wchar_t*)m_pXMLNodeCurrentField->nodeName;
		if (L"extensions" == sNodeName)
			m_pXMLNodeCurrentField = m_pXMLNodeCurrentField->firstChild;
	}
	return std::auto_ptr<CGPXField>(new CDOMGPXField(m_pXMLNodeCurrentField));
}

// ---------------------------------------------------------------
// CDOMGPXField
// ---------------------------------------------------------------

std::wstring CDOMGPXField::getName()
{
	return (const wchar_t*)m_pNode->nodeName;
}

// ---------------------------------------------------------------

std::wstring CDOMGPXField::getValue()
{
	return (const wchar_t*)m_pNode->text;
}

// ---------------------------------------------------------------

std::wstring CDOMGPXField::getAttribute(const std::wstring& AttrName)
{
	XmlNamedNodeMap pAttrList = m_pNode->attributes;
	XmlNode pAttr = pAttrList->getNamedItem(AttrName.c_str());
	if (!pAttr)	return L"";
	return (const wchar_t*)pAttr->text;
}

// ---------------------------------------------------------------
// CGPXTrackPoint
// ---------------------------------------------------------------

CGPXTrackPoint::CGPXTrackPoint(XmlNode pNode)
	: CDOMGPXElem(pNode)
{
	try
	{
		XmlNamedNodeMap pAttrList = m_pNode->attributes;
		XmlNode pAttr = pAttrList->getNamedItem(L"lon");
		std::string sVal = (const char*)pAttr->text;
		mLongitude = atof(sVal.c_str());
	}
	catch (_com_error e)
	{
		mLongitude = 0.0;
	}
	try
	{
		XmlNamedNodeMap pAttrList = m_pNode->attributes;
		XmlNode pAttr = pAttrList->getNamedItem(L"lat");
		std::string sVal = (const char*)pAttr->text;
		mLatitude = atof(sVal.c_str());
	}
	catch (_com_error e)
	{
		mLatitude = 0.0;
	}
	try
	{
		XmlNode pTimeNode = m_pNode->selectSingleNode(L"time");
		if (pTimeNode)
		{
			// *** Use an xsd:dateTime-Parser
			SYSTEMTIME st;
			int nReadFields = swscanf((const wchar_t*)pTimeNode->text, L"%d-%d-%dT%d:%d:%dZ",
				&st.wYear, &st.wMonth, &st.wDay, &st.wHour, &st.wMinute, &st.wSecond);
			if (nReadFields == 6)
			{
				if (!SystemTimeToVariantTime(&st, &mUTCTime))
					mUTCTime = 0.0;
			}
			else
				mUTCTime = 0.0;
		}
		else
			mUTCTime = 0.0;
	}
	catch (_com_error e)
	{
		mUTCTime = 0.0;
	}

}

// ---------------------------------------------------------------
// CGPXTrackSeg
// ---------------------------------------------------------------

std::auto_ptr<CGPXTrackPoint> CGPXTrackSeg::firstTrackPoint()
{
	m_pXMLTrackPointNodeList = m_pNode->selectNodes(L"trkpt");
	return std::auto_ptr<CGPXTrackPoint>(new CGPXTrackPoint(m_pXMLTrackPointNodeList->nextNode()));
}

// ---------------------------------------------------------------

std::auto_ptr<CGPXTrackPoint> CGPXTrackSeg::nextTrackPoint()
{
	return std::auto_ptr<CGPXTrackPoint>(new CGPXTrackPoint(m_pXMLTrackPointNodeList->nextNode()));
}

// ---------------------------------------------------------------
// CGPXTrack
// ---------------------------------------------------------------

std::wstring CGPXTrack::getName()
{
	XmlNode pNodeName = m_pNode->selectSingleNode(L"name");
	if (!pNodeName) return L"";
	return (const wchar_t*)pNodeName->text;
}

// ---------------------------------------------------------------

std::auto_ptr<CGPXTrackSeg> CGPXTrack::firstTrackSeg()
{
	m_pXMLTrackSegNodeList = m_pNode->selectNodes(L"trkseg");
	return std::auto_ptr<CGPXTrackSeg>(new CGPXTrackSeg(m_pXMLTrackSegNodeList->nextNode()));
}

// ---------------------------------------------------------------

std::auto_ptr<CGPXTrackSeg> CGPXTrack::nextTrackSeg()
{
	return std::auto_ptr<CGPXTrackSeg>(new CGPXTrackSeg(m_pXMLTrackSegNodeList->nextNode()));
}


// ---------------------------------------------------------------
// CGPXFileReader
// ---------------------------------------------------------------

CGPXFileReader::WaypointIterator CGPXFileReader::m_WaypointEnd(NULL);

// ---------------------------------------------------------------

CGPXFileReader::CGPXFileReader(const std::wstring& wstrFilename)
	: m_filename(wstrFilename),
	  m_pXMLDoc(MSXML::CLSID_DOMDocument)
{
	m_pXMLDoc->put_validateOnParse(VARIANT_FALSE);
	m_pXMLDoc->put_resolveExternals(VARIANT_FALSE);
	m_pXMLDoc->put_preserveWhiteSpace(VARIANT_FALSE);

	VARIANT_BOOL bLoadOk = m_pXMLDoc->load(_variant_t(m_filename.c_str()));
	if (!bLoadOk)
	{
		XmlParseError pError = m_pXMLDoc->parseError;
		throw CGPXFileReader::Error((const wchar_t*)pError->reason);
	}
	else
	{
		m_pElemGPX = m_pXMLDoc->selectSingleNode(L"gpx");
	}
}

// ---------------------------------------------------------------

bool CGPXFileReader::IsGpsVPWaypointFile()
{
	XmlNode pNodeMeta = m_pElemGPX->selectSingleNode(L"metadata");
	if (!pNodeMeta) return false;
	XmlNode pNodeDesc = pNodeMeta->selectSingleNode(L"desc");
	if (!pNodeDesc) return false;
	return (g_wstrWaypointFileMark == (const wchar_t*)pNodeDesc->text);
}

// ---------------------------------------------------------------

CGPXFileReader::~CGPXFileReader()
{
}

// ---------------------------------------------------------------

CGPXFileReader::WaypointIterator CGPXFileReader::WaypointBegin()
{
	XmlNodeList pWptList = m_pElemGPX->selectNodes(L"wpt");
	return WaypointIterator(pWptList);
}

// ---------------------------------------------------------------

std::auto_ptr<CGPXTrack> CGPXFileReader::firstTrack()
{
	m_pXMLTrackNodeList = m_pElemGPX->selectNodes(L"trk");
	return std::auto_ptr<CGPXTrack>(new CGPXTrack(m_pXMLTrackNodeList->nextNode()));
}

// ---------------------------------------------------------------

std::auto_ptr<CGPXTrack> CGPXFileReader::nextTrack()
{
	return std::auto_ptr<CGPXTrack>(new CGPXTrack(m_pXMLTrackNodeList->nextNode()));
}

// ---------------------------------------------------------------
// CGPXFileReader::WaypointIterator
// ---------------------------------------------------------------

CGPXFileReader::WaypointIterator::WaypointIterator(XmlNodeList pWPTList)
: m_pWPTList(pWPTList),
  m_currentWpt()
{
	if (m_pWPTList)
	{
		XmlNode pNode = m_pWPTList->nextNode();
		m_currentWpt.ReleaseOldAndAssign(pNode);
	}
}

// ---------------------------------------------------------------

bool CGPXFileReader::WaypointIterator::operator==(const CGPXFileReader::WaypointIterator& iter2) const
{
	return (m_currentWpt == iter2.m_currentWpt);
}

// ---------------------------------------------------------------

bool CGPXFileReader::WaypointIterator::operator!=(const CGPXFileReader::WaypointIterator& iter2) const
{
	return !(m_currentWpt == iter2.m_currentWpt);
}

// ---------------------------------------------------------------

CGPXWaypoint* CGPXFileReader::WaypointIterator::operator*()
{
	return &m_currentWpt;
}

// ---------------------------------------------------------------

CGPXWaypoint* CGPXFileReader::WaypointIterator::operator->()
{
	return &m_currentWpt;
}

// ---------------------------------------------------------------

CGPXFileReader::WaypointIterator& CGPXFileReader::WaypointIterator::operator++()
{
	XmlNode pNode = m_pWPTList->nextNode();
	m_currentWpt.ReleaseOldAndAssign(pNode); // At the end 'node' is NULL
	return *this;
}

// ---------------------------------------------------------------
// CGPXWaypointWriter
// ---------------------------------------------------------------

CGPXWaypointWriter::CGPXWaypointWriter(double dLatitude, double dLongitude)
: m_wstrXml(L"<wpt lat=\""),
  bWithExtensions(false)
{
	m_wstrXml += DoubleToText(dLatitude, 12);
	m_wstrXml += L"\" lon=\"";
	m_wstrXml += DoubleToText(dLongitude, 12);
	m_wstrXml += L"\">";
}

// ---------------------------------------------------------------

void CGPXWaypointWriter::addField(const std::wstring& FieldName, const std::wstring& FieldValue)
{
	m_wstrXml += L"<" + FieldName + L">" + FieldValue + L"</" + FieldName + L">";
}

// ---------------------------------------------------------------

// Should be used after all 'addField' but I don't check.
void CGPXWaypointWriter::setToExtensions()
{
	if (!bWithExtensions)
	{
		m_wstrXml += L"<extensions>";
		bWithExtensions = true;
	}
}

// ---------------------------------------------------------------

void CGPXWaypointWriter::addAltitude(const double FieldValue)
{
	if (-777 != FieldValue) // -777 means "unknown" in plt format
	{
		addField(L"ele", DoubleToText(FieldValue, 0));
	}
}

// ---------------------------------------------------------------

void CGPXWaypointWriter::addRadiusEx(const int FieldValue)
{
	if (0 != FieldValue)
	{
		setToExtensions();
		addField(L"gpsVP:radius", IntToText(FieldValue));
	}
}

// ---------------------------------------------------------------

void CGPXWaypointWriter::addOSMTag(const std::wstring& Key, const std::wstring& FieldValue)
{
	if (Key.length() > 0)
	{
		setToExtensions();
		m_wstrXml += L"<gpsVP:osm k=\"" + Key + L"\" v=\"" + FieldValue + L"\"/>";
	}
}

// ---------------------------------------------------------------

std::wstring CGPXWaypointWriter::Done()
{
	if (bWithExtensions)
	{
		m_wstrXml += L"</extensions>";
	}
	m_wstrXml += L"</wpt>\n";
	return m_wstrXml;
}


// ---------------------------------------------------------------
// CGPXFileWriter
// ---------------------------------------------------------------

CGPXFileWriter::CGPXFileWriter(const std::wstring& wstrFilename, const std::wstring& wstrCreator)
: m_CurrentWpt(NULL)
{
	//if (0 != _wfopen_s(&m_pFile, wstrFilename.c_str(), L"wt, ccs=UTF-8"))
	//	throw CGPXFileWriter::Error(std::wstring(L"Error while opening file"));
	m_pFile = wfopen(wstrFilename.c_str(), L"wb");
	if (!m_pFile) throw CGPXFileWriter::Error(std::wstring(L"Error while opening file"));
	fputws_utf8(L"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n");
	std::wstring wstrLine = L"<gpx version=\"1.1\" creator=\""+wstrCreator+L"\" xmlns:gpsVP=\"http://code.google.com/p/gpsvp/\">\n";
	fputws_utf8(wstrLine.c_str());
	wstrLine = L"<metadata><desc>"+g_wstrWaypointFileMark+L"</desc></metadata>\n";
	fputws_utf8(wstrLine.c_str());
}

// ---------------------------------------------------------------

void CGPXFileWriter::fputws_utf8(const std::wstring& wstrToWrite)
{
	int charcount = WideCharToMultiByte(CP_UTF8, 0, wstrToWrite.c_str(), wstrToWrite.length(), 0, 0, 0, 0);
    char* tempStr = new char[charcount+1];
	WideCharToMultiByte(CP_UTF8, 0, wstrToWrite.c_str(), wstrToWrite.length(), tempStr, charcount, 0, 0);
	tempStr[charcount] = '\0';
	fputs(tempStr, m_pFile);
    delete [] tempStr;
}

// ---------------------------------------------------------------

CGPXFileWriter::~CGPXFileWriter()
{
	if (m_pFile)
	{
		if (m_CurrentWpt)
		{
			fputws_utf8(m_CurrentWpt->Done().c_str());
			delete m_CurrentWpt;
		}
		fputws_utf8(L"</gpx>\n");
		fclose(m_pFile);
	}
}

// ---------------------------------------------------------------

void CGPXFileWriter::AddNextWaypoint(double dLatitude, double dLongitude)
{
	// flush the previous waypoint to the file 
	if (m_CurrentWpt)
	{
		fputws_utf8(m_CurrentWpt->Done().c_str());
		delete m_CurrentWpt;
	}

	// and set a new waypoint as current waypoint
	m_CurrentWpt = new CGPXWaypointWriter(dLatitude, dLongitude);
}

// ---------------------------------------------------------------
