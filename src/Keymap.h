#ifndef KEYMAP_H
#define KEYMAP_H

#include <string>
#include <list>
#include <windows.h>
#include "Common.h"

class CKeymap : public IListAcceptor2Acceptor
{
private:
	struct CAction
	{
		CAction(WORD wKey, WORD wCommand, const wchar_t * wcsDescription);
		void AddToList(IListAcceptor2 * pAcceptor);
		void UpdateCurrent(IListAcceptor2 * pAcceptor);
		Int m_iId;
		int m_wKey;
		WORD m_wCommand;
		wstring m_wstrDescription;
	};
	list<CAction> m_Actions;
	HKEY m_hRegKey;
public:
	void Init(HKEY hRegKey);
	void GetList(IListAcceptor2 * pAcceptor);
	CKeymap();
	void SetActionKey(int iAction, int nScancode);
	bool SetCommandKey(WORD wCommand, int nScancode);
	UINT Translate(int nScancode);
	void AddAction(int iCommand, const wchar_t * wcCommand);
	void Load();
	void Save();
	virtual void UpdateCurrent(int iId, IListAcceptor2 * pAcceptor);
};

#endif // KEYMAP_H