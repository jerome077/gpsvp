/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "ScreenButtons.h"
#include "IPainter.h"
#include "Commands.h"
#include "Menu.h"
#include "GDIPainter.h"

#include <map>
#include <list>
#include <algorithm>

struct CScreenButtons::Data
{
	struct Action
	{
		Action() : label(T("")), command(0) {}
		Action(const tchar_t * label_, int command_) : label(label_), command(command_) {};
		bool operator == (int i) {return command == i;}
		std::tstring label;
		int command;
	};
	int selected;
	typedef std::map<int, Action> Buttons;
	Buttons buttons;
	typedef std::list<Action> Actions;
	Actions actions;
	HKEY key;
};

CScreenButtons::CScreenButtons() : m_data(new Data)
{
	m_data->selected = -1;
}

CScreenButtons::~CScreenButtons()
{
	delete m_data;
}

void CScreenButtons::Paint(IButtonPainter * pPainter)
{
	for (Data::Buttons::iterator it = m_data->buttons.begin(); it != m_data->buttons.end(); ++it)
		pPainter->AddButton(it->second.label.c_str(), it->first, it->first == m_data->selected);	
}

void CScreenButtons::SelectButton(int i)
{
	m_data->selected = i;
}

void CScreenButtons::DeselectButton()
{
	m_data->selected = -1;
}

int CScreenButtons::GetCommand(int i)
{
	return m_data->buttons[i].command;
}

bool CScreenButtons::ContextMenu(int iButton, const ScreenPoint & sp, HWND hwnd)
{
	CMenu mmMenu;
	mmMenu.Init();
	Data::Actions::iterator it;
	Data::Actions::iterator begin = m_data->actions.begin();
	Data::Actions::iterator end = m_data->actions.end();
	for (it = begin; it != end; ++it)
		mmMenu.CreateItem(it->label.c_str(), it->command);
	DWORD dwRes = mmMenu.Popup(sp.x, sp.y, hwnd);
	it = std::find(begin, end, dwRes);
	if (it != end)
	{
		m_data->buttons[iButton] = *it;
		Save();
		return true;
	}
	return false;
}

void CScreenButtons::AddAction(int iCommand, const tchar_t * wcCommand)
{
	m_data->actions.push_back(Data::Action(wcCommand, iCommand));
}

void CScreenButtons::AddButton(int iCommand)
{
	Data::Actions::iterator it;
	Data::Actions::iterator begin = m_data->actions.begin();
	Data::Actions::iterator end = m_data->actions.end();
	it = std::find(begin, end, iCommand);
	if (it != end)
		m_data->buttons[m_data->buttons.size()] = *it;
}

void CScreenButtons::Init(HKEY hKey)
{
	m_data->key = hKey;
}

void CScreenButtons::Load()
{
	std::vector<Byte> data;
	bool fSuccess = false;
	DWORD ulTotalLen = 0;
	DWORD dwType = REG_BINARY;
	RegQueryValueEx(m_data->key, T("ScreenButtons"), 0, &dwType, 0, &ulTotalLen);
	if (ulTotalLen > 0)
	{
		if (dwType != REG_BINARY)
			return;
		data.resize(ulTotalLen);
		if (RegQueryValueEx(m_data->key, T("ScreenButtons"), 0, &dwType, &data[0], &ulTotalLen) != ERROR_SUCCESS)
			return;
		unsigned int iPos = 0;
		while (iPos < ulTotalLen)
		{
			WORD wCommand;
			memcpy(&wCommand, &data[iPos], sizeof(wCommand));
			iPos += sizeof(wCommand);

			AddButton(wCommand);
		}
	}
	if (m_data->buttons.size() < 1)
		AddButton(mcoFullScreen);
	if (m_data->buttons.size() < 2)
		AddButton(mcZoomOut);
	if (m_data->buttons.size() < 3)
		AddButton(mcZoomIn);
}

void CScreenButtons::Save()
{
	std::vector<Byte> data;
	for (Data::Buttons::iterator it = m_data->buttons.begin(); it != m_data->buttons.end(); ++it)
	{
		WORD wCommand = it->second.command;
		data.insert(data.end(), (const Byte*)&wCommand, (const Byte*)&wCommand + sizeof(wCommand));
	}
	RegSetValueEx(m_data->key, T("ScreenButtons"), 0, REG_BINARY, &data[0], data.size());
}
