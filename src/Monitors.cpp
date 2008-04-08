#include "Monitors.h"
#include "NMEAParser.h"
#include "MapApp.h"

void CSatellitesMonitor::Paint(IMonitorPainter * pPainter)
{
	pPainter->DrawMonitorLabel(m_wstrLabel.c_str());
	m_pNMEAParser->PaintSatellites(pPainter);
}

void CAzimuthMonitor::Paint(IMonitorPainter * pPainter)
{
	if (m_fSet)
	{
		std::wstring str = IntToText(m_iValue) + L"°";
		pPainter->DrawTextMonitor(m_wstrLabel.c_str(), str.c_str());
	}
	else
		pPainter->DrawTextMonitor(m_wstrLabel.c_str(), L"-");
}

void CTimeMonitor::PrepareContextMenu(IListAcceptor * pMenu)
{
	if (m_fSet)
		pMenu->AddItem(L("Set time"), cnSetTimeCmd);
	pMenu->AddItem(L("Toggle show date"), cnShowDateCmd);
}

void COptionMonitor::Paint(IMonitorPainter * pPainter)
{
	pPainter->DrawTextMonitor(m_wstrLabel.c_str(), m_Value() ? L("Yes") : L("No"));
}

void CTextMonitor::ProcessMenuCommand(int i)
{
	if (i == cnCopyText) {
		if (m_enTCSrc == TEXTCOPY_SRCURL) {
			//
			if (OpenClipboard(NULL)) {
				EmptyClipboard();
				HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, m_strURL.size() + 1); 
				void *pData = GlobalLock(h);
				memcpy(pData, m_strURL.c_str(), m_strURL.size() + 1);
				GlobalUnlock(h);
				SetClipboardData(CF_TEXT, h);
				CloseClipboard();
			}
		}
	}
}

void CTextMonitor::PrepareContextMenu(IListAcceptor * pMenu)
{
	if (m_enTCSrc == TEXTCOPY_SRCURL) {
		if ((m_strURL = app.m_request).empty()) {
		} else {
			pMenu->AddItem(L("Copy URL"), cnCopyText);
		}
	}
}
