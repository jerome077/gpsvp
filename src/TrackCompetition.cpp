/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "TrackCompetition.h"
#include "GeoPoint.h"
#include "IPainter.h"

struct TrackCompetition::Impl
{
	bool _started;
	bool _detecting;
	bool _detected;
	int _distance;
	int _bestDistance;
	int _bestTime;
	int _currentTime;
	GeoPoint _p;
	Impl() : _started(false) {}
	void Start(const GeoPoint & p) {_started = true; _detecting = false; _detected = false; _p = p;};
	bool IsStarted() {return _started;}
	bool IsDetected() {return _detected;}
	void Stop() {_started = false;}
	const GeoPoint & GetPoint() {return _p;}
	void Paint(IPainter * p)
	{
		if (_started) 
			p->PaintPoint(0x10002, _p, NULL);
	}
	void Fix(const GeoPoint & gp, unsigned long ulTime)
	{
		if (!_started)
			return;
		int distance = IntDistance(gp, _p);
		if (distance < 50)
		{
			if (_detecting)
			{
				if (_distance <= _bestDistance)
				{
					_bestDistance = distance;
					_bestTime = ulTime;
				}
			}
			else
			{
				_detecting = true;
				_detected = true;
				_bestDistance = distance;
				_bestTime = ulTime;
			}
		}
		else
		{
			if (_detecting)
			{
				_detecting = false;
			}
		}
		if (_detected)
			_currentTime = (ulTime - _bestTime + 500) / 1000;
	}
	unsigned long GetTime()
	{
		return _currentTime;
	}
};

TrackCompetition::TrackCompetition() : _impl(new Impl)
{
}

TrackCompetition::~TrackCompetition()
{
	delete _impl;
}

void TrackCompetition::Start(const GeoPoint & p) { return _impl->Start(p); };
bool TrackCompetition::IsStarted() { return _impl->IsStarted(); }
bool TrackCompetition::IsDetected() { return _impl->IsDetected(); }
void TrackCompetition::Stop() { _impl->Stop(); }
const GeoPoint & TrackCompetition::GetPoint() { return _impl->GetPoint(); }
void TrackCompetition::Paint(IPainter * p) { _impl->Paint(p); }
void TrackCompetition::Fix(const GeoPoint & gp, unsigned long ulTime) { _impl->Fix(gp, ulTime); }
unsigned long TrackCompetition::GetTime() { return _impl->GetTime(); }