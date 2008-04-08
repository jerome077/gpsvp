#include "FileDialogs.h"

#ifdef OWNFILEDIALOGS

#include "Dialogs.h"
#include "MapApp.h"
#include <resource.h>

class CFileDlg : public CMADialog
{
	enum
	{
		dmcOk = 1000,
		dmcCancel
	};
	CListView m_list;
	CEditText m_filename;
	CText m_path;
	bool FileExist(const wchar_t * wcFilename)
	{
		WIN32_FIND_DATA wwd;
		HANDLE h = FindFirstFile(wcFilename, &wwd);
		if (h && (h != INVALID_HANDLE_VALUE))
		{
			FindClose(h);
			if (!(wwd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				return true;
		}
		return false;
	}
	void FillList()
	{
		m_filename.SetText(L"");
		m_list.Clear();
		{
			// ����� �� ������ ��������� ������� ����������
			WIN32_FIND_DATA wwd;
			HANDLE h = FindFirstFile((app.m_rsCurrentFolder()).c_str(), &wwd);
			if (!h || h == INVALID_HANDLE_VALUE || 
				!(wwd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				app.m_rsCurrentFolder = L"";
			}
			FindClose(h);
		}
		int iItem = 0;
		{
			if (app.m_rsCurrentFolder() != L"")
				m_list.AddItem(L"[..]", iItem, 0, 1);
			++iItem;
		}
		{
			// ����� �� ��������� ������ ����������
			WIN32_FIND_DATA wwd;
			std::set<std::wstring> setDirectories;
			std::wstring wstrMask = app.m_rsCurrentFolder() + L"\\*.*";
			HANDLE h = FindFirstFile(wstrMask.c_str(), &wwd);
			if (h && h != INVALID_HANDLE_VALUE)
			{
				do
				{
					if ((wwd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
						&& !!wcscmp(wwd.cFileName, L".") && !!wcscmp(wwd.cFileName, L".."))
					{
						std::wstring wstrItem = L"[";
						wstrItem += std::wstring(wwd.cFileName);
						wstrItem += L"]";
						setDirectories.insert(wstrItem);
					}
				} while (FindNextFile(h, &wwd));
				FindClose(h);
			}
			for (std::set<std::wstring>::iterator it = setDirectories.begin();
				it != setDirectories.end(); ++it)
			{
				m_list.AddItem(it->c_str(), iItem, 0, 1);
				++iItem;
			}
		}
		{
			// � ����� - ������ ������
			WIN32_FIND_DATA wwd;
			std::set<std::wstring> setFiles;
			std::wstring wstrMask = app.m_rsCurrentFolder() + L"\\" + m_wstrMask;
			HANDLE h = FindFirstFile(wstrMask.c_str(), &wwd);
			if (h && h != INVALID_HANDLE_VALUE)
			{
				do
				{
					if (!(wwd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						setFiles.insert(wwd.cFileName);
					}
				} while (FindNextFile(h, &wwd));
				FindClose(h);
			}
			for (std::set<std::wstring>::iterator it = setFiles.begin();
				it != setFiles.end(); ++it)
			{
				m_list.AddItem(it->c_str(), iItem, 0, 2);
				++iItem;
			}
		}
		m_path.SetText((wstring(L"[") + app.m_rsCurrentFolder() + L"\\]").c_str());
	}
	virtual void InitDialog(HWND hDlg)
	{
		SetWindowText(hDlg, L("Select file/folder"));
		m_filename.Create(hDlg);
		m_path.Create(hDlg, L"");
		m_list.Create(hDlg, false);
		m_list.AddColumn(L("Name"), 300, 0);

		FillList();

		SetSoftkeybar(hDlg, IDR_TEMPLATE_MENUBAR_2);
		m_MenuBar.SetItemLabelAndCommand(IDC_LEFT, L("Open"), dmcOk);
		m_MenuBar.SetItemLabelAndCommand(IDC_RIGHT, L("Cancel"), dmcCancel);
		SetSubWindowProc(m_list.HWnd());
	}
	virtual void WindowPosChanged(HWND hDlg)
	{
		AddItem(hDlg, CText(hDlg, L("File name:")));
		AddItem(hDlg, m_filename);
		AddItem(hDlg, m_path);
		AddList(m_list.HWnd());
	}
	virtual void Notify(NMHDR * pMessage)
	{
		switch (pMessage->code)
		{
		case LVN_ITEMCHANGED:
			{
				NM_LISTVIEW * pLVMessage = (NM_LISTVIEW *)pMessage;
				if (pLVMessage->uNewState & LVIS_FOCUSED)
				{
					int iSelected = pLVMessage->iItem;
					if (iSelected >= 0)
					{
						LVITEM lv;
						ZeroMemory(&lv, sizeof(lv));
						lv.mask = LVIF_TEXT | LVIF_PARAM;
						wchar_t buff[1000];
						lv.iItem = iSelected;
						lv.pszText = buff;
						lv.cchTextMax = sizeof(buff);
						SendMessage(m_list.HWnd(), LVM_GETITEM, 0, LPARAM(&lv));
						if (lv.lParam == 2)
							m_filename.SetText(buff);
					}
				}
			}
			break;
		}
	}
	virtual void Command(HWND hDlg, int iCommand)
	{
		switch (iCommand)
		{
		case IDC_ACTION:
		case IDOK:
			// Process action on item
			{
				int iSelected = SendMessage(m_list.HWnd(), LVM_GETSELECTIONMARK, 0, 0);
				if (iSelected >= 0)
				{
					LVITEM lv;
					ZeroMemory(&lv, sizeof(lv));
					lv.mask = LVIF_TEXT | LVIF_PARAM;
					wchar_t buff[1000];
					lv.iItem = iSelected;
					lv.pszText = buff;
					lv.cchTextMax = sizeof(buff);
					SendMessage(m_list.HWnd(), LVM_GETITEM, 0, LPARAM(&lv));
					if (buff[0] == L'[' && lv.lParam == 1)
					{
						wchar_t * ch = wcschr(buff + 1, ']');
						if (ch)
							*ch = 0;
						if (!wcscmp(buff + 1, L".."))
						{
							wstring wstrCurrentFolder = app.m_rsCurrentFolder();
							wstrCurrentFolder.erase(wstrCurrentFolder.find_last_of(L"\\"));
							app.m_rsCurrentFolder = wstrCurrentFolder.c_str();
						}
						else
						{
							wstring wstrCurrentFolder = app.m_rsCurrentFolder();
							wstrCurrentFolder += L"\\";
							wstrCurrentFolder += buff + 1;
							app.m_rsCurrentFolder = wstrCurrentFolder.c_str();
						}
						FillList();
						return;
					}
				}
			}
			// No return by intention
		case dmcOk:
			{
				// Accept dialog for project
				if (m_fProject)
				{
					m_wstrResult = app.m_rsCurrentFolder();
					EndDialog(hDlg, 0);
					break;
				}

				// Accept filename typede in top dialog
				{
					wchar_t buff[MAX_PATH + 1];
					m_filename.GetText(buff, MAX_PATH);
					if (buff[0])
					{
						wstring wstrResult = app.m_rsCurrentFolder() + L"\\" + buff;

						if (m_fFileMustExist)
						{
							if (FileExist(wstrResult.c_str()))
							{
								m_wstrResult = wstrResult;
								EndDialog(hDlg, 0);
								break;
							}
						}
						else if (m_fSave && FileExist(wstrResult.c_str()))
						{
							if (IDYES == MessageBox(m_hDialog, L("File exists. Overwrite?"), 
								L("Warning"), MB_ICONQUESTION | MB_YESNO))
							{
								m_wstrResult = wstrResult;
								EndDialog(hDlg, 0);
								return;
							}
						}
						else
						{
							m_wstrResult = wstrResult;
							EndDialog(hDlg, 0);
							return;
						}
					}
				}
			}
			return;
		case dmcCancel:
			m_wstrResult = L"";
			EndDialog(hDlg, 0);
			return;
		}
	}
public:
	wstring m_wstrMask;
	wstring m_wstrResult;
	bool m_fFileMustExist;
	bool m_fProject;
	bool m_fSave;
};

wstring FileDialog(wstring wstrMask, MyOPENFILENAME * pof, bool fSave)
{
	static CFileDlg dlg;
	g_pNextDialog = &dlg;
	dlg.m_wstrMask = wstrMask;
	dlg.m_wstrResult = L"";
	dlg.m_fFileMustExist = (pof->Flags & OFN_FILEMUSTEXIST) != 0;
	dlg.m_fProject = (pof->Flags & OFN_PROJECT) != 0;
	dlg.m_fSave = fSave;
	DialogBox(g_hInst, (LPCTSTR)IDD_TEMPLATE, pof->hwndOwner, (DLGPROC)MADlgProc);
	return dlg.m_wstrResult;
}

bool MyGetOpenFileName(MyOPENFILENAME * pof)
{
	wchar_t * wcFilter = pof->lpstrFilter + wcslen(pof->lpstrFilter) + 1;
	wstring wstrResult = FileDialog(wcFilter, pof, false);
	if (wstrResult == L"")
		return false;
	wcsncpy(pof->lpstrFile, wstrResult.c_str(), pof->nMaxFile);
	return true;
}

bool MyGetSaveFileName(MyOPENFILENAME * pof)
{
	wchar_t * wcFilter = pof->lpstrFilter + wcslen(pof->lpstrFilter) + 1;
	wstring wstrResult = FileDialog(wcFilter, pof, true);
	if (wstrResult == L"")
		return false;
	wcsncpy(pof->lpstrFile, wstrResult.c_str(), pof->nMaxFile);
	return true;
}

#endif