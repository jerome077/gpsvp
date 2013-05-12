/*	
Copyright (c) 2005-2011, Jerome077 and others
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SOUNDS_H
#define SOUNDS_H

#include <string>

// ---------------------------------------------------------------

// Play sounds on some events...
class CSoundPlayer
{
public:
	CSoundPlayer();
	void Init(const std::wstring& wstrWavBasePath);

	// Play the integrated default sound:
	void PlayDefaultSound();
	// Play the wav file or a default sound if the file is not found or nothing if the string is empty:
	void PlayFileSound(const std::wstring& wstrWavFile);

	// Sounds for some events:
	void PlaySoundGPS();
	void PlaySoundProximity();
	void PlaySoundAltitude(double dAltitude);
	void PlaySoundUTM(double dLon360, double dLat360, int iUtmZone);
protected:
	std::wstring m_wtsrWavBasePath;
	std::wstring m_SndGPS;
	std::wstring m_SndProximity;
	// altitude:
	bool m_WithSndAltitude;
	std::wstring m_SndAltitudeX000;
	std::wstring m_SndAltitudeX100;
	std::wstring m_SndAltitudeX200;
	std::wstring m_SndAltitudeX300;
	std::wstring m_SndAltitudeX400;
	std::wstring m_SndAltitudeX500;
	std::wstring m_SndAltitudeX600;
	std::wstring m_SndAltitudeX700;
	std::wstring m_SndAltitudeX800;
	std::wstring m_SndAltitudeX900;
	int m_iLastSoundAltude;
	// utm:
	bool m_WithSndUTM;
	int m_iLastSoundUtmX, m_iLastSoundUtmY;
	std::wstring m_SndUtmX0000, m_SndUtmX1000, m_SndUtmX2000, m_SndUtmX3000, m_SndUtmX4000;
	std::wstring m_SndUtmX5000, m_SndUtmX6000, m_SndUtmX7000, m_SndUtmX8000, m_SndUtmX9000;
	std::wstring m_SndUtmY0000, m_SndUtmY1000, m_SndUtmY2000, m_SndUtmY3000, m_SndUtmY4000;
	std::wstring m_SndUtmY5000, m_SndUtmY6000, m_SndUtmY7000, m_SndUtmY8000, m_SndUtmY9000;
	std::wstring m_SndUtmXUp1, m_SndUtmXUp2, m_SndUtmXUp3, m_SndUtmXDown1, m_SndUtmXDown2, m_SndUtmXDown3;
	std::wstring m_SndUtmYUp1, m_SndUtmYUp2, m_SndUtmYUp3, m_SndUtmYDown1, m_SndUtmYDown2, m_SndUtmYDown3;
};

// ---------------------------------------------------------------


#endif // SOUNDS_H

