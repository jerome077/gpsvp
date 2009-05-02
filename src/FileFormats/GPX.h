/*
Copyright (c) 2009, Jerome077
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GPX_H
#define GPX_H

// Can't use msxml2 or better because it doesn't seems to be on SP2003 => only msxml
#import <msxml.dll> named_guids
#include <string>
#include <algorithm>

// ---------------------------------------------------------------

// Define some smart pointers for MSXML
typedef MSXML::IXMLDOMNodePtr                  XmlNode;
typedef MSXML::IXMLDOMDocumentPtr              XmlDocument;
typedef MSXML::IXMLDOMElementPtr               XmlElement;
typedef MSXML::IXMLDOMAttributePtr             XmlAttribute;
typedef MSXML::IXMLDOMNamedNodeMapPtr          XmlNamedNodeMap;
typedef MSXML::IXMLDOMNodeListPtr              XmlNodeList;
typedef MSXML::IXMLDOMParseErrorPtr            XmlParseError;

// Helper class to init COM
class ComInit
{
public:
    ComInit()  { CoInitializeEx(NULL, COINIT_MULTITHREADED); }
    ~ComInit() { CoUninitialize(); }
};

// ---------------------------------------------------------------

class CDOMGPXElem
{
protected:
	XmlNode m_pNode;
	CDOMGPXElem(XmlNode pNode) : m_pNode(pNode) {};
public:
	bool eof() { return (!m_pNode); };
};

// ---------------------------------------------------------------

class CGPXField
{
public:
	virtual bool eof() = 0;
	virtual std::wstring getName() = 0;
	virtual std::wstring getValue() = 0;
	virtual std::wstring getAttribute(const std::wstring& AttrName) = 0;
	virtual std::wstring getOSMKey() { return getAttribute(L"k"); };
	virtual std::wstring getOSMValue() { return getAttribute(L"v"); };
};

// ---------------------------------------------------------------
class CGPXWaypoint
{
public:
	virtual double getLongitude() = 0;
	virtual double getLatitude() = 0;

	// For tags directly under <wpt>:
	virtual bool hasField(const std::wstring& FieldName) = 0;
	virtual std::wstring getField(const std::wstring& FieldName) = 0;
	// For tags in the <extensions> part of <wpt>:
	virtual bool hasExtensionField(const std::wstring& FieldName) = 0;
	virtual std::wstring getExtensionField(const std::wstring& FieldName) = 0;

	// Standard fields for gpsVP-Waypoints => extra functions to avoid 'getField'
	// <name>:
	virtual bool hasName() { return hasField(L"name"); };
	virtual std::wstring getName(); // "" when not available
	// <ele>:
	virtual bool hasAltitude() { return hasField(L"ele"); };
	virtual double getAltitude() = 0;   // -777 when not available
	// <gpsVP:radius>:
	virtual bool hasRadius() { return hasExtensionField(L"gpsVP:radius"); };
	virtual int getRadius();        // 0 when not available

	// To loop on all tags:
	virtual std::auto_ptr<CGPXField> firstField() = 0;
	virtual std::auto_ptr<CGPXField> nextField() = 0;
};

// ---------------------------------------------------------------

class CDOMGPXField: public CGPXField
{
protected:
	XmlNode m_pNode;
	CDOMGPXField(XmlNode pNode) : m_pNode(pNode) {};
	friend class CDOMGPXWaypoint;
public:
	virtual bool eof() { return (!m_pNode); };
	virtual std::wstring getName();
	virtual std::wstring getValue();
	virtual std::wstring getAttribute(const std::wstring& AttrName);
};

// ---------------------------------------------------------------

class CDOMGPXWaypoint: public CGPXWaypoint
{
public:
	bool operator==(const CDOMGPXWaypoint& wpt2) const;
	virtual double getLongitude();
	virtual double getLatitude();

	virtual bool hasField(const std::wstring& FieldName);
	virtual std::wstring getField(const std::wstring& FieldName);
	virtual bool hasExtensionField(const std::wstring& FieldName);
	virtual std::wstring getExtensionField(const std::wstring& FieldName);

	virtual double getAltitude();

	virtual std::auto_ptr<CGPXField> firstField();
	virtual std::auto_ptr<CGPXField> nextField();

	CDOMGPXWaypoint() : m_pXMLNode(NULL) {};
	void ReleaseOldAndAssign(XmlNode pNode)
	{
		m_pXMLNode = pNode;
	};

private:
	XmlNode m_pXMLNode;
	XmlNode m_pXMLNodeCurrentField;
};

// ---------------------------------------------------------------

class CGPXTrackPoint: public CDOMGPXElem
{
protected:
	double mLongitude, mLatitude;
	double mUTCTime;
	friend class CGPXTrackSeg;
	CGPXTrackPoint(XmlNode pNode);
public:
	virtual double getLongitude() { return mLongitude; };
	virtual double getLatitude()  { return mLatitude; };
	virtual double getUTCTime()   { return mUTCTime; }; // as "VariantTime"
};

// ---------------------------------------------------------------

class CGPXTrackSeg: public CDOMGPXElem
{
protected:
	XmlNodeList m_pXMLTrackPointNodeList;
	friend class CGPXTrack;
	CGPXTrackSeg(XmlNode pNode) : CDOMGPXElem(pNode) {};
public:
	virtual std::auto_ptr<CGPXTrackPoint> firstTrackPoint();
	virtual std::auto_ptr<CGPXTrackPoint> nextTrackPoint();
};

// ---------------------------------------------------------------

class CGPXTrack: public CDOMGPXElem
{
protected:
	XmlNodeList m_pXMLTrackSegNodeList;
	friend class CGPXFileReader;
	CGPXTrack(XmlNode pNode) : CDOMGPXElem(pNode) {};
public:
	std::wstring getName();
	virtual std::auto_ptr<CGPXTrackSeg> firstTrackSeg();
	virtual std::auto_ptr<CGPXTrackSeg> nextTrackSeg();
};

// ---------------------------------------------------------------

class CGPXFile
{
public:
	// To be able to throw exceptions:
	class Error
	{
		std::wstring m_Msg;
	public:
		Error(const std::wstring& msg) : m_Msg(msg) {};
		Error(const Error& r) : m_Msg(r.m_Msg) {};
		const wchar_t* c_str() { return m_Msg.c_str(); };
		std::wstring& operator()() { return m_Msg; };
	};
};

// ---------------------------------------------------------------

// Reader currently based on an XML DOM parser but could be replaced through
// an XML SAX parser to increase speed.
class CGPXFileReader : public CGPXFile
{
public:
	CGPXFileReader(const std::wstring& wstrFilename);
	~CGPXFileReader();
	// As I write the file directly, I want to be sure that it only contains waypoints:
	bool IsGpsVPWaypointFile();

	// Access to waypoints:
	// So an iterator is too much, two functions first/next would be enough, but now it's already written...
	class WaypointIterator
	{
	public:
		bool operator==(const WaypointIterator& iter2) const;
		bool operator!=(const WaypointIterator& iter2) const;
		CGPXWaypoint* operator*();
		CGPXWaypoint* operator->();
		WaypointIterator& operator++();
	protected:
		WaypointIterator(XmlNodeList pWPTList);
		friend class CGPXFileReader;
	private:
		XmlNodeList m_pWPTList;
		CDOMGPXWaypoint m_currentWpt;
	};
	WaypointIterator WaypointBegin();
	WaypointIterator& WaypointEnd() { return m_WaypointEnd; };

	// Access to tracks:
	virtual std::auto_ptr<CGPXTrack> firstTrack();
	virtual std::auto_ptr<CGPXTrack> nextTrack();

private:
	std::wstring m_filename;
	XmlDocument m_pXMLDoc;
	XmlNode m_pElemGPX;
	static WaypointIterator m_WaypointEnd;
	XmlNodeList m_pXMLTrackNodeList;
};

// ---------------------------------------------------------------

class CGPXWaypointWriter
{
protected:
	std::wstring m_wstrXml;
	bool bWithExtensions;
public:
	CGPXWaypointWriter(double dLatitude, double dLongitude);

	// Caution: field order is important, see http://www.topografix.com/GPX/1/1/
    void addField(const std::wstring& FieldName, const std::wstring& FieldValue);
	// For <extensions> fields, which should come after all standard fields
	void setToExtensions();

	// Standard fields for gpsVP-Waypoints => extra functions to avoid 'addField'
	void addAltitude(const double FieldValue);
	void addName(const std::wstring& FieldValue) { addField(L"name", FieldValue); };
	void addRadiusEx(const int FieldValue);
	void addOSMTag(const std::wstring& Key, const std::wstring& FieldValue);

	// To get the XML string for this waypoint
	std::wstring Done();
};

// ---------------------------------------------------------------

// Using an XML DOM parser to write the GPX file would probably be slow
// and use much memory. Especially that the GPX is written again after each
// change. That's why I choose to write the file directly.
class CGPXFileWriter : public CGPXFile
{
public:
	CGPXFileWriter(const std::wstring& wstrFilename, const std::wstring& wstrCreator);
	~CGPXFileWriter();

	// flush the previous waypoint to the file and set a new waypoint as current waypoint.
	void AddNextWaypoint(double dLatitude, double dLongitude);
	CGPXWaypointWriter* CurrentWpt() { return m_CurrentWpt; };

protected:
	void fputws_utf8(const std::wstring& wstrToWrite);
	FILE* m_pFile;
	CGPXWaypointWriter* m_CurrentWpt;
};

// ---------------------------------------------------------------

#endif // GPX_H
