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
		Action() : label(L""), command(0) {}
		Action(const wchar_t * label_, int command_) : label(label_), command(command_) {};
		bool operator == (int i) {return command == i;}
		const wchar_t * label;
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
		pPainter->AddButton(it->second.label, it->first, it->first == m_data->selected);	
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
		mmMenu.CreateItem(it->label, it->command);
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

void CScreenButtons::AddAction(int iCommand, const wchar_t * wcCommand)
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
	vector<Byte> data;
	bool fSuccess = false;
	unsigned long ulTotalLen = 0;
	DWORD dwType = REG_BINARY;
	RegQueryValueEx(m_data->key, L"ScreenButtons", 0, &dwType, 0, &ulTotalLen);
	if (ulTotalLen > 0)
	{
		if (dwType != REG_BINARY)
			return;
		data.resize(ulTotalLen);
		if (RegQueryValueEx(m_data->key, L"ScreenButtons", 0, &dwType, &data[0], &ulTotalLen) != ERROR_SUCCESS)
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
	vector<Byte> data;
	for (Data::Buttons::iterator it = m_data->buttons.begin(); it != m_data->buttons.end(); ++it)
	{
		WORD wCommand = it->second.command;
		data.insert(data.end(), (const Byte*)&wCommand, (const Byte*)&wCommand + sizeof(wCommand));
	}
	RegSetValueEx(m_data->key, L"ScreenButtons", 0, REG_BINARY, &data[0], data.size());
}