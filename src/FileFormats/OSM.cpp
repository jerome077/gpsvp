/*
Copyright (c) 2009, Jerome077
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "OSM.h"
#include "..\Common.h"

// ---------------------------------------------------------------
// COSMWaypointWriter
// ---------------------------------------------------------------

COSMWaypointWriter::COSMWaypointWriter(int id, double dLatitude, double dLongitude)
: m_wstrXml(L"<node id=\""),
  bWithExtensions(false)
{
	m_wstrXml += IntToText(id);
	m_wstrXml += L"\" lat=\"";
	m_wstrXml += DoubleToText(dLatitude, 12);
	m_wstrXml += L"\" lon=\"";
	m_wstrXml += DoubleToText(dLongitude, 12);
	m_wstrXml += L"\" visible=\"true\">";
}

// ---------------------------------------------------------------

void COSMWaypointWriter::addTag(const std::wstring& FieldName, const std::wstring& FieldValue)
{
	m_wstrXml += L"<tag k=\"" + FieldName + L"\" v=\"" + FieldValue + L"\"/>";
}

// ---------------------------------------------------------------

std::wstring COSMWaypointWriter::Done()
{
	m_wstrXml += L"</node>\n";
	return m_wstrXml;
}


// ---------------------------------------------------------------
// COSMFileWriter
// ---------------------------------------------------------------

COSMFileWriter::COSMFileWriter(const std::wstring& wstrFilename, const std::wstring& wstrCreator)
: m_CurrentWpt(NULL),
  m_CurrentId(0)
{
	m_pFile = wfopen(wstrFilename.c_str(), L"wb");
	if (!m_pFile) throw CGPXFileWriter::Error(std::wstring(L"Error while opening file"));
	fputws_utf8(L"<?xml version='1.0' encoding='UTF-8'?>\n");
	std::wstring wstrLine = L"<osm version='0.5' generator='"+wstrCreator+L"'>\n";
	fputws_utf8(wstrLine.c_str());
}

// ---------------------------------------------------------------

void COSMFileWriter::fputws_utf8(const std::wstring& wstrToWrite)
{
	int charcount = WideCharToMultiByte(CP_UTF8, 0, wstrToWrite.c_str(), wstrToWrite.length(), 0, 0, 0, 0);
    char* tempStr = new char[charcount+1];
	WideCharToMultiByte(CP_UTF8, 0, wstrToWrite.c_str(), wstrToWrite.length(), tempStr, charcount, 0, 0);
	tempStr[charcount] = '\0';
	fputs(tempStr, m_pFile);
    delete [] tempStr;
}

// ---------------------------------------------------------------

COSMFileWriter::~COSMFileWriter()
{
	if (m_pFile)
	{
		if (m_CurrentWpt)
		{
			fputws_utf8(m_CurrentWpt->Done().c_str());
			delete m_CurrentWpt;
		}
		fputws_utf8(L"</osm>\n");
		fclose(m_pFile);
	}
}

// ---------------------------------------------------------------

void COSMFileWriter::AddNextWaypoint(double dLatitude, double dLongitude)
{
	// flush the previous waypoint to the file 
	if (m_CurrentWpt)
	{
		fputws_utf8(m_CurrentWpt->Done().c_str());
		delete m_CurrentWpt;
	}

	// and set a new waypoint as current waypoint
	m_CurrentId--;
	m_CurrentWpt = new COSMWaypointWriter(m_CurrentId, dLatitude, dLongitude);
}

// ---------------------------------------------------------------
