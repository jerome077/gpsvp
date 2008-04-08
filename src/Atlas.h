#ifndef ATLAS_H
#define ATLAS_H

#include <windows.h>
#include "Header.h"

inline bool operator == (const pair<CIMGFile, bool> & p, int id)
{
	return p.first.GetID() == id;
}

struct IStatusPainter;

class CAtlas
{
private:
	typedef list<pair<CIMGFile, bool> > FileList;
	FileList m_imgFiles;
	HKEY m_hRegKey;
	unsigned int m_uiScale10;
	IPainter * m_pPainter;
	list<CIMGFile *> m_listToPaint;
	UInt m_uiBestBits;
public:
	void Init(HKEY hRegKey);
	void Add(const wchar_t * wcFilename, IPainter * pPainter);
	void BeginPaint(unsigned int uiScale10, IPainter * pPainter, IStatusPainter * pStatusPainter);
	void PaintMapPlaceholders(IPainter * pPainter);
	void Paint(UInt uiObjectTypes, bool fDirectPaint);
	void Trim(const GeoRect &rect);
	void GetList(IListAcceptor * pAcceptor);
	void GetListUpdateCurrent(int iSelected, IListAcceptor * pAcceptor);
	void CloseMapByID(int iMap);
	void ToggleActiveByID(int iMap);
	void Load();
	void Save();
	const CIMGFile & ById(int id);
	void CloseAll();
	bool IsEmpty();
};

#endif // ATLAS_H
