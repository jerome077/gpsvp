#include "FontCache.h"
#include <map>

class CFontCache::Impl
{
public:
	struct Key
	{
		int size;
		bool bold;
		int angle;

		Key(int size_, bool bold_, int angle_):
			size(size_), bold(bold_), angle(angle_)
		{
		}

		bool operator < (const Key & k) const
		{
			if (size != k.size)
				return size < k.size;
			if (bold != k.bold)
				return bold < k.bold;
			return angle < k.angle;
		}
	};
	typedef std::map<Key, HFONT> Cache;
	Cache cache;
	std::map<int, int> size;
	std::map<int, bool> bold;
	LOGFONT lf;
	bool large;
};

CFontCache::CFontCache() : m_impl(new Impl)
{
	LOGFONT & lf = m_impl->lf;
	memset(&lf, 0, sizeof(lf));
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
#ifdef PROOF_QUALITY
	lf.lfQuality = ANTIALIASED_QUALITY | PROOF_QUALITY;
#else
	lf.lfQuality = ANTIALIASED_QUALITY | CLEARTYPE_COMPAT_QUALITY | CLEARTYPE_QUALITY;
#endif
	lf.lfPitchAndFamily = FF_ROMAN | VARIABLE_PITCH;
	wchar_t wcsName[] = L"Tahoma";
	memcpy(lf.lfFaceName, wcsName, sizeof(wcsName));
}

CFontCache::~CFontCache()
{
	delete m_impl;
}

HFONT CFontCache::GetFont(int iTemplate, int angle)
{
	int size = (m_impl->size.find(iTemplate) != m_impl->size.end()) ? m_impl->size[iTemplate] : 14;
	if (m_impl->large)
		size *= 2;
	bool bold = (m_impl->bold.find(iTemplate) != m_impl->bold.end()) ? m_impl->bold[iTemplate] : false;
	return GetFont(size, bold, angle);
}

HFONT CFontCache::GetFont(int size, bool bold, int angle)
{
	Impl::Key k (size, bold, angle);
	Impl::Cache::iterator it = m_impl->cache.find(k);
	if (it == m_impl->cache.end())
	{
		LOGFONT & lf = m_impl->lf;;
		lf.lfHeight = size;
		lf.lfWeight = (bold ? FW_BOLD : 0);
		lf.lfEscapement = angle;
		lf.lfOrientation = angle;

		HFONT h = CreateFontIndirect(&lf);
		m_impl->cache[k] = h;
		return h;
	}
	
	return it->second;
}

void CFontCache::SetTemplate(int iTemplate, int iSize, bool fBold)
{
	m_impl->size[iTemplate] = iSize;
	m_impl->bold[iTemplate] = fBold;
}

void CFontCache::SetLargeFonts(bool fLarge)
{
	m_impl->large = fLarge;
}