#ifndef FONTCACHE_H_INCLUDED
#define FONTCACHE_H_INCLUDED

#include <windows.h>

class CFontCache
{
	class Impl;
	Impl * m_impl;
	CFontCache(const CFontCache &);
	void operator =(const CFontCache &);
public:
	CFontCache();
	~CFontCache();
	HFONT GetFont(int size, bool bold, int angle);
	HFONT GetFont(int iTemplate, int angle);
	void SetTemplate(int iTemplate, int iSize, bool fBold);
	void SetLargeFonts(bool fLarge);
};

#endif FONTCACHE_H_INCLUDED
