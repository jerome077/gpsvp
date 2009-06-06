/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


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
		for (const_iterator it = _list.begin(); it != _list.end(); ++it)
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

#endif // POINTSET_H_INCLUDED
