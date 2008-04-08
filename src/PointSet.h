#ifndef POINTSET_H_INCLUDED
#define POINTSET_H_INCLUDED

#include <list>
#include "GeoPoint.h"

template <class Point>
class PointSet
{
private:
	typedef std::list<Point> List;
	List _list;
public:
	typedef typename List::const_iterator const_iterator;
	struct find_iterator
	{
		find_iterator(const_iterator found, int distance) : _found(found), _distance(distance) {}
		const_iterator _found;
		int _distance;
		const Point & operator *() const {return *_found;}
		bool operator != (const find_iterator & to) const
		{
			return _found != to._found;
		}
	};
	const_iterator begin() const {return _list.begin();}
	const_iterator end() const {return _list.end();}
	void Add(const Point & gp)
	{
		_list.push_back(gp);
	}
	find_iterator FindNearest(const Point & gp, int radius) const
	{
		find_iterator result(_list.end(), radius);
		for (List::const_iterator it = _list.begin(); it != _list.end(); ++it)
		{
			int distance = IntDistance(gp, *it);
			if (distance < result._distance)
				result = find_iterator(it, distance);
		}
		return result;
	}
	find_iterator FindEnd() const
	{
		return find_iterator(_list.end(), 0);
	}
};

#endif POINTSET_H_INCLUDED
