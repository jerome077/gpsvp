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
#include "Common.h"

extern HINSTANCE g_hInst;

// ---------------------------------------------------------------

CSoundPlayer::CSoundPlayer()
	: m_iLastSoundAltude(-777),
	  m_SndGPS(L"DEFAULT"), m_SndProximity(L"DEFAULT"),
	  m_WithSndAltitude(false),
	  m_WithSndUTM(false),
	  m_iLastSoundUtmX(0), m_iLastSoundUtmY(0)
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
		if (iniFile.GetSectionSize(L"UTM") > 0)
		{
			m_WithSndUTM = true;
			m_SndUtmX0000 = iniFile.GetValue(L"UTM", L"X0000", L"");
			m_SndUtmX1000 = iniFile.GetValue(L"UTM", L"X1000", L"");
			m_SndUtmX2000 = iniFile.GetValue(L"UTM", L"X2000", L"");
			m_SndUtmX3000 = iniFile.GetValue(L"UTM", L"X3000", L"");
			m_SndUtmX4000 = iniFile.GetValue(L"UTM", L"X4000", L"");
			m_SndUtmX5000 = iniFile.GetValue(L"UTM", L"X5000", L"");
			m_SndUtmX6000 = iniFile.GetValue(L"UTM", L"X6000", L"");
			m_SndUtmX7000 = iniFile.GetValue(L"UTM", L"X7000", L"");
			m_SndUtmX8000 = iniFile.GetValue(L"UTM", L"X8000", L"");
			m_SndUtmX9000 = iniFile.GetValue(L"UTM", L"X9000", L"");
			m_SndUtmY0000 = iniFile.GetValue(L"UTM", L"Y0000", L"");
			m_SndUtmY1000 = iniFile.GetValue(L"UTM", L"Y1000", L"");
			m_SndUtmY2000 = iniFile.GetValue(L"UTM", L"Y2000", L"");
			m_SndUtmY3000 = iniFile.GetValue(L"UTM", L"Y3000", L"");
			m_SndUtmY4000 = iniFile.GetValue(L"UTM", L"Y4000", L"");
			m_SndUtmY5000 = iniFile.GetValue(L"UTM", L"Y5000", L"");
			m_SndUtmY6000 = iniFile.GetValue(L"UTM", L"Y6000", L"");
			m_SndUtmY7000 = iniFile.GetValue(L"UTM", L"Y7000", L"");
			m_SndUtmY8000 = iniFile.GetValue(L"UTM", L"Y8000", L"");
			m_SndUtmY9000 = iniFile.GetValue(L"UTM", L"Y9000", L"");
			m_SndUtmXUp1 = iniFile.GetValue(L"UTM", L"XUp1", L"");
			m_SndUtmXUp2 = iniFile.GetValue(L"UTM", L"XUp2", L"");
			m_SndUtmXUp3 = iniFile.GetValue(L"UTM", L"XUp3", L"");
			m_SndUtmXDown1 = iniFile.GetValue(L"UTM", L"XDown1", L"");
			m_SndUtmXDown2 = iniFile.GetValue(L"UTM", L"XDown2", L"");
			m_SndUtmXDown3 = iniFile.GetValue(L"UTM", L"XDown3", L"");
			m_SndUtmYUp1 = iniFile.GetValue(L"UTM", L"YUp1", L"");
			m_SndUtmYUp2 = iniFile.GetValue(L"UTM", L"YUp2", L"");
			m_SndUtmYUp3 = iniFile.GetValue(L"UTM", L"YUp3", L"");
			m_SndUtmYDown1 = iniFile.GetValue(L"UTM", L"YDown1", L"");
			m_SndUtmYDown2 = iniFile.GetValue(L"UTM", L"YDown2", L"");
			m_SndUtmYDown3 = iniFile.GetValue(L"UTM", L"YDown3", L"");
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

void CSoundPlayer::PlaySoundUTM(double dLon360, double dLat360, int iUtmZone)
{
	if (m_WithSndUTM)
	{
		int iUtmX, iUtmY, iUtmZone2;
		iUtmZone2 = iUtmZone;
		LongLatToUTM(dLon360, dLat360, iUtmZone2, iUtmX, iUtmY);
		int iDeltaX = iUtmX - m_iLastSoundUtmX;
		int iDeltaY = iUtmY - m_iLastSoundUtmY;

		// Sound for the X coordinate:
		if ((iDeltaX >= 500) || (iDeltaX <= -500))
		{
			m_iLastSoundUtmX = 500*int((iUtmX+250)/500);
			switch (m_iLastSoundUtmX % 10000)
			{
			case 0:
				PlayFileSound(m_SndUtmX0000);
				break;
			case 1000:
				PlayFileSound(m_SndUtmX1000);
				break;
			case 2000:
				PlayFileSound(m_SndUtmX2000);
				break;
			case 3000:
				PlayFileSound(m_SndUtmX3000);
				break;
			case 4000:
				PlayFileSound(m_SndUtmX4000);
				break;
			case 5000:
				PlayFileSound(m_SndUtmX5000);
				break;
			case 6000:
				PlayFileSound(m_SndUtmX6000);
				break;
			case 7000:
				PlayFileSound(m_SndUtmX7000);
				break;
			case 8000:
				PlayFileSound(m_SndUtmX8000);
				break;
			case 9000:
				PlayFileSound(m_SndUtmX9000);
				break;
			}
			// Sound for the exit point:
			int iThirdForY = int((iUtmY%1000)/334);
			switch (iThirdForY)
			{
			case 0:
				if (iDeltaX >= 0) PlayFileSound(m_SndUtmXUp1);
				else PlayFileSound(m_SndUtmXDown1);
				break;
			case 1:
				if (iDeltaX >= 0) PlayFileSound(m_SndUtmXUp2);
				else PlayFileSound(m_SndUtmXDown2);
				break;
			case 2:
				if (iDeltaX >= 0) PlayFileSound(m_SndUtmXUp3);
				else PlayFileSound(m_SndUtmXDown3);
				break;
			}
		}

		// Sound for the Y coordinate:
		if ((iDeltaY >= 500) || (iDeltaY <= -500))
		{
			m_iLastSoundUtmY = 500*int((iUtmY+250)/500);
			switch (m_iLastSoundUtmY % 10000)
			{
			case 0:
				PlayFileSound(m_SndUtmY0000);
				break;
			case 1000:
				PlayFileSound(m_SndUtmY1000);
				break;
			case 2000:
				PlayFileSound(m_SndUtmY2000);
				break;
			case 3000:
				PlayFileSound(m_SndUtmY3000);
				break;
			case 4000:
				PlayFileSound(m_SndUtmY4000);
				break;
			case 5000:
				PlayFileSound(m_SndUtmY5000);
				break;
			case 6000:
				PlayFileSound(m_SndUtmY6000);
				break;
			case 7000:
				PlayFileSound(m_SndUtmY7000);
				break;
			case 8000:
				PlayFileSound(m_SndUtmY8000);
				break;
			case 9000:
				PlayFileSound(m_SndUtmY9000);
				break;
			}

			// Sound for the exit point:
			int iThirdForX = int((iUtmX%1000)/334);
			switch (iThirdForX)
			{
			case 0:
				if (iDeltaY >= 0) PlayFileSound(m_SndUtmYUp1);
				else PlayFileSound(m_SndUtmYDown1);
				break;
			case 1:
				if (iDeltaY >= 0) PlayFileSound(m_SndUtmYUp2);
				else PlayFileSound(m_SndUtmYDown2);
				break;
			case 2:
				if (iDeltaY >= 0) PlayFileSound(m_SndUtmYUp3);
				else PlayFileSound(m_SndUtmYDown3);
				break;
			}
		}
	}
}

// ---------------------------------------------------------------

