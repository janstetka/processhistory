#include "..\PHQuery\query.h"
#include "..\phshared\phshared.h"
#include "..\PHQuery\screen.h"
#include "..\PHQuery\resource.h"
#include "PHScroll.h"
#include <boost/filesystem.hpp>
#include <mutex>

using namespace boost::posix_time;
using namespace std;
using namespace boost;

extern PHQuery phq;
extern PHDisplay phd;
extern mutex db_mutex;
mutex proc_det;

LRESULT PHTimeInfo::OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{

	if (!phd._complete || _MemDC == NULL)
		return 0;
	lock_guard<mutex> sl(proc_det);
	CPaintDC hDC2(m_hWnd);
	if (phd._mouseover <= 0 && phd._selected <= 0)
	{
		RECT r;
		r.left = r.top = 0;
		r.right = _Width;
		r.bottom = _Height;
		COLORREF cr;
		cr = 0x00FFFFFF;

		CBrush FillBrush;
		FillBrush.CreateSolidBrush(cr);
		if (hDC2.FillRect(&r, FillBrush) == 0)
			PHTrace(Win32Error(), __LINE__, __FILE__);

	}
if ( phd._mouseover>0 && phd._selected<=0)
	{
	CFont hFont;

	hFont.CreateFontIndirectA(&phd._font);
	CFont oldFont = hDC2.SelectFont(hFont);

		if (_hIcon != NULL)
			if (hDC2.DrawIcon(10, 5, _hIcon) == 0)
				PHTrace(Win32Error(), __LINE__, __FILE__);
		string processinfo;

			processinfo = _ps2;
		RECT r;
		r.left = 0;
		r.right = 1600;
		r.top = 0;
		r.bottom = 900;
		if (hDC2.DrawText(processinfo.c_str(), processinfo.length(), &r, DT_CALCRECT) == 0)
			PHTrace(Win32Error(), __LINE__, __FILE__);
		r.bottom += 5;
		r.right += 50;
		r.left = 50;
		r.top = 5;
		
		_Height = r.bottom;
				
			_Width = phd._Width;

		if (hDC2.DrawText(processinfo.c_str(), processinfo.length(), &r, DT_LEFT | DT_VCENTER) == 0)
				PHTrace(Win32Error(), __LINE__, __FILE__);
	
hDC2.SelectFont(oldFont);
	hFont.DeleteObject();

}
else if (phd._selected > 0)
{
	RECT r;
	r.top = 0;
	r.left = 0;
	r.right = _Width;
	r.bottom = _Height;

	hDC2.SetViewportOrg(-m_ptOffset.x, -m_ptOffset.y);

	if (m_ptOffset.x == 0 && m_ptOffset.y == 0)
		r = hDC2.m_ps.rcPaint;//TODO: Is this causing drawing problems?

	if(hDC2.BitBlt(r.left, r.top, r.right - r.left, r.bottom - r.top,
		*_MemDC, r.left, r.top, SRCCOPY)==0)
		PHTrace(Win32Error(), __LINE__, __FILE__);
}
	return 0;
}
void PHTimeInfo::CreateScreenBuffer()
{
	_Width = _Height = 100;
	phd._process_detail = false;
	lock_guard<mutex> sl(proc_det);
	CalculateRect();
	DrawMemDC();

	phd._process_detail = true;
}
void PHTimeInfo::CalculateRect()
{
	RECT rPanel;
	rPanel.left = rPanel.top = 0;
	rPanel.right = _Width;
	rPanel.bottom = _Height;

	if (!_MemDC)
		_MemDC = new CMemoryDC(GetDC(), rPanel);

	CFont hFont;

	if(hFont.CreateFontIndirectA(&phd._font)==NULL)
		PHTrace(Win32Error(), __LINE__, __FILE__);;
	CFont oldFont = _MemDC->SelectFont(hFont);

	if (phd._selected > 0)
	{

		string processinfo;

			processinfo = _ps;

		RECT r;
		r.left = 0;
		r.right = 1600;
		r.top = 0;
		r.bottom = 900;
		if (_MemDC->DrawText(processinfo.c_str(), processinfo.length(), &r, DT_CALCRECT) == 0)
			PHTrace(Win32Error(), __LINE__, __FILE__);
		r.bottom += 5;
		r.right += 50;

		_Height = r.bottom;
		_Width = r.right;

	}
	_MemDC->SelectFont(oldFont);
	hFont.DeleteObject();
}
void PHTimeInfo::DrawMemDC()
	{
		RECT rPanel;
		rPanel.left = rPanel.top = 0;
		rPanel.right = _Width;
		rPanel.bottom = _Height;
		if (_MemDC)
		{
			delete _MemDC;
			_MemDC = new CMemoryDC(GetDC(), rPanel);
		}
		COLORREF cr;
		cr = 0x00FFFFFF;
	
		CBrush FillBrush;
		FillBrush.CreateSolidBrush(cr);

		CFont hFont;

		hFont.CreateFontIndirectA(&phd._font);
		CFont oldFont = _MemDC->SelectFont(hFont);

		if (_MemDC->FillRect(&rPanel, FillBrush) == 0)
			PHTrace(Win32Error(), __LINE__, __FILE__);
		string processinfo = _ps;
		if (_hIcon != NULL)
			if (_MemDC->DrawIcon(10, 5, _hIcon) == 0)
				PHTrace(Win32Error(), __LINE__, __FILE__);
		
		RECT r;
		r.left = 50;
		r.top = 5;
			r.right = _Width;
		r.bottom = _Height;

		if (_MemDC->DrawText(processinfo.c_str(), processinfo.length(), &r, DT_LEFT | DT_VCENTER) == 0)
			PHTrace(Win32Error(),__LINE__,__FILE__);		
	
	_MemDC->SelectFont(oldFont);
		hFont.DeleteObject();
	}
	
void PHTimeInfo::SetSize(SIZE m_size)
{		
	SetScrollOffset(0, 0, FALSE);
	SetScrollSize(m_size);
}

void PHTimeScale::CalcTimeScale()
{
	RECT rPanel;
	rPanel.left = rPanel.top = 0;
	rPanel.right = phd._Width;
	rPanel.bottom = _HeightTimeLine;
	if (!_MemDC)
		_MemDC = new CMemoryDC(GetDC(), rPanel);
	CFont hFont;

	hFont.CreateFontIndirectA(&phd._font);
	CFont oldFont = _MemDC->SelectFont(hFont);
	string TimeText = "00:00";
	RECT r;
	r.left = 0;
	r.right = 1600;
	r.top = 0;
	r.bottom = 900;
	if (_MemDC->DrawText(TimeText.c_str(), TimeText.length(), &r, DT_CALCRECT) == 0)
		PHTrace(Win32Error(), __LINE__, __FILE__);
	_HeightTimeLine = r.bottom+10;
	_MemDC->SelectFont(oldFont);
	hFont.DeleteObject();
}
	/* draw with markers on the minute rather than minutes from the query start time
calculate where the first minute will be*/
void PHTimeScale::DrawTimeAxis()
{
	RECT rPanel;
	rPanel.left = rPanel.top = 0;
	rPanel.right = phd._Width;
	rPanel.bottom = _HeightTimeLine;

		if (_MemDC)
			delete _MemDC;
		_MemDC = new CMemoryDC(GetDC(), rPanel);

	COLORREF cr;
	cr = 0x00FFFFFF;

	CBrush FillBrush;
	FillBrush.CreateSolidBrush(cr);

	if (_MemDC->FillRect(&rPanel, FillBrush) == 0)
		PHTrace(Win32Error(), __LINE__, __FILE__);

	CFont hFont;

	hFont.CreateFontIndirectA(&phd._font);
	CFont oldFont = _MemDC->SelectFont(hFont);

	time_duration wld=phd._WinLeft.time_of_day();
	int wls=wld.seconds();
	int i=0;
	i+=(60-wls);
	/* draw markers and times*/
	//if()//if screen width is less than a minute, mark seconds, greater than a minute mark minutes
	time_iterator titr(phd._WinLeft+seconds(i),minutes(1)); 
	/*if()
	time_iterator titr(phd._WinLeft+fractional_seconds(1)*scale,seconds());*/
	for (; titr < phd._WinRight; ++titr) 
	{
		ptime pti=*titr;
		string zero;
		if(pti.time_of_day().minutes()>9)
			zero="";
		else	
			zero="0";
		string TimeText=lexical_cast<string>(pti.time_of_day().hours())+":"+zero+lexical_cast<string>(pti.time_of_day().minutes());
					
		RECT r;
		r.left = 0;
		r.right = 1600;
		r.top = 0;
		r.bottom = 900;
		if (_MemDC->DrawText(TimeText.c_str(), TimeText.length(), &r, DT_CALCRECT) == 0)
				PHTrace(Win32Error(), __LINE__, __FILE__);
		r.left = i - 15;
		r.top = 10;
		r.bottom += 10;
		r.right += (i - 15);

		if (_MemDC->DrawText(TimeText.c_str(), TimeText.length(), &r, DT_LEFT | DT_VCENTER) == 0)
			PHTrace(Win32Error(), __LINE__, __FILE__);
		
		i+=60;
	}	
	_MemDC->SelectFont(oldFont);
	hFont.DeleteObject();
}

/*struct dcs
{
	string s;
	HICON hi;
} ;
map<long, dcs> dispcache;*///DONE: put this back how it was before 

void PHTimeInfo::DisplayInfo()
{
	if (!phd._complete)
		return;

	/*if (phd._selected > 0)//can't rely on this as mouse is hovering over process when right click
	{
		map<long, dcs>::iterator it = dispcache.find(phd._mouseover);
		if (it != dispcache.end())
		{
			_ps = it->second.s;
			_hIcon = it->second.hi;
			return;
		}
	}
	else if (phd._mouseover > 0)
	{
		
		map<long, dcs>::iterator it = dispcache.find(phd._mouseover);
		if (it != dispcache.end())
		{
			_ps2 = it->second.s;
			_hIcon = it->second.hi;
			return;
		}
	}*/
	sqlite3_stmt* stmt,*stmt2;
	string pathtxt,cltxt,usertxt;
	string Product,Description;
	string bintxt,total_duration;
	PHProcess pclProcess;
	time_duration ProcessDuration;
	unsigned long CRC;

	
	map<long, long>::iterator pit;
	if (phd._selected>0)//can't rely on this as mouse is hovering over process when right click
		pit= phq._PData.find(_ID);	
	else if (phd._mouseover>0)
		pit = phq._PData.find(phd._mouseover);
	if(pit!=phq._PData.end())
	{
		lock_guard<mutex> sl(db_mutex);
		sqlite3 *db=OpenDB();
	
		ostringstream os;
		os <<
			"SELECT CommandLine FROM Process  JOIN CommandLines ON Process.clid=CommandLines.ID WHERE Process.ID=";
		if (phd._mouseover < 1)
			os << _ID;
		else
			os << phd._mouseover;
			os<<";";
		if(sqlite3_prepare(db,os.str().c_str(),-1,&stmt,NULL)!=SQLITE_OK)
			DBError(sqlite3_errmsg(db),__LINE__,__FILE__);
		const unsigned char *commandline=0;
		if(sqlite3_step(stmt)== SQLITE_ROW )
		{
			commandline=sqlite3_column_text(stmt,0);
			if (commandline != 0)
			cltxt=(char*)commandline;
		}
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		ostringstream os2;
		os2 << "SELECT UserName,CRC FROM Process JOIN PHLogUser ON Process.UserID=PHLogUser.ID WHERE Process.ID= ";
		if (phd._selected > 0)
			os2 << _ID;
		else if(phd._mouseover>0)
			os2 << phd._mouseover;		
		os2 << ";";
		sqlite3 *db2=OpenDB();
		if(sqlite3_prepare(db2,os2.str().c_str(),-1,&stmt2,NULL)!=SQLITE_OK)
			DBError(sqlite3_errmsg(db2),__LINE__,__FILE__);
		const unsigned char *user;
		
		if(sqlite3_step(stmt2)== SQLITE_ROW )
		{
			user=sqlite3_column_text(stmt2,0);
			usertxt=(char*)user;
			CRC=sqlite3_column_int(stmt2,1);
		}
		sqlite3_finalize(stmt2);
		sqlite3_close(db2);
		map<long, string>::iterator qpit;
		if (phd._selected>0)
		 qpit=phd.qrypaths.find(_ID);
		else if (phd._mouseover>0)
			qpit = phd.qrypaths.find(phd._mouseover);

		if(qpit!=phd.qrypaths.end())
		{
			pathtxt=qpit->second;

			if ( boost::filesystem::exists(pathtxt))
			{

			GetVersionInfo(Product,Description,pathtxt);
		
			DWORD bin_type;
			if(GetBinaryType(pathtxt.c_str(),&bin_type)!=0)
			{
			if(bin_type==SCS_64BIT_BINARY)
				bintxt="64 bit executable";
			if(bin_type==SCS_32BIT_BINARY)
				bintxt="32 bit executable";
			}
			}
			else
				bintxt="Executable no longer exists";
		}
		
		map<long, PHProcess>::iterator proc_it;
		if (phd._selected>0)
		 proc_it=		phq._Processes.find(_ID);
		else if (phd._mouseover>0)
			proc_it = phq._Processes.find(phd._mouseover);
		if(proc_it!=phq._Processes.end())
		{
			pclProcess=proc_it->second;
			ProcessDuration=pclProcess.end-pclProcess.start;
		}

		map<long,HICON>::iterator iconit=phd.icons.find(pit->second);
		if(iconit!=phd.icons.end())
			_hIcon=iconit->second;
		else
			_hIcon=NULL;
	}	

	ostringstream dlgos;
	dlgos<<"Start "<<to_simple_string(pclProcess.start)<<" End "<<to_simple_string(pclProcess.end)	
	<<" Duration "<<to_simple_string(ProcessDuration)<<"\r\n"
	
	<<"User: "<<usertxt;

	dlgos<<"\r\n"<<"Path: "<<pathtxt<<"\r\n"<<"Command line: "<<cltxt<<"\r\n"<<Product<<" "<<Description
	<<"\r\n"<<bintxt<<"\r\n"<<"CRC: "<<CRC;

	if (phd._selected > 0)
	{
		_ps = dlgos.str();
		/*dcs d;
		d.hi = _hIcon;
		d.s = _ps;
		dispcache.insert(pair<long,dcs>(phd._selected,d));*/
	}
	else if (phd._mouseover > 0)
	{
		_ps2 = dlgos.str();
		/*dcs d;
		d.hi = _hIcon;
		d.s = _ps2;
		dispcache.insert(pair<long, dcs>(phd._mouseover, d));*/
	}
	SIZE sz;
	sz.cx=1;
	sz.cy=150;
	SetSize(sz);
}
LRESULT PHTimeScale::OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if (!phd._complete || _MemDC == NULL)
		return TRUE;

	CPaintDC hDC(m_hWnd);

	RECT r = hDC.m_ps.rcPaint;

		if(hDC.BitBlt(r.left, r.top, r.right - r.left, r.bottom - r.top,
		*_MemDC, r.left, r.top, SRCCOPY)==0)
		PHTrace(Win32Error(), __LINE__, __FILE__);

	return TRUE;
}