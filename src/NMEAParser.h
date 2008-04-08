#ifndef NMEAPARSER_H
#define NMEAPARSER_H

#include "PlatformDef.h"
#include "Common.h"
#include <string>
#include <vector>
#include <map>
#include <stdio.h>
#include "GeoPoint.h"
#include <stdio.h>
#include "Monitors.h"
#include "Lock.h"
#include "MonitorSet.h"

using namespace std;

struct IGPSClient
{
	enum enumConnectionStatus 
	{
		csNotConnected = 1,
		csNoFix = 2,
		csFix = 3,
		csPaused = 4,
		csDisabled = 5
	};
	virtual void NoFix() = 0;
	virtual void Fix(GeoPoint gp, double dHDOP) = 0;
	virtual void NoVFix() = 0;
	virtual void VFix(double dAltitude) = 0;
	virtual void SetConnectionStatus(enumConnectionStatus iStatus) = 0;
};
class CTrack;

//! NMEA stream parser
class CNMEAParser
{
private:
	//! Client for setting coordinates
	IGPSClient * m_pClient;
	//! Object constants
	enum enumConstants
	{
		cnMaxCommand = 100 //!< Max command length
	};
	//! Current command
	string m_strCommand;
	//! Is command started
	bool m_fCommandStarted;
	map<string, string> m_mapCommands;
	int m_iSatNum;
	map<int, int> m_mapSats;
	CSpeedMonitor m_dSpeed;
	CSpeedMonitor m_dMaxSpeed;
	CTextMonitor m_monStatus;
	CTimeMonitor m_monTime;
	CMemoryMonitor m_monGPSData;
	CTextMonitor m_monRawTime;
	std::wstring m_wstrFilename;
	Byte m_fileBuffer[4096];
	UInt m_fileBufferPos;
public:
	//! Constructor
	CNMEAParser();
	void SetClient(IGPSClient * pClient);
	//! Comand is read completely, parse it
	void CommandComplete();
	void CommandStarted();
	void ResetCommand();
	void AddData(const Byte * data, UInt uiLen);
	void NewStream();
	void ConnectionDisabled();
	void GetList(IListAcceptor * pAcceptor);
	void SaveCommands(const wchar_t * wstrFilename);
	void Pause();
	void DebugShowTime();
	void PaintSatellites(IMonitorPainter * pPainter);
	void InitMonitors(CMonitorSet & set, HKEY hRegKey, bool fDebugMode);
	void SetTime();
	void SetFilename(wchar_t * wcFilename);
	std::wstring GetFilename();
	const CTimeMonitor & GetTimeMonitor() const { return m_monTime; }
};

#endif // NMEAPARSER_H
