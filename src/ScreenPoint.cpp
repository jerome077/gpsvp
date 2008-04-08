#include "ScreenPoint.h"
#include <list>

struct ScreenRectSet::Data
{
	std::list<ScreenRect> m_listRects;
};

ScreenRectSet::ScreenRectSet() : m_data(new Data)
{
}

ScreenRectSet::~ScreenRectSet()
{
	delete m_data;
}

bool ScreenRectSet::Fit(const ScreenRect & sr)
{
	for (std::list<ScreenRect>::iterator it = m_data->m_listRects.begin(); it != m_data->m_listRects.end(); ++it)
	{
		if (true == it->IntersectHard(sr))
			return false;
	}
	m_data->m_listRects.push_back(sr);
	return true;
}
void ScreenRectSet::Reset()
{
	m_data->m_listRects.clear();
}
