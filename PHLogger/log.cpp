#include "PHLogger.h"
#include "ProcessInfo.h"
#include <sstream>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "..\phshared\phshared.h"
#include "boost/date_time/local_time_adjustor.hpp"
#include "boost/date_time/c_local_time_adjustor.hpp"
#include <set>
#include <mutex>
#include <queue>
#include <..\PHQuery\ph.h>
#include "..\background\phacker.h"

using namespace std;
using namespace boost::posix_time;
using namespace boost::date_time;

/*fine grain threading*/

extern CPHLogger logger;
map<HANDLE,CProcessInfo> process_map;

mutex db_mutex;
extern mutex start_mtx, stop_mtx;
extern condition_variable cv_start, cv_stop;

extern queue<qi> start_queue;
extern queue<HANDLE> stop_queue;
extern PH ph_instance;
void StartEvent()
{
	while (logger._Refresh > -1) {
		qi P;
		{
		unique_lock<mutex> lk(start_mtx);
		while (start_queue.empty())
			cv_start.wait(lk);
		if (start_queue.empty())
			continue;
		//if (start_queue.size() > 1)
		//	::SetWindowText(ph_instance._hWndStatusBar, ("STARTQ "+boost::lexical_cast<string>(start_queue.size())).c_str()); 
		P = start_queue.front();
		//::SetWindowText(ph_instance._hWndStatusBar, ("START PID " + boost::lexical_cast<string>(PID)).c_str());
		start_queue.pop();
	}
		logger.StartEvent(P);
	}
}
void StopEvent()
{
	while (logger._Refresh > -1) {
		HANDLE PID;
		{
			unique_lock<mutex> lk(stop_mtx);
			while (stop_queue.empty())
				cv_stop.wait(lk);
			if (stop_queue.empty())
				continue;
			if (stop_queue.size() > 1)
				::SetWindowText(ph_instance._hWndStatusBar, ("STOPQ "+boost::lexical_cast<string>(stop_queue.size())).c_str());
			PID = stop_queue.front();
			stop_queue.pop();
		}
		logger.StopEvent(PID);
	}
}

/*Process Start*/
void CPHLogger ::StartEvent( qi P)
{
	
	//PHMessage("Start: "+lexical_cast<string>(lPId));

	CProcessInfo clStart;
	//string Path;
	clStart= CProcessInfo(P.ID,P.st);//lPId);	
	clStart.ParentPID=P.parentID;

	/*May have died already*/
	
	/*Has enough data been gathered - missing executable ok, will gather on refresh*/
	if(clStart.Validate()==false)
		return ;

	//clStart.ConvertStartTime();

	HANDLE hProcess = NULL;
		PHackerOpenProcess(P.ID,PROCESS_QUERY_LIMITED_INFORMATION,&hProcess );

	
	PHProcessData phpd;
	phpd.hP=hProcess;
	phpd.time=clStart.GetStart();

	process_map.insert(pair<HANDLE,CProcessInfo>(P.ID,clStart));//CONSTRUCTOR - may be better to not store cprocessinfo in map

	phpd.ProcessID=clStart._ProcessID;

	{
	//mutex::scoped_lock log_scoped_lock(logger.processes_mutex);
	/*This needs to be put in the map whether or not the process is in the data*/
	g_clProcesses.insert(pair<HANDLE,PHProcessData>(P.ID,phpd));
	}
}

class DeadProcess
{
public:
	DeadProcess(){}
	bool ExitTime(HANDLE h)
	{
		/*FILETIME ftCreation,ftExit,ftKernel,ftUser;
		BOOL bRet = GetProcessTimes(h,&ftCreation,&ftExit,&ftKernel,&ftUser);
		if(bRet!=TRUE)
		{
			PHTrace("GetProcessTimes on open dead process handle failed"+string(Win32Error()),__LINE__,__FILE__);
			CloseHandle(h);
			return false;
		}*/
		SYSTEMTIME  ftExit;// , ftKernel, ftUser;
/*Retreive the process start time*/
		ftExit = PHackerGetProcessTimesE(h);//try using handle passed through from refresh rather than opening a new one
		/*Win32 end*/
		try
		{//ELEV - can't get a process end time on another users process unelevated?https://sourceforge.net/p/processhistory/code/25/tree/trunk/PHLogger/
			//_Exit= from_ftime<ptime>(ftExit);
			//_Exit=c_local_adjustor<ptime>::utc_to_local(_Exit);
			_Exit = ptime(boost::gregorian::date(ftExit.wYear, ftExit.wMonth, ftExit.wDay), hours(ftExit.wHour) + minutes(ftExit.wMinute) + seconds(ftExit.wSecond) + milliseconds(ftExit.wMilliseconds));
			// TODO 2020 can anything else be done here - path etc to save time in constructor
		}
		catch(...)
		{
			PHTrace("exception in exit time conversion", __LINE__, __FILE__);
			CloseHandle(h);
			return false;
		}
		
		CloseHandle(h);
		return true;
	}

	ptime _Exit;
	HANDLE _pid;
	sqlite3_int64 _dbid;
};

/*Process end*/
void CPHLogger ::StopEvent(HANDLE lPID)
{	
	//PHMessage("End: "+lexical_cast<string>(lPID));
	
	map<HANDLE,PHProcessData>::iterator llit;
	{
		//mutex::scoped_lock log_scoped_lock(logger.processes_mutex);
		llit=g_clProcesses.find(lPID);
		if(llit==g_clProcesses.end())	
			return;/*It wasn't caught starting so no information can be gathered*/

	}
	DeadProcess dp;
	dp._pid=lPID;
	if(!dp.ExitTime(llit->second.hP))
	{
		//mutex::scoped_lock log_scoped_lock(logger.processes_mutex);
		g_clProcesses.erase(lPID);
		return;
	}
	/*Only allow one thread to execute the code to log an event at a time */
	/* The process is dead, the PID is no longer valid*/
	
	ptime Creation;
	/*Check if the process started whilst PH has been running*/
	/*Which saves a DB read*/

			/*The process will have been logged when it started*/
			/*The image will already be in the data*/
			/*The process is now dead so its entry in the
			map can be removed*/
		dp._dbid=llit->second.ProcessID;
		{	
			//mutex::scoped_lock log_scoped_lock(logger.processes_mutex);
			Creation=llit->second.time;
			g_clProcesses.erase(lPID);
		}					

	/*The process may have started whilst PH wasn't running
	it will have been saved when PH started (See above)*/
	
		//dp.WriteEnd(Creation);	writing everything on process end
		map<HANDLE,CProcessInfo>::iterator pi=process_map.find(lPID);
		if(pi!=process_map.end())
		{
			pi->second.SaveProcess(dp._Exit,false);
		}
		process_map.erase(lPID);
}