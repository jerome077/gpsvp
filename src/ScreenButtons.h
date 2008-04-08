#ifndef SCREENBUTTONS_H
#define SCREENBUTTONS_H

#include <windows.h>
struct IButtonPainter;
struct ScreenPoint;

class CScreenButtons
{
private:
	struct Data;
	Data * m_data;
public:
	CScreenButtons();
	~CScreenButtons();
	void Paint(IButtonPainter * pPainter);
	int GetCommand(int iButton);
	void SelectButton(int iButton);
	void DeselectButton();
	bool ContextMenu(int iButton, const ScreenPoint & sp, HWND hwnd);
	void AddAction(int iCommand, const wchar_t * wcCommand);
	void AddButton(int iCommand);
	void Init(HKEY hKey);
	void Load();
	void Save();
};

#endif // SCREENBUTTONS_H