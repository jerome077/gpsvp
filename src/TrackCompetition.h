#ifndef TRACKCOMPETITION_H
#define TRACKCOMPETITION_H

struct GeoPoint;
struct IPainter;

class TrackCompetition
{
	struct Impl;
	Impl * _impl;
public:
	TrackCompetition();
	~TrackCompetition();
	void Start(const GeoPoint & p);
	bool IsStarted();
	bool IsDetected();
	void Stop();
	const GeoPoint & GetPoint();
	void Paint(IPainter * p);
	void Fix(const GeoPoint & gp, unsigned long ulTime);
	unsigned long GetTime();
};

#endif // TRACKCOMPETITION_H
