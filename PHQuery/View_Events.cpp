//#include "pathdlg.h"
#include "PH.h"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "screen.h"
#include "Query.h"
#include "..\phshared\phshared.h"
//#include "wtl\wtl.h"

using namespace boost;
using namespace boost::posix_time;
using namespace std;

extern PHQuery phq;
extern PH ph_instance;
extern PHDisplay phd;

/*For the query times 1 pixel = 1 second*/
ptime PHDisplay::GetEndTime(ptime Left)
{
	RECT r;
	//::GetWindowRect(ph_instance._hWnd,&r);
	GetClientRect(ph_instance._hWndResults, &r);
	_Width=r.right-r.left;
	_WinLeft=Left;
	float MinutesWide=(float)_Width/60;
	//float SecondsWide = (float)_Width / 3600; Pixel is scale milliseconds so width is scale * _Width
	//_WinRight=_WinLeft+time_duration(0,0,0,scale*Width);
	_WinRight=_WinLeft+time_duration(0,(int)MinutesWide,_Width-((int)MinutesWide*60));
	return _WinRight;
}

/*When the time or window size changes*/
void PHDisplay::UpdateWindow()//m_sBat)
{
	/*clear the old collections*/
	_ProcessAreas.clear();
	map<long,RECT>().swap(_ProcessAreas);
	_clipped_left.clear();
	set<long>().swap(_clipped_left);
	_clipped_right.clear();
	set<long>().swap(_clipped_right);
	_selected = 0;
		
	for(map<long,PHProcess>::iterator it =	phq._Processes.begin(); it!= phq._Processes.end(); it++)
	{
		PHProcess pclProcess=it->second;
		map<long,int>::iterator line_it=phq._Lines.find(it->first);
		if(line_it==phq._Lines.end())
			PHTrace("Could not find line for process",__LINE__,__FILE__);
		/*Don't draw beyond edge of window*/
		ptime start_p=pclProcess.start;
		ptime end_p=pclProcess.end;
		if(start_p<_WinLeft)
		{
			_clipped_left.insert(it->first);
			start_p=_WinLeft;
		}
		if(end_p>_WinRight)
		{
			_clipped_right.insert(it->first);
			end_p=_WinRight;
		}
		/*calculate the sizes to draw the processes*/
		RECT rProcess=CalculateRect(start_p,end_p,_WinLeft,line_it->second);
		_ProcessAreas.insert(pair<long,RECT>(it->first,rProcess));
	}
	ReadPaths();	
	
	ptime qrytime=second_clock::local_time();
	string qts="Queried at "+to_simple_string(qrytime/*rdmax*/);
	if(SetWindowText(ph_instance._hWndStatusBar,qts.c_str())==0)
		PHTrace(Win32Error(),__LINE__,__FILE__);
}

