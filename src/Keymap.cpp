#include "Keymap.h"
#include "Commands.h"

#include <vector>

extern wstring FormatKey(int nScancode);

CKeymap::CAction::CAction(WORD wKey, WORD wCommand, const wchar_t * wcsDescription)
{
	static int iNextId = 0;
	m_iId = iNextId++;
	m_wKey = wKey;
	m_wCommand = wCommand;
	m_wstrDescription = wcsDescription;
}

void CKeymap::CAction::AddToList(IListAcceptor2 * pAcceptor)
{
	int i = pAcceptor->AddItem(FormatKey(m_wKey).c_str(), m_iId, 0, m_wCommand);
	pAcceptor->AddItem(m_wstrDescription.c_str(), i, 1, m_wCommand);
}

void CKeymap::CAction::UpdateCurrent(IListAcceptor2 * pAcceptor)
{
	pAcceptor->UpdateCurrent(FormatKey(m_wKey).c_str(), 0);
	pAcceptor->UpdateCurrent(m_wstrDescription.c_str(), 1);
}

void CKeymap::Load()
{
	vector<Byte> data;
	bool fSuccess = false;
	unsigned long ulTotalLen = 0;
	DWORD dwType = REG_BINARY;
	wstring wstrKey = L"Keymap";
	RegQueryValueEx(m_hRegKey, wstrKey.c_str(), 0, &dwType, 0, &ulTotalLen);
	if (!ulTotalLen)
	{
		wstrKey = L"KeymapDef";
		RegQueryValueEx(m_hRegKey, wstrKey.c_str(), 0, &dwType, 0, &ulTotalLen);
	}
	if (ulTotalLen > 0)
	{
		if (dwType != REG_BINARY)
			return;
		data.resize(ulTotalLen);
		if (RegQueryValueEx(m_hRegKey, wstrKey.c_str(), 0, &dwType, &data[0], &ulTotalLen) != ERROR_SUCCESS)
			return;
		unsigned int iPos = 0;
		while (iPos < ulTotalLen)
		{
			WORD wCommand;
			int iScancode;
			memcpy(&wCommand, &data[iPos], sizeof(wCommand));
			iPos += sizeof(wCommand);
			memcpy(&iScancode, &data[iPos], sizeof(iScancode));
			iPos += sizeof(iScancode);

			fSuccess |= SetCommandKey(wCommand, iScancode);
		}
	}
	if (!fSuccess)
	{
		SetCommandKey(mcLeft, VK_LEFT);
		SetCommandKey(mcRight, VK_RIGHT);
		SetCommandKey(mcUp, VK_UP);
		SetCommandKey(mcDown, VK_DOWN);
		SetCommandKey(mcContextMenu, VK_RETURN);
	}
}

void CKeymap::Save()
{
	vector<Byte> data;
	for (list<CAction>::iterator it = m_Actions.begin(); it != m_Actions.end(); ++it)
	{
		WORD wCommand = it->m_wCommand;
		int iScancode = it->m_wKey;
		data.insert(data.end(), (const Byte*)&wCommand, (const Byte*)&wCommand + sizeof(wCommand));
		data.insert(data.end(), (const Byte*)&iScancode, (const Byte*)&iScancode + sizeof(iScancode));
	}
	wchar_t buf[1000];
	wsprintf(buf, L"%d", data.size());
	RegSetValueEx(m_hRegKey, L"Keymap", 0, REG_BINARY, &data[0], data.size());
}

void CKeymap::Init(HKEY hRegKey)
{
	m_hRegKey = hRegKey;
}
void CKeymap::GetList(IListAcceptor2 * pAcceptor)
{
	for (list<CAction>::iterator it = m_Actions.begin(); it != m_Actions.end(); ++it)
		it->AddToList(pAcceptor);
}
CKeymap::CKeymap()
{
}
void CKeymap::SetActionKey(int iAction, int nScancode)
{
	for (list<CAction>::iterator it = m_Actions.begin(); it != m_Actions.end(); ++it)
	{
		if (it->m_wKey == nScancode)
			it->m_wKey = 0;
		if (it->m_wCommand == iAction)
			it->m_wKey = nScancode;
	}
	Save();
	return;
}
bool CKeymap::SetCommandKey(WORD wCommand, int nScancode)
{
	if (nScancode <= 0)
		return false;
	for (list<CAction>::iterator it = m_Actions.begin(); it != m_Actions.end(); ++it)
	{
		if (it->m_wCommand == wCommand)
		{
			it->m_wKey = nScancode;
			return true;
		}
	}
	return false;
}
UINT CKeymap::Translate(int nScancode)
{
	for (list<CAction>::iterator it = m_Actions.begin(); it != m_Actions.end(); ++it)
	{
		if (it->m_wKey == nScancode)
			return it->m_wCommand;
	}
	if (nScancode == 'D')
		return mcoDebugMode;
	return -1;
}
void CKeymap::AddAction(int iCommand, const wchar_t * wcCommand)
{
	m_Actions.push_back(CAction(0, iCommand, wcCommand));
}

void CKeymap::UpdateCurrent(int iId, IListAcceptor2 * pAcceptor)
{
	for (list<CAction>::iterator it = m_Actions.begin(); it != m_Actions.end(); ++it)
	{
		if (it->m_wCommand == iId)
			it->UpdateCurrent(pAcceptor);
	}
}