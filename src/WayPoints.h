﻿/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef WAYPOINTS_H
#define WAYPOINTS_H

#include "PlatformDef.h"
#include <list>
#include <string>
#include <algorithm>
#include <vector>
#include <memory>
#include "IPainter.h"
#include "GeoPoint.h"
#include "VersionNumber.h"
#ifndef UNDER_WINE
#include "FileFormats/GPX.h"
#endif // UNDER_WINE

// Waypoints with a name beginning with this prefix are considered as models for other waypoints
const std::wstring WPT_MODEL_PREFIX = L"~";
bool isWptModel(const std::wstring& WaypointName);


class CWaypoints
{
public:
	class CPointEditor;
	static std::wstring GetCreator();
	
	// ---------------------------------------------
	enum enumWaypointPropNameSpace
	{
		nsGPX, // Standard GPX
		nsVP,  // gpsVP specific
		nsOSM  // Open Street Map
	};
	// ---------------------------------------------
	class CPointProp
	{
	public:
		virtual enumWaypointPropNameSpace Namespace() const { return nsGPX; };
		virtual std::wstring Name() const { return L""; };
		virtual std::wstring Value() const { return L""; };
		virtual bool DeleteAllowed() const { return false; };
		virtual void SetValue(const std::wstring& aValue) {};
		virtual void SetName(const std::wstring& aValue) {};
	};
	// - - - - - - - - - - - - - - - - - - - - - - -
    class CStringProp : public CPointProp
	{
		enumWaypointPropNameSpace m_namespace;
		std::wstring m_name;
		std::wstring m_value;
	public:
		CStringProp(const enumWaypointPropNameSpace& aNamespace,
			        const std::wstring& aName,
					const std::wstring& aValue) : m_namespace(aNamespace), m_name(aName), m_value(aValue) {};
		CStringProp(const CStringProp& source) : m_namespace(source.m_namespace), m_name(source.m_name), m_value(source.m_value) {};
		virtual enumWaypointPropNameSpace Namespace() const { return m_namespace; };
		virtual std::wstring Name() const { return m_name; };
		virtual std::wstring Value() const { return m_value; };
		virtual bool DeleteAllowed() const { return true; };
		virtual void SetValue(const std::wstring& aValue) { m_value = aValue; };
		virtual void SetName(const std::wstring& aValue) { m_name = aValue; };
	};
	// ---------------------------------------------
	class CPoint
	{
	protected:
		friend class CWaypoints;
		friend class CPointEditor;
		void Cache()
		{
			m_poscache = GeoPoint(m_dLongitude, m_dLatitude);
		}

		double m_dLongitude;
		double m_dLatitude;
		GeoPoint m_poscache;
		std::wstring m_wstrName;
		Int m_iId;
		double m_dLastUsed;
		int m_iRadius;
		bool m_fInProximity;
		int m_iAltitude;
		std::vector<CStringProp> m_OSMPropList; 
	public:
		CPoint(double dLon, double dLat, int iAltitude, const wchar_t * wcName);
		//CPoint(const CPoint& source);
		CPoint(); // Default constructor for use with 'Assign'
		void Paint(IPainter * pPainter, const GeoPoint * pgp);
		void Write(FILE * pFile, int nNum);
		void AddToList(IListAcceptor * pAcceptor);
		void AddToList(IListAcceptor2 * pAcceptor);
		Int GetID() {return m_iId;}
		std::wstring GetLabel() {return m_wstrName;}
		void SetLabel(const wchar_t * wcLabel) {
			m_wstrName = wcLabel; 
			std::replace(m_wstrName.begin(), m_wstrName.end(), L',', L'.');
		}
		int GetRadius(){return m_iRadius;}
		void SetRadius(int iRadius){m_iRadius = iRadius;}
		double Latitude() {return m_dLatitude;}
		double Longitude() {return m_dLongitude;}
		void Latitude(double d) {m_dLatitude = d; Cache();}
		void Longitude(double d) {m_dLongitude = d; Cache();}
		int GetAltitude(){return m_iAltitude;}
		void Altitude(int iAltitude) {m_iAltitude = iAltitude;}
		GeoPoint GetApproximatePos() const {return m_poscache;}
		void SetApproximatePos(GeoPoint gp) {m_dLongitude = Degree(gp.lon); m_dLatitude = Degree(gp.lat); Cache();}
		double GetUsed() {return m_dLastUsed;}
		bool CheckProximity(GeoPoint gp);
		bool operator == (const CPoint & to) const;
		bool operator == (int to) const;
		CWaypoints::CPointEditor GetEditor();
		// copy and merge (to make a backup before editing):
		void Assign(const CPoint& source);
		// copy and merge the complementary properties (not long, lat, alt...)
		void AssignOSM(const CPoint& source);
	};
	// ---------------------------------------------
	// To access to the properties of a waypoint and be able to apply changes
	// in a single step (because for utm or other coordinates you need x and y
	// to be able to change longitude and latitude)
	class CPointEditor
	{
	protected:
		CPoint& m_pt;
		std::wstring m_wstrLon, m_wstrLat; // Editing lon and lat postponed until commit
	public:
		// - - - - - - - - - - - - - - - - - - - - - - -
		class CPropProxy : public CPointProp
		{
			CPointProp* m_RefProp;
		public:
			CPropProxy(CPointProp* ARefProp) : m_RefProp(ARefProp) {};
			virtual enumWaypointPropNameSpace Namespace() const { return m_RefProp->Namespace(); };
			virtual std::wstring Name() const { return m_RefProp->Name(); };
			virtual std::wstring Value() const { return m_RefProp->Value(); };
			virtual bool DeleteAllowed() const { return m_RefProp->DeleteAllowed(); };
			virtual void SetValue(const std::wstring& aValue) { m_RefProp->SetValue(aValue); };
			virtual void SetName(const std::wstring& aValue) { m_RefProp->SetName(aValue); };
		};
		// - - - - - - - - - - - - - - - - - - - - - - -
		class CLongitudeProp : public CPointProp
		{
			CPoint& m_pt;
			CPointEditor& m_ptEditor;
		public:
			CLongitudeProp(CPoint& aPt, CPointEditor& aPtEditor) : m_pt(aPt), m_ptEditor(aPtEditor) {};
			CLongitudeProp(const CLongitudeProp& source) : m_pt(source.m_pt), m_ptEditor(source.m_ptEditor) {};
			virtual enumWaypointPropNameSpace Namespace() const { return nsGPX; };
			virtual std::wstring Name() const { return CoordLabelLon()+L":"; };
			virtual std::wstring Value() const
			{
				std::wstring wstrLon, wstrLat;
				CoordToText(m_pt.Longitude(), m_pt.Latitude(), wstrLon, wstrLat);
				return wstrLon;
			};
			virtual void SetValue(const std::wstring& aValue) { m_ptEditor.m_wstrLon = aValue; };
		};
		// - - - - - - - - - - - - - - - - - - - - - - -
		class CLatitudeProp : public CPointProp
		{
			CPoint& m_pt;
			CPointEditor& m_ptEditor;
		public:
			CLatitudeProp(CPoint& aPt, CPointEditor& aPtEditor) : m_pt(aPt), m_ptEditor(aPtEditor) {};
//			CLatitudeProp(const CLatitudeProp& source) : m_pt(source.m_pt) {};
			virtual enumWaypointPropNameSpace Namespace() const { return nsGPX; };
			virtual std::wstring Name() const { return CoordLabelLat()+L":"; };
			virtual std::wstring Value() const
			{
				std::wstring wstrLon, wstrLat;
				CoordToText(m_pt.Longitude(), m_pt.Latitude(), wstrLon, wstrLat);
				return wstrLat;
			};
			virtual void SetValue(const std::wstring& aValue) { m_ptEditor.m_wstrLat = aValue; };
		};
		// - - - - - - - - - - - - - - - - - - - - - - -
		class CNameProp : public CPointProp
		{
			CPoint& m_pt;
		public:
			CNameProp(CPoint& aPt) : m_pt(aPt) {};
			CNameProp(const CNameProp& source) : m_pt(source.m_pt) {};
			virtual enumWaypointPropNameSpace Namespace() const { return nsGPX; };
			virtual std::wstring Name() const { return L("Label:"); };
			virtual std::wstring Value() const { return m_pt.GetLabel(); };
			virtual void SetValue(const std::wstring& aValue) { m_pt.SetLabel(aValue.c_str()); };
		};
		// - - - - - - - - - - - - - - - - - - - - - - -
		class CAltitudeProp : public CPointProp
		{
			CPoint& m_pt;
		public:
			CAltitudeProp(CPoint& aPt) : m_pt(aPt) {};
			CAltitudeProp(const CAltitudeProp& source) : m_pt(source.m_pt) {};
			virtual enumWaypointPropNameSpace Namespace() const { return nsGPX; };
			virtual std::wstring Name() const { return L("Altitude:"); };
			virtual std::wstring Value() const { return IntToText(m_pt.GetAltitude()); };
			virtual void SetValue(const std::wstring& aValue) { 	wchar_t *end;
															int i = wcstol(aValue.c_str(), &end, 10);
															if (*end == 0) m_pt.Altitude(i);
														};
		};
		// - - - - - - - - - - - - - - - - - - - - - - -
		class CRadiusProp : public CPointProp
		{
			CPoint& m_pt;
		public:
			CRadiusProp(CPoint& aPt) : m_pt(aPt) {};
			CRadiusProp(const CRadiusProp& source) : m_pt(source.m_pt) {};
			virtual enumWaypointPropNameSpace Namespace() const { return nsVP; };
			virtual std::wstring Name() const { return L("Radius:"); };
			virtual std::wstring Value() const { return IntToText(m_pt.GetRadius()); };
			virtual void SetValue(const std::wstring& aValue) { 	wchar_t *end;
															int i = wcstol(aValue.c_str(), &end, 10);
															if (*end == 0) m_pt.SetRadius(i);
														};
		};
		// - - - - - - - - - - - - - - - - - - - - - - -
	public:
		CPointEditor(CPoint& aPt) : m_pt(aPt) {};
		int GetPropertyCount() const;
		std::auto_ptr<CPointProp> GetPropertyByIndex(int iId);
		CPointProp& AddOSMProp();
		bool RemovePropertyByIndex(int iId);
		void Commit();
	};
	// ---------------------------------------------
protected:
	std::list<CPoint> m_Points;
	std::wstring m_wstrFilename;
	// If it was a GPX files with tracks, we can't write it without losing the tracks => False to avoid overwrite:
	bool m_bCanWrite; 
	// To avoid writing the file too much (to block 'write' until 'EndUpdate' is called):
	int m_BeginUpdateCount;
	bool m_bWriteRequested;
	std::vector<int> m_ModelsIdList;
	void BeginUpdate(); 
	void EndUpdate();
public:
	class UpdateZone
	{
	protected:
		CWaypoints* m_pCtrledObj;
	public:
		UpdateZone(CWaypoints* pCtrledObj) : m_pCtrledObj(pCtrledObj) { m_pCtrledObj->BeginUpdate(); };
		~UpdateZone() { m_pCtrledObj->EndUpdate(); };
	};
public:
	CWaypoints() : m_BeginUpdateCount(0), m_bWriteRequested(false) {};
	Int AddPoint(GeoPoint gp, int iAltitude, const wchar_t * wcName, int iRadius);
	Int AddPoint(const CPoint & pt, int iRadius);
	Int AddPoint(const CPoint & pt);
	int GetNearestPoint(GeoPoint gp, double dRadius);
	void Write();
	void Write(const std::wstring& wstrFilename);
	void WriteWPT(const std::wstring& wstrFilename);
	void WriteGPX(const std::wstring& wstrFilename);
	void WriteOSM(const std::wstring& wstrFilename);
	bool CanWrite() { return m_bCanWrite; };
	void SetNameWPT(const std::wstring& wstrFilename) { m_wstrFilename = wstrFilename; };
	void SetNameGPX(const std::wstring& wstrFilename) { m_wstrFilename = wstrFilename; };
	void Read(const wchar_t * wcFilename);
	void ReadWPT(const wchar_t * wcFilename);
#ifndef UNDER_WINE
	void ReadGPX(const std::wstring& wstrFilename);
#endif // UNDER_WINE
	void Paint(IPainter * pPainter, const GeoPoint * pgp);
	void GetList(IListAcceptor * pAcceptor, GeoPoint gpCenter, int iRadius);
	void GetList(IListAcceptor2 * pAcceptor);
	std::wstring GetLabelByID(int iId);
	int GetRadiusByID(int iId);
	int GetAltitudeByID(int iId);
	void SetLabelByID(int iId, const wchar_t * wcLabel);
	void SetRadiusByID(int iId, int iRadius);
	GeoPoint GetPointByIDApprox(int iId);
	double GetUsedByID(int iId);
	void DeleteByID(int iId);
	void MovePoint(int iId, GeoPoint gp);
	bool CheckProximity(GeoPoint gp);
	void Import(const CWaypoints & from);
	CPoint & ById(int id);
	bool Empty() {return m_Points.empty();};
	UpdateZone UpdateZoneForCodeBlock() { return UpdateZone(this); };
	int GetWaypointModelCount();
	CPoint & GetWaypointModel(int modelIndex);
	std::wstring GetFilename() { return m_wstrFilename; };
	bool IsGPX();
};

// ---------------------------------------------------------------


#endif // WAYPOINTS_H
