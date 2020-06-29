#include "resource.h"
#include "..\processhistory\mainfrm.h"

CAppModule _Module;

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMainFrame wndMain;

	if(wndMain.CreateEx() == NULL)
	{
		ATLTRACE(_T("Main window creation failed!\n"));
		return 0;
	}

	wndMain.ShowWindow(nCmdShow);

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

#include "..\PHLogger\PHLogger.h"

using namespace std;
extern CPHLogger logger;

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	
	HRESULT hRes = ::CoInitialize(NULL);
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	// this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
	::DefWindowProc(NULL, 0, 0, 0L);

	AtlInitCommonControls(ICC_BAR_CLASSES| ICC_COOL_CLASSES );	// add flags to support other controls
	
	//phd.hInst=hInstance;	
	//phd._WinLeft=second_clock::local_time();
	logger._Refresh = 1000;
	//if (lpstrCmdLine!=NULL) TODO 2020 do this via gui settings
	//	logger._Refresh = boost::lexical_cast<int>(string(lpstrCmdLine));
	//logger._Refresh = 500;

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	int nRet = Run(lpstrCmdLine, nCmdShow);
	logger._Refresh = -1;
	
	_Module.Term();
	::CoUninitialize();

	return nRet;
}
