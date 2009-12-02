#ifndef screentogeo_h
#define screentogeo_h

#include "RegValues.h"
#include "screenpoint.h"
#include "GeoPoint.h"
#include "IPainter.h"

typedef void* HWND;

class CScreenToGeo : public IPainter
{
private:
	//! coordinate of window center
	ScreenPoint m_spWindowCenter;
	//! Window rect
	ScreenRect m_srWindow;
	int m_cos100;
	int m_sin100;
	//! Scale of x axis for the latitude
	long m_lXScale100;
	//! Scale in garmin points per screen point
	CRegScalar<int, REG_DWORD> m_ruiScale10;
	CRegScalar<GeoPoint, REG_BINARY> m_gpCenter;
	int m_uiScale10Cache;
	GeoPoint m_gpCenterView;
	GeoPoint m_gpCenterCache;
	bool m_fViewSet;
	int m_rotate; // the map on screen is rotated m_rotate degrees CCW
	int m_iManualTimer;

	enum { 
		ciMinZoom = 1,		//!< Minimum zoom
		ciMaxZoom = 100000	//!< Maximum zoom
	};
public:
    CScreenToGeo()
    {
        m_iManualTimer = 0;
    }
    void Init(HWND hWnd, HKEY hRegKey)
    {
        m_gpCenter.Init(hRegKey, L("Center"), GeoPoint(0, 0));
        if(abs(m_gpCenter().lon) > 1 << (GPWIDTH - 1) || abs(m_gpCenter().lat) > 1 << (GPWIDTH - 2))
        {
            // If Center is out of bounds try to guess the rigth value 
            // assuming the wrong value is due to smaller GPWIDTH
            int lat = m_gpCenter().lat;
            int lon = m_gpCenter().lon;
            while(abs(lon) > 1 << (GPWIDTH - 1) || abs(lat) > 1 << (GPWIDTH - 2))
            {
                lat >>= 1;
                lon >>= 1;
            }
            m_gpCenter.Set(GeoPoint(lon, lat));
        }
        m_fViewSet = false;
        m_ruiScale10.Init(hRegKey, L("ScaleD"), 500);
        m_ruiScale10.Set((std::max)((int)(ciMinZoom), m_ruiScale10()));
        m_ruiScale10.Set((std::min)((int)(ciMaxZoom), m_ruiScale10()));

        m_lXScale100 = cos100(m_gpCenter().lat);
        m_srWindow.Init(ScreenPoint(0,0));
        m_srWindow.Append(ScreenPoint(1,1));
        PrepareScales();
    }
    virtual void Redraw() = 0;

    GeoPoint ScreenToGeo(const ScreenPoint & pt)
    {
        AutoLock l;
        GeoPoint res;
        int dx1 = (pt.x - m_spWindowCenter.x);
        int dy1 = (pt.y - m_spWindowCenter.y);

        int dx2;
        int dy2;
        if (m_cos100 != 100 || m_sin100 != 0)
        {
            dx2 = (dx1 * m_cos100 - dy1 * m_sin100) / 100;
            dy2 = (dx1 * m_sin100 + dy1 * m_cos100) / 100;
        }
        else
        {
            dx2 = dx1;
            dy2 = dy1;
        }

        //res.lon = dx2 * m_ruiScale10() / 10 * 100 / m_lXScale100 + m_gpCenter().lon;
        //res.lat = - dy2 * m_ruiScale10() / 10 + m_gpCenter().lat;
        res = GeoPoint(
            int(((int64_t)(dx2) * m_ruiScale10() * 10 << (GPWIDTH - 24)) / m_lXScale100),
            int(-((int64_t)(dy2) * m_ruiScale10() << (GPWIDTH - 24)) / 10));
        res.lon += m_gpCenter().lon;
        res.lat += m_gpCenter().lat;
        return res;
    }
    ScreenPoint GeoToScreen(const GeoPoint & pt)
    {
        std::cerr << "GeoToScreen, " << m_uiScale10Cache << std::endl;
        AutoLock l;
        ScreenPoint res;
        int dx1 = int((int64_t)(pt.lon - m_gpCenterCache.lon) * m_lXScale100 >> (GPWIDTH - 24)) / 10 /* * 10 / 100 */ / m_uiScale10Cache;
        int dy1 = int((int64_t)(m_gpCenterCache.lat - pt.lat) * 10  >> (GPWIDTH - 24)) / m_uiScale10Cache;

        int dx2;
        int dy2;
        if (m_cos100 != 100 || m_sin100 != 0)
        {
            dx2 = (dx1 * m_cos100 + dy1 * m_sin100) / 100;
            dy2 = (- dx1 * m_sin100 + dy1 * m_cos100) / 100;
        }
        else
        {
            dx2 = dx1;
            dy2 = dy1;
        }

        int dx = dx2 + m_spWindowCenter.x;
        int dy = dy2 + m_spWindowCenter.y;

        if (dx > 1000000) dx = 1000000;
        if (dx < -1000000) dx = -1000000;
        if (dy > 1000000) dy = 1000000;
        if (dy < -1000000) dy = -1000000;

        res.x = dx;
        res.y = dy;
        return res;
    }
    ScreenRect GeoToScreen(const GeoRect & rect)
    {
        ScreenRect res;
        res.Init(GeoToScreen(GeoPoint(rect.minLon, rect.minLat)));
        res.Append(GeoToScreen(GeoPoint(rect.maxLon, rect.maxLat)));
        res.Append(GeoToScreen(GeoPoint(rect.minLon, rect.maxLat)));
        res.Append(GeoToScreen(GeoPoint(rect.maxLon, rect.minLat)));
        return res;
    }
    GeoRect ScreenToGeo(const ScreenRect & rect)
    {
        GeoRect res;
        GeoPoint gp1 = ScreenToGeo(ScreenPoint(rect.left, rect.top));
        GeoPoint gp2 = ScreenToGeo(ScreenPoint(rect.left, rect.bottom));
        GeoPoint gp3 = ScreenToGeo(ScreenPoint(rect.right, rect.top));
        GeoPoint gp4 = ScreenToGeo(ScreenPoint(rect.right, rect.bottom));
        res.minLat = (std::min)((std::min)(gp1.lat, gp2.lat), (std::min)(gp3.lat, gp4.lat));
        res.minLon = (std::min)((std::min)(gp1.lon, gp2.lon), (std::min)(gp3.lon, gp4.lon));
        res.maxLat = (std::max)((std::max)(gp1.lat, gp2.lat), (std::max)(gp3.lat, gp4.lat));
        res.maxLon = (std::max)((std::max)(gp1.lon, gp2.lon), (std::max)(gp3.lon, gp4.lon));
        return res;
    }
    const GeoPoint GetCenter() 
    {
        return m_gpCenter();
    }
    void SetView(const GeoPoint & gp, bool fManual)
    {
        AutoLock l;
#ifndef LINUX
        if (fManual && app->m_Options[mcoFollowCursor])
            m_iManualTimer = 60;
        else if (m_iManualTimer > 0)
            return;
#endif // LINUX
        GeoPoint actual = gp;
        const int cnMaxLat = 0x1B << (GPWIDTH - 7); // ~75 degrees
        const int cnMaxLon = 1 << (GPWIDTH - 1);
        if (actual.lat > cnMaxLat)
            actual.lat = cnMaxLat;
        if (actual.lat < -cnMaxLat)
            actual.lat = -cnMaxLat;
        if (actual.lon > cnMaxLon)
            actual.lon = cnMaxLon;
        if (actual.lon < -cnMaxLon)
            actual.lon = -cnMaxLon;

        m_gpCenterView = actual;
        m_fViewSet = true;
        Redraw();
    }
	//! Get-method for m_uiScale
	int GetScale() {return m_ruiScale10();}
	ScreenRect GetScreenRect(){return m_srWindow;}
	int GetScreenRotationAngle() {return m_rotate;}
    double GetXScale() 
    { 
        return double(m_uiScale10Cache) / m_lXScale100 * 10;
    }
	bool IsVertical(){return m_srWindow.right - m_srWindow.left < m_srWindow.bottom - m_srWindow.top;}
	void OnTimer() {AutoLock l; if (m_iManualTimer > 0) --m_iManualTimer;}
	bool ManualMode() {AutoLock l; return m_iManualTimer > 0;}
	void ResetManualMode() {AutoLock l; m_iManualTimer = 0;}
    void ZoomIn()
    {
        if (m_ruiScale10() == ciMinZoom)
            return;
        // Decrease scale twice
        // Correct if minimum reached
        m_ruiScale10.Set((std::max)((int)(ciMinZoom), m_ruiScale10() / 2));
        Redraw();
    }
    void ZoomOut()
    {
        if (m_ruiScale10() == ciMaxZoom)
            return;
        // Increase scale twice
        // Correct if maximum reached
        m_ruiScale10.Set((std::min)((int)(ciMaxZoom), m_ruiScale10() * 2));
        Redraw();
    }
    void Left()
    {
        // Move view left by 30 screen points
        Move(ScreenDiff(30, 0));
    }
    void Right()
    {
        // Move view right by 30 screen points
        Move(ScreenDiff(-30, 0));
    }
    void Up()
    {
        // Move view up by 30 screen points
        Move(ScreenDiff(0, 30));
    }
    void Down()
    {
        // Move view down by 30 screen points
        Move(ScreenDiff(0, -30));
    }
    void Move(ScreenDiff d)
    {
        if (d.Null() == true)
            return;
        // Move view by given number of screen points
        SetView(ScreenToGeo(m_spWindowCenter - d), true);
    }
    void PrepareScales()
    {
        m_gpCenterCache = m_gpCenter();
        m_uiScale10Cache = m_ruiScale10();
        m_lXScale100 = cos100(m_gpCenterCache.lat);
    }
    const GeoPoint& GetCenterC() {return m_gpCenterCache;}
    const UInt GetScale10C() {return m_uiScale10Cache;}
    const long GetXScale100C() {return m_lXScale100;}

};

#endif // screentogeo_h
