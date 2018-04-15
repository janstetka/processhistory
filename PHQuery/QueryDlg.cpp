#include "..\ProcessHistory\MainFrm.h"
//#include "wtl\wtl.h"
#include <atlctrls.h>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "PH.h"
#include "screen.h"
#include "query.h"
#include "..\ProcessHistory\PHScroll.h"
#include <thread>
#include "..\phshared\phshared.h"
//#include <mutex>

using namespace boost::posix_time;
using namespace boost;
using namespace std;

extern PH ph_instance;
extern PHDisplay phd;
extern PHQuery phq;
//extern mutex work_mtx;

void Worker();

void CMainFrame::DateTimeQry()
{	
		char q[100];
		int ql=m_Combo.GetWindowTextLength();
		m_Combo.GetWindowText(q,ql+1);
		/*catch exceptions thrown by boost*/
		ptime Begin;
		string s=string(q);//+".000";
		try 
		{			
			Begin=time_from_string(s);
		}
		catch(boost::exception &)
		{
			::SetWindowText(ph_instance._hWndStatusBar,"Input not in YYYY-MM-DD HH:MM:SS format.");
			return ;
		}

		if(Begin.is_not_a_date_time() )
		{
			::SetWindowText(ph_instance._hWndStatusBar,"Input not in YYYY-MM-DD HH:MM:SS format.");
			return ;
		}

		
string SQL=phq.Construct(Begin);

		/*Query the data*/
		phq.SetSQL(SQL);
		thread st(&Worker);
		st.detach();
		phd._mouseover = phd._selected = -1;
}
void Worker()
{
	//mutex::scoped_lock work_sl(work_mtx);
	//work_mtx.lock();
	::SetWindowText(ph_instance._hWndStatusBar,"Working...");
	phd._complete=false;
	if(phq.Query()==0)			
	{
		::SetWindowText(ph_instance._hWndStatusBar,"No results. Try a broader search by year/month/day.");
		//work_mtx.unlock();
	}
	else
	{
			/*set the display*/
		phd.UpdateWindow();
		phd._complete=true;
		::PostMessage (ph_instance._hWnd, UWM_DOWNLOAD_PROGRESS, 0, 0 );
	}
	phd._complete=true;
}

LRESULT CMainFrame:: NotifyGUI (UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam == 2)//mouseover found
	{
		if (phti._MemDC!=NULL)
		{
			RECT r;
			r.top = 0;
			r.bottom = phti._Height;
			r.left = 0;
			r.right = phti._Width;
			COLORREF cr;
			cr = 0x00FFFFFF;

			CBrush FillBrush;
			FillBrush.CreateSolidBrush(cr);
			if (phti._MemDC->FillRect(&r, FillBrush) == 0)
				PHTrace(Win32Error(), __LINE__, __FILE__);
			if (phti.InvalidateRect(&r, FALSE) == 0)
				PHTrace(Win32Error(), __LINE__, __FILE__);
			if (phti.UpdateWindow() == 0)
				PHTrace(Win32Error(), __LINE__, __FILE__);
		}

		if (phd._mouseover > 0)
		{
			phti.DisplayInfo();
			phti.CreateScreenBuffer();
		
		RECT r;
		r.top = 0;
		r.bottom = phti._Height;
		r.left = 0;
		
		r.right = phti._Width;
		if(phti.InvalidateRect(&r,TRUE)==0)
			PHTrace(Win32Error(), __LINE__, __FILE__);
		if(phti.UpdateWindow()==0)
		PHTrace(Win32Error(), __LINE__, __FILE__);
		}
	
		return 0;
	}
	if (wParam == 3 || wParam == 4)//mouseover gone into whitespace or rh mouse not found
	{
		RECT r;
		r.top = 0;
		r.bottom = 1;
		r.left = 0;
		r.right = 1;
		if (phti._MemDC != NULL)
			delete phti._MemDC;
		phti._MemDC = new  CMemoryDC(GetDC(), r);		
		
		if(phti.UpdateWindow()==0)
		PHTrace(Win32Error(), __LINE__, __FILE__);
		
		SIZE s;
		s.cy = s.cx = 1;
		phti.SetSize(s);
		
		return 0;
	}

	phs.CreateScreenBuffer();
	phts.CalcTimeScale();
	RECT phtsr;
	phtsr.left = phtsr.top = 0;
	phtsr.bottom = phts._HeightTimeLine;
	phtsr.right = phd._Width;
	if(phts.InvalidateRect(&phtsr)==0)
	PHTrace(Win32Error(), __LINE__, __FILE__);
	phts.DrawTimeAxis();
	if(phts.UpdateWindow()==0)
	PHTrace(Win32Error(), __LINE__, __FILE__);
	SIZE s;
	s.cx=phd._Width;
	s.cy=41*(phq._lCount);
			
	phs.SetSize(s);
	s.cy =s.cx= 1;
	phti.SetSize(s);
	//if(phs.Invalidate()==0)
	//PHTrace(Win32Error(), __LINE__, __FILE__);
	if(phti.Invalidate()==0)
	PHTrace(Win32Error(), __LINE__, __FILE__);
	if(phs.UpdateWindow()==0)
	PHTrace(Win32Error(), __LINE__, __FILE__);
	if(phti.UpdateWindow()==0)
	PHTrace(Win32Error(), __LINE__, __FILE__);
	FillUserList();

	if(wParam==1)
	{
	string dt;
		dt=to_iso_extended_string(phd._WinLeft);
		dt.replace(10,1," ");
		//remove seconds
		m_Combo.SetWindowText(dt.substr(0,dt.length()-3).c_str());
	}

	//work_mtx.unlock();
	
	return 0;
}