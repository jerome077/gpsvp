/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


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
	virtual void VFix(double dAltitude, double dSeparation) = 0;
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
	std::string m_strCommand;
	//! Is command started
	bool m_fCommandStarted;
	std::map<std::string, std::string> m_mapCommands;
	int m_iSatNum;
	std::map<int, int> m_mapSats;
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
