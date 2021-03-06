﻿/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef TRAFFIC_H
#define TRAFFIC_H

#include <string>

struct GeoPoint;
struct IPainter;

class TrafficNodes
{
public:
	TrafficNodes();
	~TrafficNodes();
	void Fix(const GeoPoint & gp, unsigned long ulTime);
	void Paint(IPainter * p, bool fTrafficFlags) const;
	bool PaintFastestWay(const GeoPoint & gpFrom, const GeoPoint & gpTo, IPainter * p) const;
	void AddNode(const GeoPoint & gp);
	bool IsQueueEmpty();
	std::string GetRequest(const GeoPoint & gp);
	void PopRequest(const std::string & request);
	void RefreshTrafficData();
	void Disable();
	void TrafficData(const std::string & request, const char * data, int size);
private:
	struct Data;
	Data * m_pData;
};

#endif // TRAFFIC_H
