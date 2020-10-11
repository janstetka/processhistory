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
	set<HANDLE> OldProcesses;

condition_variable cv,cv_start,cv_stop;
mutex m_mutex,start_mtx,stop_mtx;
void RefreshThread()
{
	CProgressBarCtrl m_sBar;
	m_sBar.Attach(ph_instance._hWndProgress);
	LoadPathData(m_sBar);//TODO 2020 don't read all these in
	//LoadCommandLines(m_sBar); 
	PhpEnablePrivileges();

	
	logger.procdb = OpenDB();
thread worker(WorkerThread);
	while (logger._Refresh > -1)
	{
		Sleep(logger._Refresh);
		//
		lock_guard<mutex> lock(m_mutex);
		cv.notify_one();
	}
}
mutex worker_mtx;
extern void StartEvent();
extern void StopEvent();

queue<qi> start_queue;
queue<HANDLE> stop_queue;

void WorkerThread()
{
	thread start_e(&StartEvent);
	thread stop_e(&StopEvent);

	//set<long> procs;
	while (logger._Refresh > -1) {
		{
			unique_lock<mutex> lk(m_mutex);
			cv.wait(lk);
		}
		//lock_guard<mutex> sl(worker_mtx);
ptime rb = microsec_clock::local_time();

		/*  Take a snapshot of all processes in the system. */

		 //  ntquery is excessively page faulting 

		/*  Walk the snapshot of the processes, and for each process,
		display information. */


		//long ProcessID;
		phqi startqi;
		ProcessHackerInitialGetProcess(&startqi);
		if (startqi.ID )
		{
			set<HANDLE> Processes;
			do
			{
				if (startqi.ID > (HANDLE)4)
				{
					Processes.insert(startqi.ID);

					if (process_map.find(startqi.ID) == process_map.end())//if not already got this one - log
					{
						unique_lock<mutex> lock(start_mtx);
						qi tqi;
						tqi.ID = startqi.ID;
						tqi.parentID = startqi.parentID;
						tqi.st = startqi.st;
						start_queue.push(tqi);
						cv_start.notify_one();
					}
				}
				ProcessHackerGetNextProcess(&startqi);
			} while (startqi.ID );

			//result present in first but not second
			set<HANDLE> result;
			set_difference(OldProcesses.begin(), OldProcesses.end(), Processes.begin(), Processes.end(), inserter(result, result.end()));


			//set<HANDLE>::iterator it;
			for (auto & it : result)
			{
				unique_lock<mutex> lock(stop_mtx);
				stop_queue.push(it);
				cv_stop.notify_one();
			}

			OldProcesses = Processes;
		}
ProcessHackerCleanUp();
ptime re = microsec_clock::local_time();
		time_duration rtd= re - rb;
		//::SetWindowText(ph_instance._hWndStatusBar, (to_simple_string(rtd)+" pm# "+ boost::lexical_cast<string>(process_map.size())).c_str());
	}
		/* Do not forget to clean up the snapshot object. */

	}
