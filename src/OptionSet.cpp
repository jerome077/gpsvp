/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "OptionSet.h"
#include "Monitors.h"
#include "MenuBar.h"
#include "Commands.h"

struct COptionSet::Option
{
	COptionMonitor value;
	int iCommand;
	void CheckMenuItem(CMenuBar & menubar)
	{
		menubar.GetMenu().CheckMenuItem(iCommand, value());
	}
};

struct COptionSet::Data
{
	typedef std::list<Option> OptionList;
	OptionList m_listOptions;
	typedef std::map<int, Option*> Commands;
	Commands m_Commands;
	HMENU m_hMenu;
	HKEY m_hRegKey;
	CMonitorSet * m_pMonitorSet;
};

COptionSet::COptionSet() : m_data(new Data) {};
COptionSet::~COptionSet() {delete m_data;}

void COptionSet::Init(HKEY hKey, CMonitorSet * pMonitorSet)
{
	AutoLock l;
	m_data->m_hRegKey = hKey;
	m_data->m_pMonitorSet = pMonitorSet;
}

void COptionSet::AddOption(wchar_t * wcLabel, wchar_t * wcRegName, bool fDefault, int iCommand)
{
	AutoLock l;
	m_data->m_listOptions.push_back(Option());
	Option & option = m_data->m_listOptions.back();
	option.iCommand = iCommand;
	option.value.SetIdL(wcLabel);
	if (wcRegName)
		option.value.SetRegistry(m_data->m_hRegKey, wcRegName, fDefault);
	else
		option.value = fDefault;
	// m_pMonitorSet->AddMonitor(&option.value);
	m_data->m_Commands[iCommand] = &option;
}

void COptionSet::CheckMenu(CMenuBar & menuBar)
{
	AutoLock l;
	for (Data::OptionList::iterator it = m_data->m_listOptions.begin(); it != m_data->m_listOptions.end(); ++it)
		it->CheckMenuItem(menuBar);
}

bool COptionSet::ProcessCommand(int iCommand, CMenuBar & menuBar)
{
	AutoLock l;
	Data::Commands::iterator it = m_data->m_Commands.find(iCommand);
	if (it == m_data->m_Commands.end())
		return false;
	it->second->value = !it->second->value();
	it->second->CheckMenuItem(menuBar);
	return true;
}

bool COptionSet::operator [] (int iCommand)
{
	AutoLock l;
	Data::Commands::iterator it = m_data->m_Commands.find(iCommand);
	if (it == m_data->m_Commands.end())
		return false;
	return it->second->value();
}
