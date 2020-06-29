#include <map>
#include <set>
#include "..\phlogger\ProcessInfo.h"
//#include <tlhelp32.h>

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
extern map<long,CProcessInfo> process_map;
extern PH ph_instance;
void WorkerThread();
	set<long> OldProcesses;
	//set<long> ignore;
condition_variable cv,cv_start,cv_stop;
mutex m_mutex,start_mtx,stop_mtx;
void RefreshThread()
{
	CProgressBarCtrl m_sBar;
	m_sBar.Attach(ph_instance._hWndProgress);
	LoadPathData(m_sBar);
	//LoadCommandLines(m_sBar);//TODO 2020 don't read all these in / generally could minimise reads, lots of depreciated in compile 
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

queue<long> start_queue,stop_queue;

void WorkerThread()
{
	thread start_e(&StartEvent);
	thread stop_e(&StopEvent);

	set<long> procs;
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


		long ProcessID;
		ProcessID = ProcessHackerInitialGetProcess();
		if (ProcessID > -1)
		{
			set<long> Processes;
			do
			{
				Processes.insert(ProcessID);
				
				if (process_map.find(ProcessID) == process_map.end())//if not already got this one - log
				{
					//, ProcessID);
					HANDLE hProcess = OpenProcess(
						PROCESS_QUERY_INFORMATION,FALSE, ProcessID);
					if (hProcess != NULL)
					{
						if (procs.find(ProcessID) == procs.end()) //TODO  not elevated  PHack is using PROCESS_QUERY_LIMITED_INFORMATION,
						{
							unique_lock<mutex> lock(start_mtx);
							start_queue.push(ProcessID);
							cv_start.notify_one();
							procs.insert(ProcessID);
						}
						CloseHandle(hProcess);
					}

				}
				ProcessID = ProcessHackerGetNextProcess();
			} while (ProcessID > -1);

			//result present in first but not second
			set<long> result;
			set_difference(OldProcesses.begin(), OldProcesses.end(), Processes.begin(), Processes.end(), inserter(result, result.end()));


			set<long>::iterator it;
			for (it = result.begin(); it != result.end(); it++)
			{
				if(procs.find(*it)!=procs.end())
					procs.erase(*it);
				unique_lock<mutex> lock(stop_mtx);
				stop_queue.push(*it);
				cv_stop.notify_one();
			}

			OldProcesses = Processes;
		}
ProcessHackerCleanUp();
ptime re = microsec_clock::local_time();
		time_duration rtd= re - rb;
		::SetWindowText(ph_instance._hWndStatusBar, (to_simple_string(rtd)+" refresh#"+ boost::lexical_cast<string>(procs.size())+"pm# "+ boost::lexical_cast<string>(process_map.size())).c_str());
	}
		/* Do not forget to clean up the snapshot object. */

	}
