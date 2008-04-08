#ifndef WAYPOINTS_H
#define WAYPOINTS_H

#include "PlatformDef.h"
#include <list>
#include <string>
#include <algorithm>
#include "IPainter.h"
#include "GeoPoint.h"

using namespace std;

class CWaypoints
{
public:
	class CPoint
	{
	protected:
		void Cache()
		{
			m_poscache = GeoPoint(m_dLongitude, m_dLatitude);
		}

		double m_dLongitude;
		double m_dLatitude;
		GeoPoint m_poscache;
		wstring m_wstrName;
		Int m_iId;
		double m_dLastUsed;
		int m_iRadius;
		bool m_fInProximity;
		int m_iAltitude;
	public:
		CPoint(double dLon, double dLat, int iAltitude, const wchar_t * wcName);
		void Paint(IPainter * pPainter, const GeoPoint * pgp);
		void Write(FILE * pFile, int nNum);
		void AddToList(IListAcceptor * pAcceptor);
		void AddToList(IListAcceptor2 * pAcceptor);
		Int GetID() {return m_iId;}
		wstring GetLabel() {return m_wstrName;}
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
	};
protected:
	list<CPoint> m_Points;
	wstring m_wstrFilename;
public:
	Int AddPoint(GeoPoint gp, int iAltitude, const wchar_t * wcName, int iRadius);
	Int AddPoint(const CPoint & pt);
	int GetNearestPoint(GeoPoint gp, double dRadius);
	void Write();
	void Read(const wchar_t * wcFilename);
	void Paint(IPainter * pPainter, const GeoPoint * pgp);
	void GetList(IListAcceptor * pAcceptor, GeoPoint gpCenter, int iRadius);
	void GetList(IListAcceptor2 * pAcceptor);
	wstring GetLabelByID(int iId);
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
};

#endif // WAYPOINTS_H
