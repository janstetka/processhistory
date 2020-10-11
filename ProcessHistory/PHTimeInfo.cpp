#include "..\PHQuery\query.h"
#include "..\phshared\phshared.h"
#include "..\PHQuery\screen.h"
//#include "..\PHQuery\resource.h"
#include "PHScroll.h"
#include <filesystem>
#include <mutex>
#include "boost/format.hpp"

using namespace boost::posix_time;
using namespace std;
//using namespace boost::filesystem;

extern PHQuery phq;
extern PHDisplay phd;
extern mutex db_mutex;

void PHTimeInfo::DoPaint(CDCHandle hDC2)
{
	// query has not finished return 
	if (!phd._complete )
		return;
	
	//  process selected draw from _MemDC 
if ( _MemDC != NULL)
{
	if(hDC2.BitBlt(0, 0, _Width, _Height,*_MemDC, 0, 0, SRCCOPY)==0)
		PHTrace(Win32Error(), __LINE__, __FILE__);
}
	return;
}
void PHTimeInfo::CreateScreenBuffer()
{
	_Width = _Height = 1;
	phd._process_detail = false;

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

	if (_MemDC)
		delete _MemDC;
		_MemDC = new CMemoryDC(GetDC(), rPanel);
	
	CFont hFont;

	if(hFont.CreateFontIndirectA(&phd._font)==NULL)
		PHTrace(Win32Error(), __LINE__, __FILE__);;
	CFont oldFont = _MemDC->SelectFont(hFont);

		RECT r;
		r.left = 0;
		r.right = 1600;
		r.top = 0;
		r.bottom = 900;
		_MemDC->DrawText(_ps.c_str(),static_cast<int>( _ps.length()), &r, DT_CALCRECT);
			//PHTrace(Win32Error(), __LINE__, __FILE__);
		r.bottom += 5;
		r.right += 50;

		_Height = r.bottom;
		_Width = r.right;

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
			delete _MemDC;
		_MemDC = new CMemoryDC(GetDC(), rPanel);
		
		COLORREF cr;
		cr = 0x00FFFFFF;
	
		CBrush FillBrush;
		FillBrush.CreateSolidBrush(cr);

		CFont hFont;

		hFont.CreateFontIndirectA(&phd._font);
		CFont oldFont = _MemDC->SelectFont(hFont);

		if (_MemDC->FillRect(&rPanel, FillBrush) == 0)
			PHTrace(Win32Error(), __LINE__, __FILE__);

		if (_hIcon != NULL)
			if (_MemDC->DrawIcon(10, 5, _hIcon) == 0)
				PHTrace(Win32Error(), __LINE__, __FILE__);
		
		RECT r;
		r.left = 50;
		r.top = 5;
			r.right = _Width;
		r.bottom = _Height;

		_MemDC->DrawText(_ps.c_str(), static_cast<int>(_ps.length()), &r, DT_LEFT | DT_VCENTER);
			//PHTrace(Win32Error(),__LINE__,__FILE__);		
	
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
	if (_MemDC)
		delete _MemDC;
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
	_MemDC->DrawText(TimeText.c_str(), static_cast<int>(TimeText.length()), &r, DT_CALCRECT);
		//PHTrace(Win32Error(), __LINE__, __FILE__);
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
	int64_t wls=wld.seconds();
	int i=0;
	i+=(60- static_cast<int>(wls));
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
		string TimeText = to_string(pti.time_of_day().hours()) + ":" + zero + to_string(pti.time_of_day().minutes());
					
		RECT r;
		r.left = 0;
		r.right = 1600;
		r.top = 0;
		r.bottom = 900;
		_MemDC->DrawText(TimeText.c_str(), static_cast<int>(TimeText.length()), &r, DT_CALCRECT);
				//PHTrace(Win32Error(), __LINE__, __FILE__);
		r.left = i - 15;
		r.top = 10;
		r.bottom += 10;
		r.right += (i - 15);

		_MemDC->DrawText(TimeText.c_str(), static_cast<int>(TimeText.length()), &r, DT_LEFT | DT_VCENTER);
			//PHTrace(Win32Error(), __LINE__, __FILE__);
		
		i+=60;
	}	
	_MemDC->SelectFont(oldFont);
	hFont.DeleteObject();
}

void PHTimeInfo::DisplayInfo()
{
	if (!phd._complete)
		return;

	sqlite3_stmt* stmt,*stmt2;
	string pathtxt,cltxt,usertxt;
	string Product,Description;
	string bintxt,total_duration;
	PHProcess pclProcess;
	time_duration ProcessDuration;

	 if (phd._mouseover>0)
		_ID=phd._mouseover;
	
	map<long, long>::iterator pit;

		pit= phq._PData.find(_ID);	

	if(pit!=phq._PData.end())
	{
		lock_guard<mutex> sl(db_mutex);
			
		string os=			"SELECT CommandLine FROM Process  JOIN CommandLines ON Process.clid=CommandLines.ID WHERE Process.ID="+ to_string(_ID)+";";
			if (sqlite3_prepare(tidb, os.c_str(), -1, &stmt, NULL) != SQLITE_OK)
				DBError(sqlite3_errmsg(tidb), __LINE__, __FILE__);
		const unsigned char *commandline=0;
		if(sqlite3_step(stmt)== SQLITE_ROW )
		{
			commandline=sqlite3_column_text(stmt,0);
			if (commandline != 0)
			cltxt=(char*)commandline;
		}
		sqlite3_finalize(stmt);
		
		string os2;
		os2 = "SELECT UserName FROM Process JOIN PHLogUser ON Process.UserID=PHLogUser.ID WHERE Process.ID= "+to_string(_ID)+";";
		
		if(sqlite3_prepare(tidb,os2.c_str(),-1,&stmt2,NULL)!=SQLITE_OK)
			DBError(sqlite3_errmsg(tidb),__LINE__,__FILE__);
		const unsigned char *user;
		
		if(sqlite3_step(stmt2)== SQLITE_ROW )
		{
			user=sqlite3_column_text(stmt2,0);
			usertxt=(char*)user;
		}
		sqlite3_finalize(stmt2);
		
		map<long, string>::iterator qpit;

		 qpit=phd.qrypaths.find(_ID);


		if(qpit!=phd.qrypaths.end())
		{
			pathtxt=qpit->second;

			if ( std::filesystem::exists(pathtxt))
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

		 proc_it=		phq._Processes.find(_ID);

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
	/*dlgos<<"Start "<<to_simple_string(pclProcess.start)<<" End "<<to_simple_string(pclProcess.end)	
	<<" Duration "<<to_simple_string(ProcessDuration)<<"\r\n"
	
	<<"User: "<<usertxt;

	dlgos << "\r\n" << "Path: " << pathtxt << "\r\n" << "Command line: " << cltxt << "\r\n" << Product << " " << Description
		<< "\r\n" << bintxt << "\r\n";*/
	dlgos << boost::format("Start %s End %s Duration %s \r\nUser: %s \r\nPath: %s \r\nCommand line: %s \r\nProduct %s Description\r\n%s") % pclProcess.start % pclProcess.end % ProcessDuration % usertxt % pathtxt % cltxt % Product % Description % bintxt;

		_ps = dlgos.str();

}
LRESULT PHTimeScale::OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CPaintDC hDC(m_hWnd);
	if (!phd._complete || _MemDC == NULL)
		return TRUE;
	
	if (hDC.BitBlt(0, 0, phd._Width, _HeightTimeLine,*_MemDC, 0, 0, SRCCOPY)==0)
		PHTrace(Win32Error(), __LINE__, __FILE__);
}