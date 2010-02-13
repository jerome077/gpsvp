/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "PointSet.h"
#include "Traffic.h"
#include "GeoPoint.h"
#include "Lock.h"
#include "IPainter.h"
#include "MapApp.h"
#include <list>
#include <map>
#include <set>
#include <strstream>
#include <algorithm>

inline bool operator < (const GeoPoint & gp1, const GeoPoint & gp2)
{
	if (gp1.lat == gp2.lat)
		return gp1.lon < gp2.lon;
	return gp1.lat < gp2.lat;
}

static const double dTrafficRadius = 100;
static const int iTrafficRadius = int(dTrafficRadius);

struct TrafficPoint
{
	TrafficPoint(const GeoPoint & gp) : p(gp){};
	GeoPoint p;
	bool operator == (const TrafficPoint & tp) const {return tp.p == p;}
};

typedef std::list<TrafficPoint> TrafficPoints;

struct TrafficLeg
{
	TrafficLeg(const GeoPoint & from, const GeoPoint & to, double speed) :
		gpFrom(from), gpTo(to), dSpeed(speed)
	{
	}
	GeoPoint gpFrom;
	GeoPoint gpTo;
	double dSpeed;
};

struct ILegSender
{
	virtual void SendLeg(const TrafficLeg & leg) = 0;
};

struct Splitter
{
	Splitter(ILegSender * legSender) : _legSender(legSender), _turnCount(0), _currentTurn(_turns.FindEnd()), _ulPrevTime(0) { Reset(); }
	Splitter() : _legSender(NULL), _turnCount(0), _currentTurn(_turns.FindEnd()), _ulPrevTime(0) { Reset(); }
	void Init(ILegSender * legSender) { _legSender = legSender; }
	ILegSender * _legSender;
	bool _started;
	double _currentLength;
	GeoPoint _lastPoint;
	struct Turn
	{
		GeoPoint _p;
		mutable int _n;
		Turn(const GeoPoint & p) : _p(p), _n(1) {}
		operator const GeoPoint & () const {return _p;}
	};
	typedef PointSet<Turn> Turns;
	Turns _turns;
	Turns::find_iterator _currentTurn;
	GeoPoint _currentTurnNearest;
	unsigned long _currentTurnNearestTime;
	int _currentTurnDistance;
	struct WayPoint
	{
		GeoPoint _gp;
		unsigned long _ulTime;
		WayPoint(const GeoPoint & gp, unsigned long time) : _gp(gp), _ulTime(time) {}
	};
	typedef std::list<WayPoint> Leg;
	Leg _currentLeg;
	int _turnCount;
	void Reset()
	{
		_started = false;
		_currentLength = 0;
		_currentLeg.clear();
	}
	void AddExtTurn(const GeoPoint & gp)
	{
		Turns::find_iterator it = _turns.FindNearest(gp, 1);
		if (it != _turns.FindEnd())
			return;
		_turns.Add(gp);
	}
	unsigned long _ulPrevTime;
	GeoPoint _prevGp;
	void Send(const GeoPoint & gp, unsigned long ulTime)
	{
		if (_ulPrevTime != 0)
		{
			double distance = DoubleDistance(_prevGp, gp);
			if (distance < dTrafficRadius)
				return;
			if (distance < 2000 + dTrafficRadius * 10)
			{
				const TrafficLeg & tl = TrafficLeg(_prevGp, gp, distance * 3600 /* * 1000 / 1000 */ / (ulTime - _ulPrevTime));
				_legSender->SendLeg(tl);
			}
			else
			{
				const TrafficLeg & tl = TrafficLeg(_prevGp, gp, distance * 3600 /* * 1000 / 1000 */ / (ulTime - _ulPrevTime));
			}
		}
		_ulPrevTime = ulTime;
		_prevGp = gp;
	}
	void SendReset()
	{
		_ulPrevTime = 0;
	}
	void Fix(const GeoPoint & gp, unsigned long ulTime)
	{
		if (_currentTurn != _turns.FindEnd())
		{
			int distance = IntDistance(gp, *_currentTurn);
			if (distance < _currentTurnDistance)
			{
				_currentTurnDistance = distance;
				_currentTurnNearest = gp;
				_currentTurnNearestTime = ulTime;
			}
			if (distance < dTrafficRadius)
				return;
			Send(_currentTurnNearest, _currentTurnNearestTime);
			++ (*_currentTurn)._n;
			_currentLeg.clear();
			_currentLeg.push_back(WayPoint(gp, ulTime));
			_lastPoint = gp;
			_currentLength = 0;
			_currentTurn = _turns.FindEnd();
			return;
		}
		_currentTurn = _turns.FindNearest(gp, iTrafficRadius);
		if (_currentTurn != _turns.FindEnd())
		{
			_currentTurnNearest = gp;
			_currentTurnDistance = _currentTurn._distance;
			_currentTurnNearestTime = ulTime;
			return;
		}
		if (!_started)
		{
			_currentLeg.push_back(WayPoint(gp, ulTime));
			_lastPoint = gp;
			_started = true;
			return;
		}
		double dist = DoubleDistance(_lastPoint, gp);
		if (dist < 25)
			return;
		if (dist > 2000)
		{
			Reset();
			SendReset();
			Fix(gp, ulTime);
			return;
		}
		_currentLeg.push_back(WayPoint(gp, ulTime));
		_currentLength += DoubleDistance(_lastPoint, gp);
		double length = DoubleDistance(_currentLeg.front()._gp, gp);
		if (_currentLength > 2000)
		{
			Send(gp, ulTime);
			_currentLeg.clear();
			_currentLeg.push_back(WayPoint(gp, ulTime));
			_currentLength = 0;
		}
		if (_currentLength > 10 + 1.02 * length)
		{
			Leg::iterator bestPoint = _currentLeg.begin();
			int bestDistance = IntDistance(_currentLeg.front()._gp, bestPoint->_gp) + IntDistance(gp, bestPoint->_gp);
			for (Leg::iterator it = _currentLeg.begin(); it != _currentLeg.end(); ++it)
			{
				int distance = IntDistance(_currentLeg.front()._gp, it->_gp) + IntDistance(gp, it->_gp);
				if (distance > bestDistance)
				{
					bestPoint = it;
					bestDistance = distance;
				}
			}
			//if (_turnCount < 100)
			//	++_turnCount;
			//else
			//	_turns.pop_front();
			Send(bestPoint->_gp, bestPoint->_ulTime);
			_currentLeg.clear();
			_currentLeg.push_back(WayPoint(gp, ulTime));
			_currentLength = 0;
		}
		_lastPoint = gp;
	}
	void Paint(IPainter * p) const
	{
		//for (Turns::const_iterator it = _turns.begin(); it != _turns.end(); ++it)
		//	p->PaintPoint(0xffff, *it, IntToText((*it)._n).c_str());
	}
};

struct TrafficNodes::Data : public ILegSender
{
	Data() : m_ulLastTime(0), m_fRefresh(false), m_fTrafficLoaded(false) {
		m_Splitter.Init(this);
	}
	void Fix(const GeoPoint & gp, unsigned long ulTime);
	void Paint(IPainter * p, bool fTrafficFlags) const;
	void AddNode(const GeoPoint & gp);
	bool PaintFastestWay(const GeoPoint & gpFrom, const GeoPoint & gpTo, IPainter * p) const;
	void AddTraffic(const GeoPoint & gp1, const GeoPoint & gp2, int speed)
	{
		if (speed < 0)
			return;
		m_Traffic.push_back(std::make_pair(std::make_pair(gp1, gp2), speed));
		double time = DoubleDistance(gp1, gp2) / speed;
		m_RoutingGraph[gp1].push_back(Destination(gp2, time));
	}
	void ClearTraffic()
	{
		m_Traffic.clear();
		m_RoutingGraph.clear();
	}
	void SendLeg(const TrafficLeg & leg);

	unsigned long m_ulLastTime;
	GeoPoint m_gpLastNode;
	GeoPoint m_gpLastFix;
	std::list<TrafficLeg> m_TrafficLegs;
	GeoPoint m_gpPrevious;
	double m_dLength;
	TrafficPoints m_Points;
	std::wstring m_wstrFilename;
	bool m_fRefresh;
	bool m_fTrafficLoaded;
	typedef std::list<std::pair<std::pair<GeoPoint, GeoPoint>, int > > Traffic;
	Traffic m_Traffic;
	typedef std::pair<int, int> NodeRegion;
	typedef std::set<NodeRegion> NodeRegions;
	NodeRegions m_RegionsToLoad;
	NodeRegions m_LoadedRegions;
	struct Destination
	{
		Destination(const GeoPoint & to, double time) : _to(to), _time(time){}
		double _time;
		GeoPoint _to;
	};
	typedef std::list<Destination> Destinations;
	typedef std::map<GeoPoint, Destinations> RoutingGraph;
	RoutingGraph m_RoutingGraph;
	Splitter m_Splitter;
};

TrafficNodes::TrafficNodes() : m_pData(new Data)
{	
}

TrafficNodes::~TrafficNodes()
{	
	delete m_pData;
}

bool TrafficNodes::PaintFastestWay(const GeoPoint & gpFrom, const GeoPoint & gpTo, IPainter * p) const
{
	return m_pData->PaintFastestWay(gpFrom, gpTo, p);
}

struct QueueNode
{
	double _time;
	GeoPoint _gp;
	bool operator < (const QueueNode & than) const
	{
		if (_time != than._time)
			return _time < than._time;
		return _gp < than._gp;
	}
	QueueNode(double time, const GeoPoint & gp) : _time(time), _gp(gp) {}
};
typedef std::set<QueueNode> Queue;
struct WayNode
{
	WayNode(const GeoPoint & from, double time) : _from(from), _time(time) {}
	WayNode() : _time(0) {}
	GeoPoint _from;
	double _time;
};
typedef std::map<GeoPoint, WayNode> Way;

bool TrafficNodes::Data::PaintFastestWay(const GeoPoint & gpFrom, const GeoPoint & gpTo, IPainter * p) const
{
	if (m_RoutingGraph.empty())
		return false;

	Queue queue;
	Way way;

	for (RoutingGraph::const_iterator it = m_RoutingGraph.begin(); it != m_RoutingGraph.end(); ++it)
	{
		const GeoPoint & gpCurrentFrom = it->first;
		GeoRect r;
		r.Init(gpCurrentFrom);
		// assert(GPWIDTH<31);
		r.Expand((1 << GPWIDTH) / (40000000 / 2500));
		if (!r.Contain(gpFrom))
			continue;
		int iFrom = IntDistance(gpFrom, gpCurrentFrom);
		const Destinations & d = it->second;
		for (Destinations::const_iterator dit = d.begin(); dit != d.end(); ++dit)
		{
			const GeoPoint & gpCurrentTo = dit->_to;
			r.Init(gpCurrentFrom);
			r.Append(gpCurrentTo);
			r.Expand((1 << GPWIDTH) / (40000000 / 100));
			if (!r.Contain(gpFrom))
				continue;
			int iTo = IntDistance(gpFrom, gpCurrentTo);
			int iTotal = IntDistance(gpCurrentFrom, gpCurrentTo);
			if (iTotal + iTrafficRadius > iFrom + iTo)
			{
				queue.insert(QueueNode(iTo / dit->_time, gpCurrentTo));
				way[gpCurrentTo] = WayNode();
			}
		}
	}

	while (!queue.empty())
	{
		QueueNode qn = *queue.begin();
		queue.erase(queue.begin());
		const GeoPoint & gpCur = qn._gp;
		RoutingGraph::const_iterator rgit = m_RoutingGraph.find(gpCur);
		if (rgit == m_RoutingGraph.end())
			continue;
		const Destinations & dst = rgit->second;
		for (Destinations::const_iterator it = dst.begin(); it != dst.end(); ++it)
		{
			double dtime = qn._time + it->_time;
			Way::iterator wit = way.find(it->_to);
			if (wit != way.end() && (wit->second._time <= dtime))
				continue;
			way[it->_to] = WayNode(gpCur, dtime);
			queue.insert(QueueNode(dtime, it->_to));
		}
	}

	if (way.empty())
		return false;
	GeoPoint gpBest = way.begin()->first;
	double dBestDist = DoubleDistance(gpBest, gpTo);
	double dBestTime = way.begin()->second._time;
	for (Way::iterator it = way.begin(); it != way.end(); ++it)
	{
		const GeoPoint & gpCurrent = it->first;
		double dDist = DoubleDistance(gpCurrent, gpTo);
		const double & dTime = it->second._time;
		if (dTime + dDist < dBestTime + dBestDist)
		{
			dBestDist = dDist;
			dBestTime = dTime;
			gpBest = gpCurrent;
		}
	}

	if (dBestDist * 2 > DoubleDistance(gpFrom, gpTo))
		return false;

	int max = 1000;
	p->StartPolyline(0x201, 0);
	p->AddPoint(gpTo);
	while (true)
	{
		p->AddPoint(gpBest);
		double time = way[gpBest]._time;
		if (time == 0)
			break;
		gpBest = way[gpBest]._from;
		if (!--max)
			break;
	}
	p->AddPoint(gpFrom);
	p->FinishObject();
	return true;
}

void TrafficNodes::Data::Fix(const GeoPoint & gp, unsigned long ulTime)
{
	m_Splitter.Fix(gp, ulTime);

	//const NodeRegion & reg = std::make_pair(gp.lat24() / 10000 , gp.lon24() / 10000);
	//if (m_LoadedRegions.find(reg) == m_LoadedRegions.end())
	//	m_RegionsToLoad.insert(reg);
	//if (IntDistance(gp, m_gpLastNode) < dTrafficRadius)
	//{
	//	m_ulLastTime = ulTime;
	//	m_gpLastFix = gp;
	//	return;
	//}
	//for (TrafficPoints::iterator it = m_Points.begin(); it != m_Points.end(); ++it)
	//{
	//	const GeoPoint & node = it->p;
	//	if (IntDistance(gp, node) < dTrafficRadius)
	//	{
	//		if (m_ulLastTime != 0)
	//		{
	//			m_TrafficLegs.push_back(TrafficLeg(m_gpLastNode, node, 
	//				DoubleDistance(m_gpLastFix, gp) * 3600 /* * 1000 / 1000 */ / (ulTime - m_ulLastTime),
	//				m_dLength + DoubleDistance(m_gpLastFix, m_gpLastNode) + DoubleDistance(gp, node)));
	//		}
	//		m_gpLastNode = node;
	//		m_gpLastFix = gp;
	//		m_ulLastTime = ulTime;
	//		m_dLength = 0; 
	//	}
	//	else
	//	{
	//		if (m_ulLastTime != 0)
	//			m_dLength += DoubleDistance(m_gpPrevious, gp);
	//	}
	//	m_gpPrevious = gp;
	//}
}

void TrafficNodes::Data::Paint(IPainter * p, bool fTrafficFlags) const
{
	if (app.m_Options[mcoDebugMode])
	{
		m_Splitter.Paint(p);
	}
	//if (app.m_Options[mcoShowTrafficNodes])
	//{
	//	for (TrafficPoints::const_iterator it = m_Points.begin(); it != m_Points.end(); ++it)
	//		p->PaintPoint(0xfffd, it->p, 0);
	//}
	if (app.m_Options[mcoShowTrafficInformation])
	{
		for (Traffic::const_iterator it = m_Traffic.begin(); it != m_Traffic.end(); ++it)
		{
			GeoRect r;
			r.Init(it->first.first);
			r.Append(it->first.second);
			if (!p->WillPaint(r))
				continue;
			unsigned int type = 0x100;
			if (it->second > 40)
				type = 0x102;
			else if (it->second > 20)
				type = 0x101;
			p->StartPolyline(type, 0);
			p->AddPoint(it->first.first);
			p->AddPoint(it->first.second);
			p->FinishObject();
			if (fTrafficFlags)
				p->PaintPoint(0xffff, it->first.first, 0);
		}
	}
}

void TrafficNodes::Data::AddNode(const GeoPoint & gp)
{
	m_Points.push_back(gp);
}

void TrafficNodes::Fix(const GeoPoint & gp, unsigned long ulTime)
{
	AutoLock l;
	m_pData->Fix(gp, ulTime);
}

void TrafficNodes::Paint(IPainter * p, bool fTrafficFlags) const
{
	AutoLock l;
	return m_pData->Paint(p, fTrafficFlags);
}

void TrafficNodes::AddNode(const GeoPoint & gp)
{
	AutoLock l;
	m_pData->AddNode(gp);
}

bool TrafficNodes::IsQueueEmpty()
{
	AutoLock l;
	return !m_pData->m_fRefresh && m_pData->m_TrafficLegs.empty();
}

std::string TrafficNodes::GetRequest(const GeoPoint & gp)
{
	AutoLock l;
	char request[1000];
	if (m_pData->m_fRefresh)
	{
		sprintf(request, "http://%s/GetTraffic2.php?lat=%d&lng=%d", app.GetServerName(), gp.lat24(), gp.lon24());
		return request;
	}
	if (m_pData->m_fTrafficLoaded && !m_pData->m_TrafficLegs.empty())
	{
		const TrafficLeg & leg = m_pData->m_TrafficLegs.front();
		sprintf(request, "http://%s/TrafficLeg3.php?flat=%d&flng=%d&tlat=%d&tlng=%d&speed=%d", app.GetServerName(),
			leg.gpFrom.lat24(), leg.gpFrom.lon24(), leg.gpTo.lat24(), leg.gpTo.lon24(), int(leg.dSpeed));
		return request;
	}
	return "";
}

void TrafficNodes::RefreshTrafficData()
{
	AutoLock l;
	m_pData->m_fRefresh = true;
}

void TrafficNodes::PopRequest(const std::string & request)
{
	AutoLock l;
	if (strstr(request.c_str(), "/GetTraffic2.php?"))
		m_pData->m_fRefresh = false;
	if (!m_pData->m_TrafficLegs.empty())
		m_pData->m_TrafficLegs.pop_front();
}

void TrafficNodes::TrafficData(const std::string & request, const char * data, int size)
{
	AutoLock l;
	if (strstr(request.c_str(), "/TrafficLeg3.php?"))
	{
		if (size != 2)
		{}
	}
	if (strstr(request.c_str(), "/GetTrafficNodes2.php?") && size)
	{
		if (!size)
			return;
		std::istrstream lines(data, size);
		std::string line;
		std::string region, lat, lng;
		{
			if (!std::getline(lines, line))
				return;
			std::istrstream fields(line.c_str(), line.length());
			if ( !(
				std::getline(fields, region, ',') 
				&& std::getline(fields, lat, ',') 
				&& std::getline(fields, lng, ',')
				) )
				return;
		}
		const Data::NodeRegion & reg = std::make_pair(atoi(lat.c_str()), atoi(lng.c_str()));
		m_pData->m_RegionsToLoad.erase(reg);
		m_pData->m_LoadedRegions.insert(reg);
		while(std::getline(lines, line))
		{
			std::istrstream fields(line.c_str(), line.length());
			if (
				std::getline(fields, lat, ',') 
				&& std::getline(fields, lng, ',')
				)
			{
				const GeoPoint & gp = GeoPoint24(atoi(lng.c_str()), atoi(lat.c_str()));
				AddNode(gp);
			}
		}
	}
	if (strstr(request.c_str(), "/GetTraffic2.php?"))
	{
		if (size)
		{
			m_pData->ClearTraffic();
			std::istrstream lines(data, size);
			std::string line;
			while(std::getline(lines, line))
			{
				std::istrstream fields(line.c_str(), line.length());
				std::string flat, flng, tlat, tlng, speed;
				if (
					std::getline(fields, flat, ',') 
					&& std::getline(fields, flng, ',') 
					&& std::getline(fields, tlat, ',') 
					&& std::getline(fields, tlng, ',') 
					&& std::getline(fields, speed, ',')
					)
				{
					const GeoPoint & gp1 = GeoPoint24(atoi(flng.c_str()), atoi(flat.c_str()));
					const GeoPoint & gp2 = GeoPoint24(atoi(tlng.c_str()), atoi(tlat.c_str()));
					m_pData->AddTraffic(gp1, gp2, atoi(speed.c_str()));
					m_pData->m_Splitter.AddExtTurn(gp1);
					m_pData->m_Splitter.AddExtTurn(gp2);
				}
			}
		}
		m_pData->m_fTrafficLoaded = true;
	}
}

void TrafficNodes::Data::SendLeg(const TrafficLeg & leg)
{
	m_TrafficLegs.push_back(leg);
	if (m_TrafficLegs.size() > 100)
		m_TrafficLegs.pop_front();
}

void TrafficNodes::Disable()
{
	AutoLock l;
	m_pData->m_fRefresh = false;
}
