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
			sprintf(url, "http://gpsvp.com/TeamGPS.php?name=%S&channel=%S", name.c_str(), channel.c_str());
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

#endif TEAM_H_INCLUDED
