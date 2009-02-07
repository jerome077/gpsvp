/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "NMEAParser.h"
#include "DebugOutput.h"

//! Constructor
CNMEAParser::CNMEAParser() : 
	m_fCommandStarted(false), 
	m_iSatNum(0), 
	m_pClient(0)
{
	m_monStatus = L"-";
	m_monTime.Reset();
	m_monRawTime = L"-";
}

void CNMEAParser::SetClient(IGPSClient * pClient)
{
	m_pClient = pClient;
}

//! Comand is read completely, parse it
void CNMEAParser::CommandComplete()
{
	// Vector for parameters
	vector<string> listParts;
	// Reset command started flag
	m_fCommandStarted = false;
	// Start from the beginning
	string::size_type pos = 0;
	string::size_type nextpos = 0;
	// Look for commas
	while ((nextpos = m_strCommand.find(',', pos)) != string::npos)
	{
		// Add text between commas to list
		listParts.push_back(m_strCommand.substr(pos, nextpos - pos));
		// Advance to position after comma
		pos = nextpos + 1;
	}
	// Add text after the last comma
	listParts.push_back(m_strCommand.substr(pos));

	m_mapCommands[listParts[0]] = m_strCommand;

	// Now analyze the command
	// Coordinates from GPS device
	if (listParts[0] == "GPGGA" && listParts.size() >= 10)
	{
		//check fix quality and loc number availablity before we go any further
		double fixQuality = myatof(listParts[6].c_str());
		if ( (fixQuality == 0) || (listParts[2].length() < 5 || listParts[4].length() < 5) )
		{
			// If no number then no track and no cursor
			m_pClient->NoFix();
			m_monStatus = L("No fix");
			m_pClient->SetConnectionStatus(IGPSClient::csNoFix);
			m_monTime.Reset();
			m_monRawTime = L"-";
		}
		else
		{
			// Else get coordinates
			if (listParts[9].size() != 0)
			{
				double dAltitude = myatof(listParts[9].c_str());
				m_pClient->VFix(dAltitude);
			}
			else
			{
				m_pClient->NoVFix();
			}
			double dLatitude = myatof(listParts[2].substr(2).c_str()) / 60 + atof(listParts[2].substr(0, 2).c_str());
			if (listParts[3] == "S")
				dLatitude = -dLatitude;
			double dLongitude = myatof(listParts[4].substr(3).c_str()) / 60 + atof(listParts[4].substr(0, 3).c_str());
			if (listParts[5] == "W")
				dLongitude = -dLongitude;
			double dHDOP = myatof(listParts[8].c_str());
			m_pClient->Fix(GeoPoint(dLongitude, dLatitude), dHDOP);
			m_monStatus = L("Fix");
			m_pClient->SetConnectionStatus(IGPSClient::csFix);
		}
	}
	if (listParts[0] == "GPGSV" && listParts.size() >= 20)
	{
		m_iSatNum = atoi(listParts[3].c_str());
		int iPartNum = atoi(listParts[2].c_str());
		for (int i = (iPartNum - 1) * 4; i < iPartNum * 4 && i < m_iSatNum; ++i)
		{
			m_mapSats[i] = atoi(listParts[(i - (iPartNum - 1) * 4) * 4 + 7].c_str());
		}
	}
	if (listParts[0] == "GPRMC" && listParts.size() >= 10)
	{
		m_dSpeed.Set(myatof(listParts[7].c_str()) * cdNauticalMile);
		if (m_dSpeed.Get() > m_dMaxSpeed.Get())
			m_dMaxSpeed.Set(m_dSpeed.Get());
		if (listParts[1].length() >= 6 && listParts[9].length() >= 6)
		{
			// And time
			m_monTime.Set(
				atoi(listParts[9].substr(4, 2).c_str()) + 2000,
				atoi(listParts[9].substr(2, 2).c_str()),
				atoi(listParts[9].substr(0, 2).c_str()),
				atoi(listParts[1].substr(0, 2).c_str()),
				atoi(listParts[1].substr(2, 2).c_str()),
				atoi(listParts[1].substr(4, 2).c_str()));
			m_monRawTime = a2w(listParts[1].c_str()).c_str();
		}
	}
}
void CNMEAParser::CommandStarted()
{
	// Clear buffer for command
	m_strCommand.resize(0);
	// Check flag
	m_fCommandStarted = true;
}
void CNMEAParser::ResetCommand()
{
	// No, no command text
	m_fCommandStarted = false;
}
void CNMEAParser::AddData(const Byte * data, UInt uiLen)
{
	AutoLock l;
	
	if (!m_wstrFilename.empty())
	{
		const Byte * d = data;
		UInt i = uiLen;
		while (i)
		{
			UInt toWrite = mymin(i, 4096 - m_fileBufferPos);
			memcpy(m_fileBuffer + m_fileBufferPos, d, toWrite);
			m_fileBufferPos += toWrite;
			d += toWrite;
			i -= toWrite;
			if (m_fileBufferPos == 4096)
			{
				FILE * file = wfopen(m_wstrFilename.c_str(), L"ab");
				if (file)
				{
					fwrite(m_fileBuffer, 1, m_fileBufferPos, file);
					fclose(file);
				}
				m_fileBufferPos = 0;
			}
		}
	}

	m_monGPSData += uiLen;
	// We parse all the data
	while (uiLen > 0)
	{
		UInt uiPos = 0;
		// Command may be continued
		if (m_fCommandStarted)
		{
			// Find length of string with good symbols
			while ((uiPos < uiLen) && (data[uiPos] > 0x20) && (data[uiPos] < 0x80))
				++uiPos;
			// Append the string to command text
			m_strCommand.append((char*)data, uiPos);
			// Command may have finished
			if (uiPos < uiLen)
			{
				// Parse it
				CommandComplete();
				// And skip terminating character
				++uiPos;
			}
			// Command text has limited length
			if (m_strCommand.length() > cnMaxCommand)
				ResetCommand();
		}
		else
		{
			while ((uiPos < uiLen) && (data[uiPos] != '$'))
				++uiPos;
			if (uiPos < uiLen)
			{
				CommandStarted();
				++uiPos;
			}
		}
		data += uiPos;
		uiLen -= uiPos;
	}
		
}

void CNMEAParser::NewStream()
{
	AutoLock l;
	m_dSpeed.Reset();
	m_pClient->NoFix();
	m_pClient->NoVFix();
	m_monStatus = L("No");
	m_pClient->SetConnectionStatus(IGPSClient::csNotConnected);
	m_monTime.Reset();
	m_monRawTime = L"-";
}

void CNMEAParser::ConnectionDisabled()
{
	AutoLock l;
	m_monStatus = L("Disabled");
	m_pClient->SetConnectionStatus(IGPSClient::csDisabled);
}

void CNMEAParser::GetList(IListAcceptor * pAcceptor)
{
	AutoLock l;
	for (map<string, string>::iterator it = m_mapCommands.begin(); it != m_mapCommands.end(); ++it)
	{
		wchar_t wstr[1000];
		swprintf(wstr, 1000, L"%S", it->second.c_str());
		pAcceptor->AddItem(wstr, 0);
	}
}
void CNMEAParser::SaveCommands(const wchar_t * wstrFilename)
{
	AutoLock l;
	FILE * pFile = wfopen(wstrFilename, L"wt");
	if (!pFile)
		return;
	for (map<string, string>::iterator it = m_mapCommands.begin(); it != m_mapCommands.end(); ++it)
		fprintf(pFile, "%s\n", it->second.c_str());
	fclose(pFile);
}
void CNMEAParser::Pause()
{
	AutoLock l;
	m_monStatus = L("Paused");
	m_monTime.Reset();
	m_monRawTime = L"-";
	m_pClient->SetConnectionStatus(IGPSClient::csPaused);
	ResetCommand();
}
void CNMEAParser::DebugShowTime()
{
	AutoLock l;
	SYSTEMTIME st;
	GetSystemTime(&st);
	m_monTime.Set(st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
}

void CNMEAParser::PaintSatellites(IMonitorPainter * pPainter)
{
	AutoLock l;
	ScreenPoint spSize = pPainter->GetMonitorSize();
	spSize.x -= 2;
	spSize.y -= 4;
	int iCount = max(m_iSatNum, 12);
	for (int i = 0; i < m_iSatNum; ++i)
	{
		ScreenPoint spFrom = ScreenPoint(2 + spSize.x * i / iCount, 2 + spSize.y + 1);
		ScreenPoint spTo = ScreenPoint(2 + spSize.x * (i + 1) / iCount - 1, 2 + spSize.y * (100 - m_mapSats[i]) / 100);
		ScreenRect srBar;
		srBar.Init(spFrom);
		srBar.Append(spTo);
		pPainter->DrawBar(srBar);
	}
}

void CNMEAParser::InitMonitors(CMonitorSet & set, HKEY hRegKey, bool fDebugMode)
{
	AutoLock l;
	m_dSpeed.SetIdL(L"Speed");
	set.AddMonitor(&m_dSpeed);

	m_dMaxSpeed.SetIdL(L"Max speed");
	m_dMaxSpeed.SetRegistry(hRegKey, L"MaxSpeed");
	m_dMaxSpeed.SetResetable();
	set.AddMonitor(&m_dMaxSpeed);

	m_monStatus.SetIdL(L"Connection");
	set.AddMonitor(&m_monStatus);

	m_monTime.SetIdL(L"GPS time");
	set.AddMonitor(&m_monTime);

	m_monGPSData.SetIdL(L"GPS data");
	set.AddMonitor(&m_monGPSData);
	m_monGPSData = 0;

	m_monRawTime.SetIdL(L"Raw GPS time");
	if (fDebugMode)
		set.AddMonitor(&m_monRawTime);
}

void CNMEAParser::SetTime()
{
	AutoLock l;
	m_monTime.SetTime();
}

void CNMEAParser::SetFilename(wchar_t * wcFilename)
{
	AutoLock l;
	m_wstrFilename = wcFilename;
	m_fileBufferPos = 0;
}

std::wstring CNMEAParser::GetFilename()
{
	AutoLock l;
	return m_wstrFilename;
}
