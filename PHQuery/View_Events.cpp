#include "PH.h"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "screen.h"
#include "Query.h"
#include "..\phshared\phshared.h"
#include "..\ProcessHistory\MainFrm.h"
#include <mutex>
#include "..\ProcessHistory\Drilldown.h"

using namespace boost::posix_time;
using namespace std;
using namespace boost::gregorian;

extern PHQuery phq;
extern PH ph_instance;
mutex work_mtx;

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
		
	for(/*map<long,PHProcess>::iterator*/ auto& it :	phq._Processes)
	{
		PHProcess pclProcess=it.second;
		map<long,int>::iterator line_it=phq._Lines.find(it.first);
		if(line_it==phq._Lines.end())
			PHTrace("Could not find line for process",__LINE__,__FILE__);
		/*Don't draw beyond edge of window*/
		ptime start_p=pclProcess.start;
		ptime end_p=pclProcess.end;
		if(start_p<_WinLeft)
		{
			_clipped_left.insert(it.first);
			start_p=_WinLeft;
		}
		if(end_p>_WinRight)
		{
			_clipped_right.insert(it.first);
			end_p=_WinRight;
		}
		/*calculate the sizes to draw the processes*/
		RECT rProcess=CalculateRect(start_p,end_p,_WinLeft,line_it->second);
		_ProcessAreas.insert(pair<long,RECT>(it.first,rProcess));
	}
	ReadPaths();	
	
	ptime qrytime=second_clock::local_time();
	string qts="Queried at "+to_simple_string(qrytime/*rdmax*/);
	if(SetWindowText(ph_instance._hWndStatusBar,qts.c_str())==0)
		PHTrace(Win32Error(),__LINE__,__FILE__);
}


//using namespace boost;
void Worker4(Worker4Thread w4t)
{
	lock_guard<mutex> sl(work_mtx);
	month_iterator titr(w4t.start, 1);
	w4t.m_sBar->ProgSetRange(0, 12);
	sqlite3* db = OpenDB();
	for (; titr <= date(boost::lexical_cast<int>(w4t.s), 12, 31); ++titr)
	{
		ostringstream os;
		os << "SELECT ID FROM Process"
			<< ModifiedConstruct(ptime(*titr, minutes(0)), ptime(*titr + months(1), minutes(0)));
		os << ";";


		sqlite3_stmt* stmt;
		if (sqlite3_prepare(db, os.str().c_str(), -1, &stmt, NULL) != SQLITE_OK)
			DBError(sqlite3_errmsg(db), __LINE__, __FILE__);

		date monthit = *titr;

		if (sqlite3_step(stmt) == SQLITE_ROW)
			w4t.m_Combo.AddString(to_iso_extended_string(monthit).substr(0, 7).c_str());

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
	w4t.m_sBar->ProgSetRange(0, w4t.start.end_of_month().day());
	sqlite3* db = OpenDB();
	for (; ditr <= w4t.start.end_of_month(); ++ditr)
	{
		ostringstream os;
		os << "SELECT ID FROM Process"
			<< ModifiedConstruct(ptime(*ditr, minutes(0)), ptime(*ditr + days(1), minutes(0)));
		os << ";";

		sqlite3_stmt* stmt;
		if (sqlite3_prepare(db, os.str().c_str(), -1, &stmt, NULL) != SQLITE_OK)
			DBError(sqlite3_errmsg(db), __LINE__, __FILE__);
		if (sqlite3_step(stmt) == SQLITE_ROW)
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
	time_iterator hitr(ptime(w4t.start), hours(1));
	w4t.m_Combo.ResetContent();
	w4t.m_sBar->ProgSetRange(0, 24);
	sqlite3* db = OpenDB();
	for (; hitr < ptime(w4t.start, hours(24)); ++hitr)
	{
		ostringstream os;
		os << "SELECT DATETIME(MIN(CreationTime)) FROM Process"
			<< ModifiedConstruct(*hitr, (*hitr) + hours(1));
		os << ";";

		sqlite3_stmt* stmt;
		if (sqlite3_prepare(db, os.str().c_str(), -1, &stmt, NULL) != SQLITE_OK)
			DBError(sqlite3_errmsg(db), __LINE__, __FILE__);
		//PHTrace(os.str(),__LINE__,__FILE__);
		if (sqlite3_step(stmt) == SQLITE_ROW)
		{
			const unsigned char* firstinhour = 0;
			firstinhour = sqlite3_column_text(stmt, 0);
			ptime start_in_hour;
			if (firstinhour != 0)
			{
				try
				{
					start_in_hour = time_from_string((const char*)firstinhour);
				}
				catch (std::exception&)
				{
					PHTrace("time_from_string failed", __LINE__, __FILE__);
				}
				if (start_in_hour >= *hitr && start_in_hour <= *hitr + hours(1))
					w4t.m_Combo.AddString(to_iso_extended_string(start_in_hour).replace(10, 1, " ").substr(0, 16).c_str());
				else
					w4t.m_Combo.AddString(to_iso_extended_string(*hitr).replace(10, 1, " ").substr(0, 16).c_str());
			}
		}
		sqlite3_finalize(stmt);

		w4t.m_sBar->ProgSetPos(static_cast<int>(hitr->time_of_day().hours()));
	}
	sqlite3_close(db);
	w4t.m_sBar->ProgSetPos(0);
}
