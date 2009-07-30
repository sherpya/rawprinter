// RawPrinter.cpp : main source file for RawPrinter.exe
//

#include "stdafx.h"
#include "resource.h"
#include "Dialogs.h"

CAppModule _Module;

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMainDlg dlgMain;

	if(dlgMain.Create(NULL) == NULL)
	{
        DWORD err = GetLastError();
		ATLTRACE(_T("Main dialog creation failed!\n"));
		return 0;
	}

	dlgMain.ShowWindow(nCmdShow);

	int nRet = theLoop.Run();
	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{

#ifdef _UNICODE
    LPWSTR *_argv;
    int _argc;
    _argv = CommandLineToArgvW(GetCommandLineW(), &_argc);
#else
typedef struct {
  int newmode;
} _startupinfo;

typedef void (__cdecl *gma)(int *argc, char ***argv, char ***envp, int glob, _startupinfo *si);

    int _argc = 0;
    char **_argv = 0;
    char **dummy_environ;
    _startupinfo start_info = { 0 };
    gma getmainargs = (gma) GetProcAddress(LoadLibraryA("msvcrt.dll"), "__getmainargs");
    getmainargs(&_argc, &_argv, &dummy_environ, 0 /* noglob */, &start_info);
#endif

    if (_argc > 1)
        return CMainDlg::RawPrint(_argv[1]);

//	HRESULT hRes = ::CoInitialize(NULL);
// If you are running on NT 4.0 or higher you can use the following call instead to
// make the EXE free threaded. This means that calls come in on a random RPC thread.
    HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

    // Make richedit control work
	HMODULE hInstRich = ::LoadLibrary(CRichEditCtrl::GetLibraryName());
	ATLASSERT(hInstRich != NULL);

	AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();
	::CoUninitialize();

	return nRet;
}
