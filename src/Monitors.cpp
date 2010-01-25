/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


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
	m_wstrValue = m_fSet ? IntToText(m_iValue) + L"°" : L"-";
	pPainter->DrawTextMonitor(&m_cached, m_wstrLabel.c_str(), m_wstrValue.c_str(), m_wstrValueCached != m_wstrValue);
	m_wstrValueCached = m_wstrValue;
}

void CTimeMonitor::PrepareContextMenu(IListAcceptor * pMenu)
{
	if (m_fSet)
		pMenu->AddItem(L("Set time"), cnSetTimeCmd);
	pMenu->AddItem(L("Toggle show date"), cnShowDateCmd);
}

void COptionMonitor::Paint(IMonitorPainter * pPainter)
{
	m_wstrValue = m_Value() ? L("Yes") : L("No");
	pPainter->DrawTextMonitor(&m_cached, m_wstrLabel.c_str(), m_wstrValue.c_str(), m_wstrValueCached != m_wstrValue);
	m_wstrValueCached = m_wstrValue;
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
