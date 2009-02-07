/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


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