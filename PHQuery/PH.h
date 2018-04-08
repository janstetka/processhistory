#ifndef PH_H
#define PH_H

#include <windows.h>

class PH
{
	public:		

	HWND _hWnd;
	HWND _hWndStatusBar;
	HWND _hWndCombo2;
	HWND _hWndCombo3;
	HWND _hWndResults;
	HWND _hWndProgress;
} ;

bool FilterExec();
bool FilterUser();

#endif