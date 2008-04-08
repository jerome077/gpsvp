#ifndef SUN_H_INCLUDED
#define SUN_H_INCLUDED

#include "Monitors.h"

struct CSun
{
	CSun() : m_fSet(false) {}
	CTimeMonitor m_monSunrise;
	CTimeMonitor m_monSunset;
	CTextMonitor m_monDaytime;
	void Fix(const CTimeMonitor & monTime, const GeoPoint & p);
	bool m_fSet;
	bool m_fLastSet;
};

#endif // SUN_H_INCLUDED
