/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef TEAM_H_INCLUDED
#define TEAM_H_INCLUDED

#include "GeoPoint.h"
#include <string>

class CTeam
{
public:
	CTeam() : ulCurrent(0), ulSuccess(0), update(false) {}
	const std::wstring & GetChannel() {return channel;}
	const std::wstring & GetName() {return name;}
	void SetChannel(const wchar_t * value) {channel = value;}
	void SetName(const wchar_t * value) {name = value;}
	void Fix(const GeoPoint & gp, unsigned long ulTime)
	{
		ulCurrent = ulTime;
		gpCurrent = gp;
	}
	void Update() {update = true;}
	std::string GetRequest()
	{
		if (update) {
			char url[10000];
			// Maybe, change coordinate format to signed decimal floats? (Possible locale problems.)
			sprintf(url, "http://gpsvp.com/TeamGPS.php?name=%S&channel=%S&lat=%d&lng=%d", name.c_str(), channel.c_str(), gpCurrent.lat24(), gpCurrent.lon24());
			return url;
		}
		return "";
	}
	void Response(const char * data, int length)
	{
	}
private:
	std::wstring channel;
	std::wstring name;
	unsigned long ulCurrent;
	unsigned long ulRequest;
	unsigned long ulSuccess;
	GeoPoint gpCurrent;
	GeoPoint gpRequest;
	GeoPoint gpSuccess;
	bool update;
};

#endif // TEAM_H_INCLUDED
