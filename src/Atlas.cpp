#include "Atlas.h"
#include "ipainter.h"
#include "MapApp.h"
#include <algorithm>

void CAtlas::Init(HKEY hRegKey)
{
	m_hRegKey = hRegKey;
	Load();
}
void CAtlas::Add(const wchar_t * wcFilename, IPainter * pPainter)
{
	for (FileList::iterator it = m_imgFiles.begin(); it != m_imgFiles.end(); ++it)
	{
		if (it->first.GetFilename() == wcFilename)
			return;
	}
	m_imgFiles.push_back(make_pair(CIMGFile(), true));
	if (!m_imgFiles.back().first.Parse(wcFilename))
	{
		m_imgFiles.pop_back();
		return;
	}
	Save();
	if (!pPainter->WillPaint(m_imgFiles.back().first.GetRect()))
		pPainter->SetView(m_imgFiles.back().first.GetCenter(), true);
	app.CheckMenu();
}
void CAtlas::BeginPaint(unsigned int uiScale10, IPainter * pPainter, IStatusPainter * pStatusPainter)
{
	m_uiScale10 = uiScale10;
	m_pPainter = pPainter;

	m_uiBestBits = 100;
	FileList::iterator it;
	int iMapsToRead = 0;
	for (it = m_imgFiles.begin(); it != m_imgFiles.end(); ++it)
	{
		if (!it->second)
			continue;
		if (!pPainter->WillPaint(it->first.GetRect()))
			continue;
		if (it->first.WillRead())
			++iMapsToRead;
	}
	if (iMapsToRead > 0 && pStatusPainter)
	{
		wchar_t buff[1000];
		swprintf(buff, 1000, ((iMapsToRead == 1) ? L("Reading a map") : L("Reading %d maps")), iMapsToRead);
		pStatusPainter->PaintText(buff);
		pStatusPainter->SetProgressItems(0, iMapsToRead);
	}
	for (it = m_imgFiles.begin(); it != m_imgFiles.end(); ++it)
	{
		if (!it->second)
			continue;
		if (!pPainter->WillPaint(it->first.GetRect()))
			continue;
		if (it->first.WillRead() && pStatusPainter)
		{
			it->first.ParseSubdivisions(pStatusPainter, 1);
			pStatusPainter->Advance(0);
		}
		UInt bits = it->first.GetLevelByScale(uiScale10, pPainter);
		UInt uiBestScale = 10 << (24 - m_uiBestBits);
		UInt uiNewScale = 10 << (24 - bits);
		if (m_uiBestBits == 100)
			m_uiBestBits = bits;
		if (uiBestScale < uiScale10)
		{
			if (bits < m_uiBestBits)
				m_uiBestBits = bits;
		}
		else if (uiNewScale < uiScale10)
		{
		}
		else if (bits > m_uiBestBits)
		{
			m_uiBestBits = bits;
		}
	}
	m_listToPaint.clear();
	UInt uiBestScale = 1 << (24 - m_uiBestBits);
	if (uiBestScale < uiScale10 / 10  / 4)
	{
		m_uiBestBits = 0;
		return;
	}
	for (it = m_imgFiles.begin(); it != m_imgFiles.end(); ++it)
	{
		if (!it->second)
			continue;
		if (!pPainter->WillPaint(it->first.GetRect()))
			continue;
		if (it->first.GetLevelByScale(uiScale10, pPainter) != m_uiBestBits)
			continue;
		if (m_listToPaint.empty())
		{
			m_listToPaint.push_back(&(it->first));
			continue;
		}
		bool fAdd = true;
		for (list<CIMGFile*>::iterator it2 = m_listToPaint.begin(); it2 != m_listToPaint.end(); )
		{
			bool fMoved = false;
			while (it2 != m_listToPaint.end() && it->first.GetRect().Contain((*it2)->GetRect()))
			{
				if ((*it2)->GetRect().Contain(pPainter->GetRect()))
				{
					fAdd = false;
					break;
				}
				list<CIMGFile*>::iterator it3 = it2;
				++it2;
				fMoved = true;
				m_listToPaint.erase(it3);
			}
			if (!fMoved)
				++it2;
		}
		if (fAdd)
			m_listToPaint.push_back(&(it->first));
	}
}
void CAtlas::PaintMapPlaceholders(IPainter * pPainter)
{
	FileList::iterator it;
	for (it = m_imgFiles.begin(); it != m_imgFiles.end(); ++it)
	{
		if (!it->second)
			continue;
		if (!pPainter->WillPaint(it->first.GetRect()))
			continue;
		list<UInt> levels = it->first.GetLevels(pPainter);
		list<UInt>::iterator l = levels.begin();
		if (l == levels.end())
			continue;
		if (*l > m_uiBestBits)				
		{
			std::wstring wstrName = it->first.GetFilename();
			std::wstring::size_type slash = wstrName.find_last_of(L"\\/");
			if (slash != std::wstring::npos)
				wstrName = wstrName.substr(slash + 1);
			pPainter->StartPolygon(0xff, wstrName.c_str());
			GeoRect r = it->first.GetRect();
			pPainter->AddPoint(GeoPoint(r.minLon, r.minLat));
			pPainter->AddPoint(GeoPoint(r.maxLon, r.minLat));
			pPainter->AddPoint(GeoPoint(r.maxLon, r.maxLat));
			pPainter->AddPoint(GeoPoint(r.minLon, r.maxLat));
			pPainter->AddPoint(GeoPoint(r.minLon, r.minLat));
			pPainter->FinishObject();
		}
	}
}
void CAtlas::Paint(UInt uiObjectTypes, bool fDirectPaint)
{
	list<CIMGFile *>::iterator it;
	for (it = m_listToPaint.begin(); it != m_listToPaint.end(); ++it)
	{
		if (!m_pPainter->WillPaint((*it)->GetRect()))
			continue;
		(*it)->Paint(m_pPainter, m_uiBestBits, uiObjectTypes, fDirectPaint);
	}
}
void CAtlas::Trim(const GeoRect &rect)
{
	FileList::iterator it;
	for (it = m_imgFiles.begin(); it != m_imgFiles.end(); ++it)
		it->first.Trim(rect);
}
void CAtlas::GetList(IListAcceptor * pAcceptor)
{
	for (FileList::iterator it = m_imgFiles.begin(); it != m_imgFiles.end(); ++it)
	{
		pAcceptor->AddItem(((it->second ? L"+ " : L"- ") + it->first.GetFilename()).c_str(), it->first.GetID());
	}
}
void CAtlas::GetListUpdateCurrent(int iSelected, IListAcceptor * pAcceptor)
{
	for (FileList::iterator it = m_imgFiles.begin(); it != m_imgFiles.end(); ++it)
	{
		if (it->first.GetID() == iSelected)
			pAcceptor->UpdateCurrent(((it->second ? L"+ " : L"- ") + it->first.GetFilename()).c_str());
	}
}
void CAtlas::CloseMapByID(int iMap)
{
	for (FileList::iterator it = m_imgFiles.begin(); it != m_imgFiles.end(); ++it)
	{
		if (it->first.GetID() == iMap)
		{
			m_imgFiles.erase(it);
			Save();
			app.CheckMenu();
			return;
		}
	}
}
void CAtlas::ToggleActiveByID(int iMap)
{
	for (FileList::iterator it = m_imgFiles.begin(); it != m_imgFiles.end(); ++it)
	{
		if (it->first.GetID() == iMap)
		{
			it->second = !it->second;
			Save();
			return;
		}
	}
}
void CAtlas::Load()
{
	vector<Byte> data;
	unsigned long ulTotalLen = 0;
	DWORD dwType = REG_BINARY;
	wstring wstrKey = L"Atlas";
	RegQueryValueEx(m_hRegKey, wstrKey.c_str(), 0, &dwType, 0, &ulTotalLen);
	if (!ulTotalLen)
	{
		wstrKey = L"AtlasDef";
		RegQueryValueEx(m_hRegKey, wstrKey.c_str(), 0, &dwType, 0, &ulTotalLen);
	}
	if (ulTotalLen > 0)
	{
		if (dwType != REG_BINARY)
			return;
		data.resize(ulTotalLen);
		if (RegQueryValueEx(m_hRegKey, wstrKey.c_str(), 0, &dwType, &data[0], &ulTotalLen) != ERROR_SUCCESS)
			return;
		m_imgFiles.clear();
		unsigned int uiPos = 0;
		while (uiPos < ulTotalLen)
		{
			int iLen, iSize;
			wstring wstr;
			bool fFlag = true;
			memcpy(&iSize, &data[uiPos], sizeof(iLen));
			iLen = iSize;
			uiPos += sizeof(iLen);
			if (((wchar_t *)&data[uiPos])[iLen - 1] == 0)
			{
				iLen -= 2;
				fFlag = ((wchar_t *)&data[uiPos])[iLen + 1] != 0;
			}
			wstr.assign((wchar_t *)&data[uiPos], iLen);
			uiPos += sizeof(wchar_t) * iSize;
			m_imgFiles.push_back(make_pair(CIMGFile(), fFlag));
			if (!m_imgFiles.back().first.Parse(wstr.c_str()))
				m_imgFiles.pop_back();
		}
	}
}
void CAtlas::Save()
{
	vector<Byte> data;
	for (FileList::iterator it = m_imgFiles.begin(); it != m_imgFiles.end(); ++it)
	{
		wstring wstrFilename = it->first.GetFilename();
		int len = wstrFilename.length();
		int len2 = len + 2;
		data.insert(data.end(), (const Byte*)&len2, (const Byte*)&len2 + sizeof(len2));
		data.insert(data.end(), (const Byte*)&wstrFilename.c_str()[0], (const Byte*)&wstrFilename.c_str()[0] + sizeof(wchar_t) * len); 
		wchar_t zero = 0;
		wchar_t flag = it->second;
		data.insert(data.end(), (const Byte*)&zero, (const Byte*)&zero + sizeof(zero));
		data.insert(data.end(), (const Byte*)&flag, (const Byte*)&flag + sizeof(flag));
	}
	if (data.size() > 0)
		RegSetValueEx(m_hRegKey, L"Atlas", 0, REG_BINARY, &data[0], data.size());
	else
		RegSetValueEx(m_hRegKey, L"Atlas", 0, REG_BINARY, 0, 0);
}
const CIMGFile & CAtlas::ById(int id)
{
	FileList::iterator it = std::find(m_imgFiles.begin(), m_imgFiles.end(), id);
	if (it == m_imgFiles.end())
	{
		static CIMGFile stub;
		return stub;
	}
	return it->first;
}

bool CAtlas::IsEmpty()
{
	return m_imgFiles.empty();
}

void CAtlas::CloseAll()
{
	m_imgFiles.clear();
	Save();
	app.CheckMenu();
}