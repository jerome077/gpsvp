/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


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
		std::wstring m_wstrDescription;
	};
	std::list<CAction> m_Actions;
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
