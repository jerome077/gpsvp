/*
Copyright (c) 2009, Jerome077
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef OSM_H
#define OSM_H

#include "GPX.h"

// ---------------------------------------------------------------

class COSMWaypointWriter
{
protected:
	std::wstring m_wstrXml;
	bool bWithExtensions;
public:
	COSMWaypointWriter(int id, double dLatitude, double dLongitude);

    void addTag(const std::wstring& TagName, const std::wstring& TagValue);

	// To get the XML string for this waypoint
	std::wstring Done();
};

// ---------------------------------------------------------------

// To export a waypoint file as osm file (Open Street Map)
// Siehe http://wiki.openstreetmap.org/wiki/OSM_Protocol_Version_0.5
class COSMFileWriter : public CGPXFile
{
public:
	COSMFileWriter(const std::wstring& wstrFilename, const std::wstring& wstrCreator);
	~COSMFileWriter();

	// flush the previous waypoint to the file and set a new waypoint as current waypoint.
	void AddNextWaypoint(double dLatitude, double dLongitude);
	COSMWaypointWriter* CurrentWpt() { return m_CurrentWpt; };

protected:
	void fputws_utf8(const std::wstring& wstrToWrite);
	FILE* m_pFile;
	COSMWaypointWriter* m_CurrentWpt;
	int m_CurrentId;
};

// ---------------------------------------------------------------

#endif // OSM_H
