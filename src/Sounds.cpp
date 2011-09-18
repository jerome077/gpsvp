/*
Copyright (c) 2005-2011, Jerome077 and others
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Sounds.h"
#include "File.h"
#include "SimpleIniExt.h"

extern HINSTANCE g_hInst;

// ---------------------------------------------------------------

CSoundPlayer::CSoundPlayer()
	: m_iLastSoundAltude(-777),
	  m_SndGPS(L"DEFAULT"), m_SndProximity(L"DEFAULT"),
	  m_WithSndAltitude(false)
{
};

// ---------------------------------------------------------------

void CSoundPlayer::Init(const std::wstring& wstrWavBasePath)
{
	m_wtsrWavBasePath = wstrWavBasePath;

	// Checking if there is an ini file:
	std::wstring wstrIniFilename = m_wtsrWavBasePath + L"Sounds.ini";
	CSimpleIniExtW iniFile;
    if (SI_OK == iniFile.LoadAnsiOrUtf8File(wstrIniFilename.c_str()))
	{
		m_SndGPS = iniFile.GetValue(L"System", L"GPS", L"DEFAULT");
		m_SndProximity = iniFile.GetValue(L"System", L"Proximity", L"DEFAULT");
		if (iniFile.GetSectionSize(L"Altitude") > 0)
		{
			m_WithSndAltitude = true;
			m_SndAltitudeX000 = iniFile.GetValue(L"Altitude", L"X000", L"");
			m_SndAltitudeX100 = iniFile.GetValue(L"Altitude", L"X100", L"");
			m_SndAltitudeX200 = iniFile.GetValue(L"Altitude", L"X200", L"");
			m_SndAltitudeX300 = iniFile.GetValue(L"Altitude", L"X300", L"");
			m_SndAltitudeX400 = iniFile.GetValue(L"Altitude", L"X400", L"");
			m_SndAltitudeX500 = iniFile.GetValue(L"Altitude", L"X500", L"");
			m_SndAltitudeX600 = iniFile.GetValue(L"Altitude", L"X600", L"");
			m_SndAltitudeX700 = iniFile.GetValue(L"Altitude", L"X700", L"");
			m_SndAltitudeX800 = iniFile.GetValue(L"Altitude", L"X800", L"");
			m_SndAltitudeX900 = iniFile.GetValue(L"Altitude", L"X900", L"");
		}
	}
}

// ---------------------------------------------------------------

void CSoundPlayer::PlayDefaultSound()
{
	PlaySound(L"ProximitySound", g_hInst, SND_RESOURCE | SND_ASYNC);
}

// ---------------------------------------------------------------

void CSoundPlayer::PlayFileSound(const std::wstring& wstrWavFile)
{
	if (wstrWavFile.empty()) return;
	std::wstring wstrWavFullname = m_wtsrWavBasePath + wstrWavFile;
	if (FileExist(wstrWavFullname.c_str()))
		PlaySound(wstrWavFullname.c_str(), g_hInst, SND_FILENAME | SND_ASYNC);
	else
		PlayDefaultSound();
}

// ---------------------------------------------------------------

void CSoundPlayer::PlaySoundGPS()
{
	PlayFileSound(m_SndGPS);
}

// ---------------------------------------------------------------

void CSoundPlayer::PlaySoundProximity()
{
	PlayFileSound(m_SndProximity);
}

// ---------------------------------------------------------------

void CSoundPlayer::PlaySoundAltitude(double dAltitude)
{
	if (m_WithSndAltitude)
	{
		int iAlt = int(dAltitude + 0.5);
		int iDelta = iAlt - m_iLastSoundAltude;
		if ((iDelta >= 50) || (iDelta <= -50))
		{
			m_iLastSoundAltude = 50*int((iAlt+25)/50);
			switch (m_iLastSoundAltude % 1000)
			{
			case 0:
				PlayFileSound(m_SndAltitudeX000);
				break;
			case 100:
				PlayFileSound(m_SndAltitudeX100);
				break;
			case 200:
				PlayFileSound(m_SndAltitudeX200);
				break;
			case 300:
				PlayFileSound(m_SndAltitudeX300);
				break;
			case 400:
				PlayFileSound(m_SndAltitudeX400);
				break;
			case 500:
				PlayFileSound(m_SndAltitudeX500);
				break;
			case 600:
				PlayFileSound(m_SndAltitudeX600);
				break;
			case 700:
				PlayFileSound(m_SndAltitudeX700);
				break;
			case 800:
				PlayFileSound(m_SndAltitudeX800);
				break;
			case 900:
				PlayFileSound(m_SndAltitudeX900);
				break;
			}
		}
	}
}

// ---------------------------------------------------------------
