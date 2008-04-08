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
