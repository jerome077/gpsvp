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