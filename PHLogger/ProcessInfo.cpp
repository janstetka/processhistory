#include "phlogger.h"
#include "ProcessInfo.h"
#include "PHLogUser.h"
#include <string>
#include <sstream>
#include "..\phshared\phshared.h"
#include "boost/format.hpp"
#include "..\background\phacker.h"
#include "nowide\convert.hpp"
#include <mutex>

using namespace boost::posix_time;
using namespace boost::gregorian;
using namespace std;
using namespace boost::date_time;


extern CPHLogger logger;
//extern map<string,long> PHPaths;

 extern mutex db_mutex;

/*Find out the user name and start time*/
CProcessInfo::CProcessInfo(HANDLE lPId, SYSTEMTIME ftCreation)
{
	//_is_already_logged=false;
	_PId=lPId;
	_bValid=true;
	_UserID=-1;	
	// Image and times at least require PROCESS_QUERY_LIMITED_INFORMATION
	
	HANDLE hProcess=NULL;

		 PHackerOpenProcess(lPId,
			PROCESS_QUERY_LIMITED_INFORMATION,&hProcess)  ;

	
		//if (hProcess != NULL)//ELEV:phacker gets start time from ntquery
		//{
			/*Win32 Start*/

			/*Retreive the process start time*/
			

				_WIN32Time = ftCreation;

		try{
				_StartTime = ptime(boost::gregorian::date( _WIN32Time.wYear,_WIN32Time.wMonth, _WIN32Time.wDay),hours( _WIN32Time.wHour)+minutes( _WIN32Time.wMinute)+seconds( _WIN32Time.wSecond)+milliseconds( _WIN32Time.wMilliseconds));
		}
		catch (...)
		{
			PHTrace("exception in exit time conversion", __LINE__, __FILE__);
			CloseHandle(hProcess);
			_bValid = false;
return;
		}

			string path;//ELEV phacker uses id to get image unelevated
			PWSTR pCL = PHackGetImageFile(_PId, hProcess);

			if (pCL != 0)
			{
				path = nowide::narrow(pCL);
				free(pCL);
			}		
			else
			{
				_bValid = false;
				return;
			}
			if (!path.empty())
				_path = path;
			else
			{
				_bValid = false;
				return;
			}
			if (hProcess != NULL)
				CloseHandle(hProcess);

	HANDLE elhProcess = NULL;
		PHackerOpenProcess(lPId,
			PROCESS_QUERY_INFORMATION,&elhProcess );
	//user may require elevation
	if(elhProcess!=NULL)
	{
		PWSTR pCL = PHackerGetUser(elhProcess,lPId);
		if (pCL != 0)
		{
			_User = nowide::narrow(pCL);
			free(pCL);
		}
		_UserID=GetUserID(_User);
		CloseHandle(elhProcess);
	}
	//command line may require elevation	
	HANDLE hCmdProcess = NULL;
	  PHackerOpenProcess(lPId,
		PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,&hCmdProcess) ;
	if(hProcess!=NULL)
	{
		PWSTR pCL = PHiGetCommandLine(hCmdProcess);
		if (pCL != 0)
		{
			_commandline = nowide::narrow(pCL);
			free(pCL);
		}
		CloseHandle(hCmdProcess);
	}	
}

//void CProcessInfo::ConvertStartTime()
//{
//	_StartTime=from_ftime<ptime>(_WIN32Time);
	/*store as local time*/
//	_StartTime=c_local_adjustor<ptime>::utc_to_local(_StartTime);
//}

/*Are the time, PID, user valid*/
bool CProcessInfo::Validate()	{	return _bValid;	}

extern sqlite3_int64 getclid(string cl);

extern map<HANDLE, CProcessInfo> process_map;

 sqlite3_int64 CProcessInfo::GetParentID() // will only work if parent process has ended
{
	 //record what details we have about the parent, i.e. start time etc. in case can be linked when query is run
	 // otherwise can display these details at query

	 // will have gathered these details in constructor - need to know which member of map to lookup
	 map<HANDLE, CProcessInfo>::iterator it=process_map.find(ParentPID);
	 if (it != process_map.end())
	 {
		 // lookup whether have already saved
		 it->second.SaveProcess(ptime(),true); // put more checking in below to only save info on process that is available i.e. won't have exit time in this case
	 }

	 // goal is right click go to parent (use DBID)
	 // or to display gather all as string
	 // * add to process table if not already there and refer to that, would need to check (below) before adding  processes that havn't already been partially recorded as a parent!!!!
	 //cases
	 //parent already finished
	 //parent running
	 return -1;
}
/*Log the process starting*/
 void CProcessInfo::SaveProcess(ptime ExitTime, bool parent)
 {	 
	 sqlite3_int64 PathID = GetPathID(_path);
	 if (PathID == -1)
		 return;
	 ostringstream clSQL;
	 clSQL.str("");
	 clSQL << boost::format("INSERT INTO Process(CreationTime,PathID,CLID,UserID,Destruction) VALUES (JULIANDAY('%s'),%d,%d,%d,JULIANDAY('%s'));") % GetStartTime() % PathID % getclid(_commandline) % _UserID % BoostToSQLite(ExitTime) ;
	 //,ParentID,%d% GetParentID()
	 


	 /*clSQL << "INSERT INTO Process(CreationTime,PathID,CLID,UserID,";
		 if (!parent)
			 clSQL << "Destruction";
		 
		clSQL<< ",ParentID) VALUES (JULIANDAY('" << GetStartTime()
		 << "')," << PathID;

	 sqlite3_int64 CLID = getclid(_commandline);
	 //if(CLID==-1)
	 //	return;
	 clSQL << "," << CLID;
	 clSQL << "," << _UserID;
	 if (!parent)
	 {
	 
	 clSQL << "," << "JULIANDAY('" << BoostToSQLite(ExitTime);
	 clSQL << "'),";
 }
	clSQL<<GetParentID();
	clSQL<<");";*/
	
	char * szErr;
	{
		lock_guard<mutex> sl(db_mutex);
	//mutex::scoped_lock lock(db_mutex);
	if(SQLITE_OK!=sqlite3_exec(logger.procdb,clSQL.str().c_str(),NULL,NULL,&szErr))
		DBError(szErr,__LINE__,__FILE__);
	else 
		_ProcessID = sqlite3_last_insert_rowid(logger.procdb);
	}
	if(_ProcessID<1)
		PHTrace("ProcessID less than 1",__LINE__,__FILE__);			
	
	return;
}

