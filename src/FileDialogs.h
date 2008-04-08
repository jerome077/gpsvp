#ifndef FILEDIALOGS_H
#define FILEDIALOGS_H

#include <windows.h>
#include <Commdlg.h>

#if defined(OWNFILEDIALOGS)

struct MyOPENFILENAME
{
	HWND hwndOwner;
	long lStructSize;
	wchar_t * lpstrFilter;
	wchar_t * lpstrFile;
	int nMaxFile;
	HINSTANCE hInstance;
	DWORD Flags;
};

// enum {OFN_EXPLORER = 0x01, OFN_FILEMUSTEXIST = 0x02, OFN_PROJECT = 0x04};

bool MyGetSaveFileName(MyOPENFILENAME * pof);
bool MyGetOpenFileName(MyOPENFILENAME * pof);

#ifdef OPENFILENAME
#	undef OPENFILENAME
#endif
#ifdef GetSaveFileName
#	undef GetSaveFileName
#endif
#ifdef GetOpenFileName
#	undef GetOpenFileName
#endif
#define OPENFILENAME MyOPENFILENAME
#define GetSaveFileName MyGetSaveFileName
#define GetOpenFileName MyGetOpenFileName

#endif

#endif // FILEDIALOGS_H