#include "PHLogger.h"
#include "ProcessInfo.h"
#include <sstream>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "..\phshared\phshared.h"
#include "boost/date_time/local_time_adjustor.hpp"
#include "boost/date_time/c_local_time_adjustor.hpp"
#include <set>
#include <mutex>

using namespace std;
using namespace boost::posix_time;
using namespace boost;

/*fine grain threading*/

extern CPHLogger logger;
map<long,CProcessInfo> process_map;

mutex db_mutex;

/*Process Start*/
void CPHLogger ::StartEvent( long lPId)
{
	//PHMessage("Start: "+lexical_cast<string>(lPId));

	CProcessInfo clStart;
	//string Path;
	clStart= CProcessInfo(lPId);	

	/*May have died already*/
	if (clStart.Validate() == false)
		return;

	clStart.GetExecutableImage();
	
	/*Has enough data been gathered - missing executable ok, will gather on refresh*/
	if(clStart.Validate()==false)
		return ;

	clStart.ConvertStartTime();

	HANDLE hProcess=OpenProcess(PROCESS_QUERY_INFORMATION ,FALSE,lPId);

	
	PHProcessData phpd;
	phpd.hP=hProcess;
	phpd.time=clStart.GetStart();

	process_map.insert(pair<long,CProcessInfo>(lPId,clStart));//CONSTRUCTOR - may be better to not store cprocessinfo in map

	phpd.ProcessID=clStart._ProcessID;

	{
	//mutex::scoped_lock log_scoped_lock(logger.processes_mutex);
	/*This needs to be put in the map whether or not the process is in the data*/
	logger.g_clProcesses.insert(pair<long,PHProcessData>(lPId,phpd));
	}
}

class DeadProcess
{
public:
	DeadProcess(){}
	bool ExitTime(HANDLE h)
	{
		FILETIME ftCreation,ftExit,ftKernel,ftUser;
		BOOL bRet = GetProcessTimes(h,&ftCreation,&ftExit,&ftKernel,&ftUser);
		if(bRet!=TRUE)
		{
			PHTrace("GetProcessTimes on open dead process handle failed"+string(Win32Error()),__LINE__,__FILE__);
			CloseHandle(h);
			return false;
		}
		
		/*Win32 end*/
		try
		{
			_Exit= from_ftime<ptime>(ftExit);
			_Exit=date_time::c_local_adjustor<ptime>::utc_to_local(_Exit);
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
	int _pid;
	long _dbid;
};

/*Process end*/
void CPHLogger ::StopEvent(long lPID)
{	
	//PHMessage("End: "+lexical_cast<string>(lPID));
	
	map<long,PHProcessData>::iterator llit;
	{
		//mutex::scoped_lock log_scoped_lock(logger.processes_mutex);
		llit=logger.g_clProcesses.find(lPID);
		if(llit==logger.g_clProcesses.end())	
			return;/*It wasn't caught starting so no information can be gathered*/

	}
	DeadProcess dp;
	dp._pid=lPID;
	if(!dp.ExitTime(llit->second.hP))
	{
		//mutex::scoped_lock log_scoped_lock(logger.processes_mutex);
		logger.g_clProcesses.erase(lPID);
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
			logger.g_clProcesses.erase(lPID);
		}					

	/*The process may have started whilst PH wasn't running
	it will have been saved when PH started (See above)*/
	
		//dp.WriteEnd(Creation);	writing everything on process end
		map<long,CProcessInfo>::iterator pi=process_map.find(lPID);
		if(pi!=process_map.end())
		{
			pi->second.SaveProcess(dp._Exit);
		}
		process_map.erase(lPID);
}