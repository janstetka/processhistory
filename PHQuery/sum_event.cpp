#include "screen.h"
#include "wtl\wtl.h"

extern PHDisplay phd;

void PHRB(int x, int y,HWND h) 
{ 
	POINT pt;
	pt.x=x;
	pt.y=y;
	phd.phs_hWnd=h;
	phd.FindOnScreen(x,y);
}

