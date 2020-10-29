#include <map>
#include <set>
#include "..\phlogger\ProcessInfo.h"

using namespace std;

#include "..\phlogger\PHLogger.h"
#include <algorithm>
#include "..\phquery\PH.h"
#include "..\ProcessHistory\Progress.h"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "..\background\phacker.h"
#include <thread>
#include <mutex>
#include <queue>
using namespace boost::posix_time;

extern CPHLogger logger;
extern map<HANDLE,CProcessInfo> process_map;
extern PH ph_instance;
void WorkerThread();
	

condition_variable cv,cv_start,cv_stop;
mutex m_mutex,start_mtx,stop_mtx;

//phrefresh.cpp - timer thread, process start/stop detection thread. Ensures regular timing 
extern void StartEvent();
extern void StopEvent();

void RefreshThread()
{
	CProgressBarCtrl m_sBar;
	m_sBar.Attach(ph_instance._hWndProgress);
	LoadPathData(m_sBar);//TODO 2020 don't read all these in
	//LoadCommandLines(m_sBar); 
	PhpEnablePrivileges();

	//long lived db handle for  efficiency
	logger.procdb = OpenDB();
	//launch process event detection thread
	thread worker(WorkerThread);
	thread start_e(&StartEvent);
	thread stop_e(&StopEvent);

	while (logger._Refresh > -1)
	{
		Sleep(logger._Refresh);
		//refresh interval lock
		lock_guard<mutex> lock(m_mutex);
		cv.notify_one();
	}
}

set<qi> start_queue;
queue<HANDLE> stop_queue;
ptime rb;
//rationale for detect & gather information considering 
// - ensure start events always dealt with first by condition variabe 
// - ensure exclusivity of enumeration, start events and stop events by condition variable
// keep start queues small for info gathering 
// match frequency of process events

//  scalable transfer events info gather thread via minimal number of threads to reduce cpu load 


//

// requirements
// enumeration : on time, exclusive, first
// start events : second
// stop events: later, expensive 


//stop events can be queued
void WorkerThread()
{
	set<HANDLE> OldProcesses;
	while (logger._Refresh > -1) {
		{//lock on the refresh interval
			unique_lock<mutex> lk(m_mutex);
			cv.wait(lk);
		}


		rb = microsec_clock::local_time();

		//  ntquery is excessively page faulting 

		phqi startqi;
		
		

		if (ProcessHackerInitialGetProcess(&startqi)>-1)
		{
			set<HANDLE> Processes;
			bool startq=false;
			do
			{
				if (startqi.ID > (HANDLE)4)
				{
					Processes.insert(startqi.ID);

					if (process_map.find(startqi.ID) == process_map.end())//if not already got this one - process creation event
					{
						unique_lock<mutex> lk(start_mtx);// only lock when we have something to put in queue
						qi tqi;
						tqi.ID = startqi.ID;
						tqi.parentID = startqi.parentID;
						tqi.st = startqi.st;
						tqi.pt = boost::posix_time::ptime(boost::gregorian::date(tqi.st.wYear, tqi.st.wMonth, tqi.st.wDay), boost::posix_time::hours(tqi.st.wHour) + boost::posix_time::minutes(tqi.st.wMinute) + boost::posix_time::seconds(tqi.st.wSecond) + boost::posix_time::milliseconds(tqi.st.wMilliseconds));
						start_queue.insert(tqi);
						startq = true;
					}
				}
			} while (ProcessHackerGetNextProcess(&startqi) > -1);
			if(startq)
				cv_start.notify_one();//notify when queue is full fairer - set is ordered  on start time on every insertion
			//result present in first but not second = process exit event	
			set<HANDLE> result;
			set_difference(OldProcesses.begin(), OldProcesses.end(), Processes.begin(), Processes.end(), inserter(result, result.end()));
			{
				unique_lock<mutex> lk(stop_mtx);
				for (auto& it : result)
					stop_queue.push(it);
				if (!startq && !stop_queue.empty())
					cv_stop.notify_one();
			}
			OldProcesses = Processes;


			//timing - refresh interval should not be less than time refresh takes
			ptime re = microsec_clock::local_time();
			time_duration rtd = re - rb;
			//rtds=to_simple_string(rtd);

			ProcessHackerCleanUp();

	}
	}//while
}//worker thread
