#include "resource.h"
#include "..\ProcessHistory\MainFrm.h"
#include "AboutDlg.h"
#include "ph.h"
#include "screen.h"
#include "query.h"
#include "..\phshared\PHShared.h"
#include "nowide\convert.hpp"
#if defined (_WIN64)
#include <thread>
#include <mutex>
#else
#include "boost\thread\mutex.hpp"
#include <boost\thread\lock_guard.hpp> 
#include <boost\thread\thread.hpp>
using namespace boost;
#endif

using namespace std;
using namespace boost::posix_time;

using namespace boost::gregorian;

extern PHDisplay phd;
PH ph_instance;
extern PHQuery phq;
ptime wr,wl;
extern mutex db_mutex;

mutex work_mtx;

void PHJump(string SQL, time_duration td);
struct LRTdata
{
	time_duration td;
	string s;
};
void LeftRightThread(LRTdata lrt)
{
	if (phq.Query() != 0)
	{
		::SetWindowText(ph_instance._hWndStatusBar, "Working...");
		phd._mouseover = phd._selected = -1;
		phd.UpdateWindow();
		phd._complete = true;
		::PostMessage(ph_instance._hWnd, UWM_DOWNLOAD_PROGRESS, 1, 0);
	}
	else
	{
		ostringstream os;
		os << lrt.s;
		//check for processes further than one screen width

		if (FilterUser())
			os << " AND USERID=" << phd.filterUserID;
		if (FilterExec())
			os << " AND CRC=" << phd.filterCRC;
		os << ";";
		PHJump(os.str(), lrt.td);
	}
}

void LeftRight(WPARAM wParam)
{
	wr = phd._WinRight, wl = phd._WinLeft;
	switch (wParam)
	{

	case VK_RIGHT:
	{
		//work_mtx.lock();// avoid deadlock with the recursion
		string where = phq.Construct(phd._WinRight);
		phq.SetSQL(where);
		phd._complete = false;
		
		ostringstream os;
		LRTdata lrtd;
		os << "SELECT DATETIME(MIN(CreationTime)) FROM Process WHERE CreationTime> JULIANDAY('" << BoostToSQLite(wr) << "')";
		lrtd.s = os.str();
		lrtd.td=time_duration(0, 0, 0);
		thread st(LeftRightThread, lrtd);
		st.detach();
	}//case
	break;
	case VK_LEFT:
	{
		LRTdata lrtd;
		//work_mtx.lock();// avoid deadlock with the recursion
		lrtd.td = phd._WinRight - phd._WinLeft;
		string where = phq.Construct(phd._WinLeft - lrtd.td);

		phq.SetSQL(where);

		ostringstream os;
		os << "SELECT DATETIME(MAX(Destruction)) From Process WHERE Destruction < JULIANDAY('" << BoostToSQLite(wl) << "')";
		lrtd.s = os.str();
		phd._complete = false;
		thread st(LeftRightThread, lrtd);
		st.detach();
		
	}
	}
}
void PHJump(string SQL,time_duration td)
{
				sqlite3 *db;
				sqlite3_stmt* stmt;
				{
					lock_guard<mutex> sl(db_mutex);
					db = OpenDB();
					if (sqlite3_prepare(db, SQL.c_str(), -1, &stmt, NULL) != SQLITE_OK)
						DBError(sqlite3_errmsg(db), __LINE__, __FILE__);
				}
				ptime result;
				if (sqlite3_step(stmt) == SQLITE_ROW)
				{
					const unsigned char * jumpresult = 0;
					jumpresult = sqlite3_column_text(stmt, 0);

					if (jumpresult != 0)
					{
						result = time_from_string((const char*)jumpresult);
						string SQLw = phq.Construct(result-td);
						phq.SetSQL(SQLw);
						phd._complete = false;
						if (phq.Query() != 0)
						{
							::SetWindowText(ph_instance._hWndStatusBar, "Working...");
							phd.UpdateWindow();
							phd._complete = true;
							::PostMessage(ph_instance._hWnd, UWM_DOWNLOAD_PROGRESS, 1, 0);
						}
					}
					else
					{
						phd._WinRight = wr;
						phd._WinLeft = wl;
						::SetWindowText(ph_instance._hWndStatusBar, "End of recorded processes");
					}
				}
				else
				{
					::SetWindowText(ph_instance._hWndStatusBar, "End of recorded processes");
					phd._WinRight = wr;
					phd._WinLeft = wl;
				}
				sqlite3_finalize(stmt);
				sqlite3_close(db);
				//::MessageBox(ph_instance._hWnd,"No more processes in database","No processes",MB_OK);
				phd._complete = true;//clears screen on refresh without this
			}//query

#include "..\background\PHacker.h"
#include "..\PHLogger\PHLogger.h"
CPHLogger logger;

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
			// create command bar window
	HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
	// attach menu
	m_CmdBar.AttachMenu(GetMenu());
	// load command bar images
	m_CmdBar.LoadImages(IDR_MAINFRAME);

	// remove old menu
	SetMenu(NULL);

	CreateSimpleStatusBar();
	
	m_sBar.SubclassWindow(m_hWndStatusBar);
	
	int arrParts[] = 
		{ 
			ID_DEFAULT_PANE, 
			ID_PANE_2,
			ID_PANE_1
			};
m_sBar.SetPanes(arrParts, 3, false);

m_sBar.ProgCreate(2 );
phq.long_processes = true;
m_sBar.SetPaneText(ID_PANE_2, "ALL" );
	ph_instance._hWnd=m_hWnd;
	HWND hWndToolBar2 = CreateSimpleToolBarCtrl(m_hWnd, IDR_FILTER_TOOLBAR, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
	HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE_EX);// | TBSTYLE_LIST | CCS_ADJUSTABLE);
	
	AddToolbarButtonText(hWndToolBar, ID_BUTTON40092);
	AddToolbarButtonText(hWndToolBar, ID_BUTTON40070);
	AddToolbarButtonText(hWndToolBar, ID_BUTTON40071);

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
	AddSimpleReBarBand(hWndCmdBar);
	AddSimpleReBarBand(hWndToolBar, NULL, TRUE);
	AddSimpleReBarBand(hWndToolBar2, NULL, TRUE);

	m_Combo = CreateToolbarComboBox(hWndToolBar, ID_PLACEHOLDER , 16, 16, CBS_DROPDOWN);
	m_Combo2 = CreateToolbarComboBox(hWndToolBar2, ID_EXEC_FILTER_COMBO, 50, 16, CBS_DROPDOWN | CBS_AUTOHSCROLL);
	m_Combo3 = CreateToolbarComboBox(hWndToolBar2, ID_USERFILTER_COMBO, 25, 16, CBS_DROPDOWNLIST | CBS_AUTOHSCROLL);

	ph_instance._hWndCombo2=m_Combo2.m_hWnd;
	ph_instance._hWndCombo3 = m_Combo3.m_hWnd;

	m_Combo.SetWindowTextA("YYYY[-MM-DD HH:MM:SS]");
	//m_Combo2.SetWindowTextA("Executable");

	ph_instance._hWndStatusBar=m_hWndStatusBar;

	HWND hWndEdit = ::GetWindow(m_Combo.m_hWnd, GW_CHILD);
	if (hWndEdit)
		m_cmdTextBox.SubclassWindow(hWndEdit);
	
	hWndEdit = ::GetWindow(m_Combo2.m_hWnd, GW_CHILD);
	if (hWndEdit)
		m_cmdTextBox2.SubclassWindow(hWndEdit);
	m_cmdTextBox.hwndMain = m_cmdTextBox2.hwndMain = m_hWnd;

	

	CMessageLoop* pLoop = _Module.GetMessageLoop();
	ATLASSERT(pLoop != NULL);
	pLoop->AddMessageFilter(this);
		
	if( m_wndHorzSplit.Create (*this , rcDefault,"QuerySplit" )==NULL)
	PHTrace(Win32Error(), __LINE__, __FILE__);
	 m_wndHorzSplit.m_bFullDrag = false;
	if( m_wndHorzSplit2.Create(m_wndHorzSplit, rcDefault, "timeinfosplit")==NULL)
	PHTrace(Win32Error(), __LINE__, __FILE__);
	 
	m_hWndClient=m_wndHorzSplit;
	if(phs.Create(m_wndHorzSplit, rcDefault, "QueryResults", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE)==NULL)
		PHTrace(Win32Error(), __LINE__, __FILE__);
	if(phti.Create(m_wndHorzSplit2, rcDefault, "Process", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE)==NULL)
		PHTrace(Win32Error(), __LINE__, __FILE__);
	if(phts.Create(m_wndHorzSplit2, rcDefault, "Time Scale", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE)==NULL)
		PHTrace(Win32Error(), __LINE__, __FILE__);
	
	ph_instance._hWndResults=phs.m_hWnd;
	phti.tidb =  OpenDB();

	m_wndHorzSplit.SetSplitterPanes(phs, m_wndHorzSplit2);
		
	m_wndHorzSplit2.SetSplitterPanes(phts,phti);
	m_wndHorzSplit.SetDefaultActivePane(phs);
	UpdateLayout();
	m_wndHorzSplit.SetSplitterPosPct(75);
	m_wndHorzSplit2.SetSplitterPosPct(25);

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
   
   
   // Initialize GDI+.
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
//#if defined (_WIN64)
	PWSTR pCL = ProcessHackerStart();
	
	if (pCL != 0)
	{
		ProcessHackerVer = nowide::narrow(pCL);
		free(pCL);
	}
//#endif
	ph_instance._hWndProgress = m_sBar.m_Progress;
	logger.StartProcessHistory();
	FillUserList();
	return 0;
}

/*select processes that start,end or overlap the times*/
string ModifiedConstruct(ptime Begin,ptime end)
{
	string SQL=" WHERE ";	

	ostringstream os;
	string begin_txt=BoostToSQLite(Begin),end_txt=BoostToSQLite(end);

	//assert(!begin_txt.empty() || !end_txt.empty());
	os<<"SELECT JULIANDAY('"	<< begin_txt	<<"'),JULIANDAY('"
	<<end_txt	<<"');";

	sqlite3 *db;
	sqlite3_stmt* stmt;
	string ejtxt,bjtxt;
	lock_guard<mutex> sl(db_mutex);
	db = OpenDB();
	if(sqlite3_prepare(db,os.str().c_str(),-1,&stmt,NULL)!=SQLITE_OK)
		DBError(sqlite3_errmsg(db),__LINE__,__FILE__);

	const unsigned char *bjul=0;
	const unsigned char *ejul=0;
	
	if(sqlite3_step(stmt)== SQLITE_ROW )
	{	
		bjul =sqlite3_column_text(stmt,0);		
		ejul =sqlite3_column_text(stmt,1);
	}
	//assert(!bjul==0 || !ejul==0);
	if(bjul==0 || ejul==0)
		PHTrace("Error constructing query",__LINE__,__FILE__);
	ejtxt=(char*)ejul;
	bjtxt=(char*)bjul;
	sqlite3_finalize(stmt);
	sqlite3_close(db);

	os.str("");
	if (phd.filter_exec || phd.filterUserID > 0)
		os << " ( ";
	/*Starts within times - events too*/
	os<<"( CreationTime>"	<<bjtxt	<<" AND CreationTime<"	<<ejtxt

	/*Ends within times*/
	<<" AND Destruction NOTNULL ) OR ( Destruction<"	<<ejtxt	<<" AND Destruction>"	<<bjtxt
	<<" AND Destruction NOTNULL)";
	if(phq.long_processes)
	{
	/*Overlaps times*/
		os<<" OR ( CreationTime<"	<<bjtxt	<<" AND Destruction>"	<<ejtxt
		<<" AND Destruction NOTNULL) ";
	}
	if (phd.filter_exec || phd.filterUserID > 0)
			os << " ) ";
	if(phd.filter_exec)
		os<< " AND CRC="<<phd.filterCRC<<" ";
	if(phd.filterUserID>0)
		os<<" AND USERID="<<phd.filterUserID<<" ";
	SQL+=os.str();
	//PHTrace(SQL, __LINE__, __FILE__);
	return SQL;
}

struct Worker4Thread
{
	date start;
	CComboBox m_Combo;
	string s;
	HWND h;
	CMPSBarWithProgress* m_sBar;
};
//using namespace boost;
void Worker4(Worker4Thread w4t)
{
	lock_guard<mutex> sl(work_mtx);
	month_iterator titr(w4t.start,1);
	w4t.m_sBar->ProgSetRange(0,12);
	sqlite3 *db=OpenDB();
	for(;titr<=date(boost::lexical_cast<int>(w4t.s),12,31);++titr)
	{
		ostringstream os;
		os<<"SELECT ID FROM Process"
			<<	ModifiedConstruct(ptime(*titr,minutes(0)),ptime(*titr+months(1),minutes(0)));
		os<<";";
		
		
		sqlite3_stmt* stmt;
		if(sqlite3_prepare(db,os.str().c_str(),-1,&stmt,NULL)!=SQLITE_OK)
			DBError(sqlite3_errmsg(db),__LINE__,__FILE__);

		date monthit=*titr;
			
		if(sqlite3_step(stmt)== SQLITE_ROW )
			w4t.m_Combo.AddString(to_iso_extended_string(monthit).substr(0,7).c_str());
			
		sqlite3_finalize(stmt);
			
		w4t.m_sBar->ProgSetPos(monthit.month());
	}
	sqlite3_close(db);	
	w4t.m_sBar->ProgSetPos(0);
}

void Worker7(Worker4Thread w4t)
{
	lock_guard<mutex> sl(work_mtx);
	day_iterator ditr(w4t.start);
	w4t.m_sBar->ProgSetRange(0,w4t.start.end_of_month().day());
	sqlite3 *db=OpenDB();
	for(;ditr<=w4t.start.end_of_month();++ditr)
	{
		ostringstream os;
		os<<"SELECT ID FROM Process"
		<<	ModifiedConstruct(ptime(*ditr,minutes(0)),ptime(*ditr+days(1),minutes(0)));
		os<<";";
		
		sqlite3_stmt* stmt;
		if(sqlite3_prepare(db,os.str().c_str(),-1,&stmt,NULL)!=SQLITE_OK)
			DBError(sqlite3_errmsg(db),__LINE__,__FILE__);
		if(sqlite3_step(stmt)== SQLITE_ROW )
			w4t.m_Combo.AddString(to_iso_extended_string(*ditr).c_str());
		sqlite3_finalize(stmt);
				
		w4t.m_sBar->ProgSetPos(ditr->day());
	}
	sqlite3_close(db);
	w4t.m_sBar->ProgSetPos(0);
}
/*for each ROW returned to find out when any processes start in hour*/
void Worker10(Worker4Thread w4t)
{
	lock_guard<mutex> sl(work_mtx);
	time_iterator hitr(ptime(w4t.start),hours(1));
	w4t.m_Combo.ResetContent();
	w4t.m_sBar->ProgSetRange(0,24);
	sqlite3 *db=OpenDB();
	for(;hitr<ptime(w4t.start,hours(24));++hitr)
	{
		ostringstream os;
		os<<"SELECT DATETIME(MIN(CreationTime)) FROM Process"
			<<	ModifiedConstruct(*hitr,(*hitr)+hours(1));
		os<<";";
		
		sqlite3_stmt* stmt;
		if(sqlite3_prepare(db,os.str().c_str(),-1,&stmt,NULL)!=SQLITE_OK)
			DBError(sqlite3_errmsg(db),__LINE__,__FILE__);
				//PHTrace(os.str(),__LINE__,__FILE__);
		if(sqlite3_step(stmt)== SQLITE_ROW )
		{
			const unsigned char * firstinhour=0;
			firstinhour = sqlite3_column_text(stmt,0);
			ptime start_in_hour;
			if(firstinhour!=0)
			{
				try
				{
					start_in_hour=time_from_string((const char *)firstinhour);
				}
				catch(std::exception &)
				{
					PHTrace("time_from_string failed",__LINE__,__FILE__);
				}
				if(start_in_hour>=*hitr && start_in_hour<=*hitr+hours(1))
					w4t.m_Combo.AddString(to_iso_extended_string(start_in_hour).replace(10,1," ").substr(0,16).c_str());
				else
					w4t.m_Combo.AddString(to_iso_extended_string(*hitr).replace(10,1," ").substr(0,16).c_str());
			}
		}
		sqlite3_finalize(stmt);
		
		w4t.m_sBar->ProgSetPos(hitr->time_of_day().hours());
	}
	sqlite3_close(db);	
	w4t.m_sBar->ProgSetPos(0);
}

void CMainFrame::ParseQry()
{
	char q[100];
	int ql=m_Combo.GetWindowTextLength();
	FilterExec();
	FilterUser();
	switch(ql)
	{
	case 16:
		//work_mtx.lock();// avoid deadlock with the recursion
		DateTimeQry();
	break;
	case 4://year list months with data
	{
		m_Combo.GetWindowText(q,ql+1);
		string s=string(q);
		//iterate through creating date time in SQL for begin /end of each month with in year, if any results add to drop down
		date start;
		try
		{
			start = date(boost::lexical_cast<int>(s), 1, 1);
		}
		catch(...)
		{
			::SetWindowText(ph_instance._hWndStatusBar,"Input expected YYYY");
			return;
		}
		
		m_Combo.ResetContent();
		Worker4Thread w4t;
		w4t.m_Combo=m_Combo;
		w4t.s=s;
		w4t.start=start;
		w4t.h=m_hWnd;
		w4t.m_sBar=&m_sBar;
		thread st(&Worker4,w4t);
		st.detach();
	}
	break;
	case 7://year-mm list days
	{
		m_Combo.GetWindowText(q,ql+1);
		string s=string(q);
		
		m_Combo.ResetContent();
		date start;
		try
		{
			s+="-01";
			start=from_string(s);
		}
		catch(...)
		{
			::SetWindowText(ph_instance._hWndStatusBar,"Input expected YYYY-MM");
			return;
		}
		Worker4Thread w4t;
		w4t.m_Combo=m_Combo;
		w4t.start=start;
		w4t.h=m_hWnd;
		w4t.m_sBar=&m_sBar;
		thread st(&Worker7,w4t);
		st.detach();
	}
	break;
	case 10://year-mm-dd list hours in drop down
	{
		m_Combo.GetWindowText(q,ql+1);
		string s=string(q);
		ostringstream os;
		date startd;
		try
		{
		startd=from_string(s);
		}
		catch(...)
		{
			::SetWindowText(ph_instance._hWndStatusBar,"Input expected YYYY-MM-DD");
			return;
		}
		Worker4Thread w4t;
		w4t.m_Combo=m_Combo;
		w4t.start=startd;
		w4t.h=m_hWnd;
		w4t.m_sBar=&m_sBar;
		thread st(&Worker10,w4t);
		st.detach();
	}
	break;
	}
}


LRESULT CMainFrame::OnHelpAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	AboutCtrl ctrl;
	ctrl.DoModal();
	phs.SetFocus();
	return 0;
}


LRESULT CMainFrame::OnViewKey(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
			/*uses about dialog code as both dialogs have the same function*/
	KeyCtrl ctrl;
	ctrl.DoModal();
	phs.SetFocus();//TODO: this causes problems
	return 0;
}

LRESULT CMainFrame::OnProcessViewSelected(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(phd._selected>0)
	{
		//FoundProcess(phd._selected);
		phti._ID=phd._selected;
		phti.DisplayInfo();
		phti.CreateScreenBuffer();
		SIZE sz;
		sz.cx=phti._Width;
		sz.cy=phti._Height;
		phti.SetSize(sz);
		phd._mouseover = -1;
	}
	phs.SetFocus();
	return 0;
	
}
#include <boost/filesystem.hpp>
using namespace boost::filesystem;
LRESULT CMainFrame::OnExecRunSel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(phd._selected>0)
	{
		map<long,string>::iterator qpit=phd.qrypaths.find(phd._selected);
		if(qpit!=phd.qrypaths.end())
		{
			if ( exists(qpit->second))
			{
			if((int)ShellExecute(NULL, "Open",qpit->second.c_str(),NULL,NULL,SW_SHOWDEFAULT)<33)
				PHTrace(Win32Error(),__LINE__,__FILE__);
			}
			else
				MessageBox("The executable no longer exists","Executable no longer exists",MB_OK);
		}
	}
	return 0;
}

LRESULT CMainFrame::OnExecOCF(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	string command;
	command ="/select,";
	if(phd._selected>0)
	{
		map<long,string>::iterator qpit=phd.qrypaths.find(phd._selected);
		if(qpit!=phd.qrypaths.end())
		{
			if ( exists(qpit->second))
			{
			command +=qpit->second;
			
			// open the containing folder
			if((int)ShellExecute(NULL, "Open", "Explorer", command.c_str(), 
                                                        NULL, SW_SHOWDEFAULT)<33)
				PHTrace(Win32Error(),__LINE__,__FILE__);
			}
			else
				MessageBox("The executable no longer exists","Executable no longer exists",MB_OK);
		}
	}
	return 0;
}
void ContextWorker()
{
	phd._complete = false;
	if (phq.Query() != 0)
	{
		phd._mouseover = phd._selected = -1;
		::SetWindowText(ph_instance._hWndStatusBar, "Working...");
		phd.UpdateWindow();
		phd._complete = true;
		::PostMessage(ph_instance._hWnd, UWM_DOWNLOAD_PROGRESS, 1, 0);
	}
}
LRESULT CMainFrame::OnGotoEnd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	map<long,PHProcess>::iterator it=phq._Processes.find(phd._selected);
	if(it!=phq._Processes.end())
	{
		//work_mtx.lock();
		string where=phq.Construct(it->second.end);
		
		phq.SetSQL(where);

		thread st(&ContextWorker);
		st.detach();
	}
	phs.SetFocus();
	return 0;
}

LRESULT CMainFrame::OnGotoBegin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	map<long,PHProcess>::iterator it=phq._Processes.find(phd._selected);
	if(it!=phq._Processes.end())
	{
		//work_mtx.lock();
		string where=phq.Construct(it->second.start);
		
		phq.SetSQL(where);

		thread st(&ContextWorker);
		st.detach();
	}
	phs.SetFocus();
	return 0;
}

LRESULT CMainFrame::PHTBQry(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	//work_mtx.lock();
	ParseQry();
	phs.SetFocus();
	return 0;
}

LRESULT CMainFrame::PHTBLeft(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{	
	if(phd._complete)
	{
		//work_mtx.lock();
		LeftRight(VK_LEFT);
	}
	phs.SetFocus();
	return 0;
}

LRESULT CMainFrame::PHTBRight(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if(phd._complete)
	{
		//work_mtx.lock();
		LeftRight(VK_RIGHT);
	}
	phs.SetFocus();
	return 0;
}

LRESULT CMainFrame::GoToMostRecent(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{		
	sqlite3 *db;
	sqlite3_stmt* stmt;
	ostringstream os;
	os << "SELECT DATETIME(MAX(Destruction)) FROM Process";
	if (FilterUser() || FilterExec())
		os << " WHERE ";
	if (FilterUser())
		os << " USERID=" << phd.filterUserID;
	if (FilterUser() && FilterExec())
		os << " AND ";
		if(FilterExec())
		os << " CRC=" << phd.filterCRC;
	os << ";";
	{
		lock_guard<mutex> sl(db_mutex);
		db = OpenDB();
		if (sqlite3_prepare(db, os.str().c_str(), -1, &stmt, NULL) != SQLITE_OK)
			DBError(sqlite3_errmsg(db), __LINE__, __FILE__);
	}
	if(sqlite3_step(stmt)== SQLITE_ROW )
	{
		//work_mtx.lock();
		const unsigned char * mrt=0;
		mrt=sqlite3_column_text(stmt,0);
		if (mrt != 0)
		{
			string file = (char*)mrt;
			//subtract sreen width
			RECT r;
			phs.GetClientRect( &r);
			int _Width = r.right - r.left;
			float MinutesWide = (float)_Width / 60;
			string where = phq.Construct(time_from_string(file) - time_duration(0, (int)MinutesWide, _Width-((int)MinutesWide*60)));
			//worker_mtx.lock();
			phq.SetSQL(where);
			thread st(&ContextWorker);
			st.detach();
			/*
			else
				::SetWindowText(ph_instance._hWndStatusBar, "No processes in database");*/
		}
		else
			::SetWindowText(ph_instance._hWndStatusBar, "No processes in database");
	}
	else
		::SetWindowText(ph_instance._hWndStatusBar, "No processes in database");
	sqlite3_finalize(stmt);

	sqlite3_close(db);
	phs.SetFocus();
	return 0;
}
#include "atldlgs.h"
#include "..\phshared\Crc32Static.h"
LRESULT CMainFrame::OnFilterExecutable(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	char User[300];
	
	int length = ::GetWindowTextLength(ph_instance._hWndCombo2);
	if (length > 0)
	{
		::GetWindowText(ph_instance._hWndCombo2, User, length + 1);
		//dlg.m_szFileName = User;
		if (string(User) == "Executable")
			strcpy(User, "");
	}
	else
		strcpy(User , "");
CFileDialog dlg(TRUE, NULL, User, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, 
			"Executables (*.exe)\0*.exe\0All Files (*.*)\0*.*\0");

		if(IDOK==dlg.DoModal())
			{//check for cancel

			/*path p;
			try
			{
				string s=dlg.m_szFileName;
				p=path(s,native);
				if(!exists(p))
				{
					MessageBox(ph_instance._hWnd,"Path does not exist.","Filesystem",MB_OK);
					return true;
				}
			}
			catch(boost::exception & e)
			{
				MessageBox(ph_instance._hWnd,"Not a valid path.","Filesystem",MB_OK);
				return true;
			}
			string _path=system_complete(p).native_file_string();
			unsigned long CRC=GetFileCRC(_path);*/
			
			::SetWindowText(ph_instance._hWndStatusBar, "Executable filter set");
			m_Combo2.SetWindowTextA(dlg.m_szFileName);
	}
			
	return 0;
}
LRESULT CMainFrame::OnSetExecFilter(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	if (phd._selected > 0)
	{
		::SetWindowText(ph_instance._hWndStatusBar, "Executable filter set");
		map<long, string>::iterator qpit;
		qpit = phd.qrypaths.find(phd._selected);

		if (qpit != phd.qrypaths.end())
		{
		
		m_Combo2.SetWindowTextA(qpit->second.c_str());
		phd.filter_exec = true;
		ParseQry();
		phs.SetFocus();
	}
	}
	return 0;
}

LRESULT CMainFrame::OnFont(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CFontDialog fd;
	fd.SetLogFont(&phd._font);
	if (IDOK == fd.DoModal())
	{
		phd._font = fd.m_lf;
		if(phs.Invalidate()==0)
		PHTrace(Win32Error(), __LINE__, __FILE__);
		if(phti.Invalidate()==0)
		PHTrace(Win32Error(), __LINE__, __FILE__);
		if(phts.Invalidate()==0)
		PHTrace(Win32Error(), __LINE__, __FILE__);
		if(phs.UpdateWindow()==0)
		PHTrace(Win32Error(), __LINE__, __FILE__);		
		if(phti.UpdateWindow()==0)
		PHTrace(Win32Error(), __LINE__, __FILE__); 
		if(phts.UpdateWindow()==0)
		PHTrace(Win32Error(), __LINE__, __FILE__);
	}
	return 0;
}
//doesn't behave quite right when no results leaves last query on screen but doesn't redraw it.
// -- UI- progress bar  - clk progress to cancel?

// ++ manual control of refresh rate -  time each cycle
// ++sub second processes  visible, draw as greater than a second

//-- UI - for USER AND EXEC run query on dd selection
 

// LEAVE drop crc
//LEAVE optimise - can version info code be dropped, what phacker does?

// LEAVE: show parent child relationship (in details) -> got to parent process???
