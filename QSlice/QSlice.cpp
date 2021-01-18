// QSlice.cpp : main source file for QSlice.exe
//

#include "pch.h"
#include "resource.h"
#include "MainDlg.h"

CAppModule _Module;

int Run(LPTSTR /*lpstrCmdLine*/ = nullptr, int nCmdShow = SW_SHOWDEFAULT) {
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMainDlg dlgMain;

	if (dlgMain.Create(nullptr) == nullptr) 	{
		ATLTRACE(_T("Main dialog creation failed!\n"));
		return 0;
	}

	dlgMain.ShowWindow(nCmdShow);

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow) {
	HRESULT hRes = ::CoInitialize(nullptr);
	ATLASSERT(SUCCEEDED(hRes));

	AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(lpstrCmdLine, nCmdShow);

	_Module.Term();
	::CoUninitialize();

	return nRet;
}
