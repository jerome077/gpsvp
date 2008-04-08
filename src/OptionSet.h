#ifndef OPTIONSET_H
#define OPTIONSET_H

#include <list>
#include <map>

class CMonitorSet;
class CMenuBar;

struct HKEY__;
typedef HKEY__ * HKEY;

class COptionSet
{
	struct Option;
	struct Data;
	Data * m_data;
public:
	COptionSet();
	~COptionSet();
	void Init(HKEY hKey, CMonitorSet * pMonitorSet);
	void AddOption(wchar_t * wcLabel, wchar_t * wcRegName, bool fDefault, int iCommand);
	void CheckMenu(CMenuBar & menuBar);
	bool ProcessCommand(int iCommand, CMenuBar & menuBar);
	bool operator [] (int iCommand);
};

#endif // OPTIONSET_H
