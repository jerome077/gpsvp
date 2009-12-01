/*
Copyright (c) 2005-2008, Vsevolod E. Shorin
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of Vsevolod E. Shopin nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "FileDialogs.h"

#ifdef OWNFILEDIALOGS

#include "Dialogs.h"
#include "MapApp.h"
#include <resource.h>
#include <sstream>

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
	bool FileExist(const tchar_t * wcFilename)
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
		m_filename.SetText(m_wstrResult.c_str());
		m_list.Clear();
		{
			// Here we just check if the folder exists
			WIN32_FIND_DATA wwd;
			HANDLE h = FindFirstFile((app.m_rsCurrentFolder()).c_str(), &wwd);
			if (!h || h == INVALID_HANDLE_VALUE || 
				!(wwd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				app.m_rsCurrentFolder = T("");
			}
			FindClose(h);
		}
		int iItem = 0;
		{
			if (app.m_rsCurrentFolder() != T(""))
				m_list.AddItem(T("[..]"), iItem, 0, 1);
			++iItem;
		}
		{
			// Here we make the list of folders
			WIN32_FIND_DATA wwd;
			std::set<std::tstring> setDirectories;
			std::tstring wstrMask = app.m_rsCurrentFolder() + T("\\*.*");
			HANDLE h = FindFirstFile(wstrMask.c_str(), &wwd);
			if (h && h != INVALID_HANDLE_VALUE)
			{
				do
				{
					if ((wwd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) 
						&& !!wcscmp(wwd.cFileName, T(".")) && !!wcscmp(wwd.cFileName, T("..")))
					{
						std::tstring wstrItem = T("[");
						wstrItem += std::tstring(wwd.cFileName);
						wstrItem += T("]");
						setDirectories.insert(wstrItem);
					}
				} while (FindNextFile(h, &wwd));
				FindClose(h);
			}
			for (std::set<std::tstring>::iterator it = setDirectories.begin();
				it != setDirectories.end(); ++it)
			{
				m_list.AddItem(it->c_str(), iItem, 0, 1);
				++iItem;
			}
		}
		{
			// And here we make the list of files
			WIN32_FIND_DATA wwd;
			std::set<std::tstring> setFiles;
			// m_wstrMask could contain several extensions separated with ";"
			std::wstringstream wssMaskStream(m_wstrMask);
			std::tstring wstrMaskItem;
			while (std::getline(wssMaskStream, wstrMaskItem, L';'))
			{
				std::tstring wstrMask = app.m_rsCurrentFolder() + T("\\") + wstrMaskItem;
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
			}
			for (std::set<std::tstring>::iterator it = setFiles.begin();
				it != setFiles.end(); ++it)
			{
				m_list.AddItem(it->c_str(), iItem, 0, 2);
				++iItem;
			}
		}
		m_path.SetText((std::tstring(T("[")) + app.m_rsCurrentFolder() + T("\\]")).c_str());
		m_filename.SetFocus();
	}
	virtual void InitDialog(HWND hDlg)
	{
		SetWindowText(hDlg, L("Select file/folder"));
		m_filename.Create(hDlg);
		m_path.Create(hDlg, T(""));
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
		ReinitItemY();
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
						tchar_t buff[1000];
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
					tchar_t buff[1000];
					lv.iItem = iSelected;
					lv.pszText = buff;
					lv.cchTextMax = sizeof(buff);
					SendMessage(m_list.HWnd(), LVM_GETITEM, 0, LPARAM(&lv));
					if (buff[0] == L'[' && lv.lParam == 1)
					{
						tchar_t * ch = wcschr(buff + 1, ']');
						if (ch)
							*ch = 0;
						if (!wcscmp(buff + 1, T("..")))
						{
							std::tstring wstrCurrentFolder = app.m_rsCurrentFolder();
							wstrCurrentFolder.erase(wstrCurrentFolder.find_last_of(T("\\")));
							app.m_rsCurrentFolder = wstrCurrentFolder.c_str();
						}
						else
						{
							std::tstring wstrCurrentFolder = app.m_rsCurrentFolder();
							wstrCurrentFolder += T("\\");
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
					tchar_t buff[MAX_PATH + 1];
					m_filename.GetText(buff, MAX_PATH);
					if (buff[0])
					{
						std::tstring wstrResult = app.m_rsCurrentFolder() + T("\\") + buff;

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
			m_wstrResult = T("");
			EndDialog(hDlg, 0);
			return;
		}
	}
public:
	std::tstring m_wstrMask;
	std::tstring m_wstrResult;
	bool m_fFileMustExist;
	bool m_fProject;
	bool m_fSave;
};

std::tstring FileDialog(std::tstring wstrMask, MyOPENFILENAME * pof, bool fSave)
{
	static CFileDlg dlg;
	g_pNextDialog = &dlg;
	dlg.m_wstrMask = wstrMask;
	dlg.m_wstrResult = pof->lpstrFile; // Default value
	dlg.m_fFileMustExist = (pof->Flags & OFN_FILEMUSTEXIST) != 0;
	dlg.m_fProject = (pof->Flags & OFN_PROJECT) != 0;
	dlg.m_fSave = fSave;
	DialogBox(g_hInst, (LPCTSTR)IDD_TEMPLATE, pof->hwndOwner, (DLGPROC)MADlgProc);
	return dlg.m_wstrResult;
}

bool MyGetOpenFileName(MyOPENFILENAME * pof)
{
	tchar_t * wcFilter = pof->lpstrFilter + wcslen(pof->lpstrFilter) + 1;
	std::tstring wstrResult = FileDialog(wcFilter, pof, false);
	if (wstrResult == T(""))
		return false;
	wcsncpy(pof->lpstrFile, wstrResult.c_str(), pof->nMaxFile);
	return true;
}

bool MyGetSaveFileName(MyOPENFILENAME * pof)
{
	tchar_t * wcFilter = pof->lpstrFilter + wcslen(pof->lpstrFilter) + 1;
	std::tstring wstrResult = FileDialog(wcFilter, pof, true);
	if (wstrResult == T(""))
		return false;
	wcsncpy(pof->lpstrFile, wstrResult.c_str(), pof->nMaxFile);
	return true;
}

#endif
