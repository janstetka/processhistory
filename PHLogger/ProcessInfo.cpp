#include "phlogger.h"
#include "ProcessInfo.h"
#include "PHLogUser.h"
#include <string>
#include <sstream>
#include "..\phshared\phshared.h"
#include "boost/date_time/local_time_adjustor.hpp"
#include "boost/date_time/c_local_time_adjustor.hpp"
#include "boost/algorithm/string.hpp"
#include <set>
#include "..\phshared\Crc32Static.h"
#include "..\background\phacker.h"
#include "nowide\convert.hpp"
#include <mutex>

using namespace boost::posix_time;
using namespace boost::gregorian;
using namespace std;
using namespace boost;

extern CPHLogger logger;
//extern map<string,long> PHPaths;

 extern mutex db_mutex;

/*Find out the user name and start time*/
CProcessInfo::CProcessInfo(long lPId)
{
	//_is_already_logged=false;
	_PId=lPId;
	_bValid=true;
	_UserID=-1;
	
	/*Win32 Start*/
	FILETIME ftCreation,ftExit,ftKernel,ftUser;

	HANDLE hProcess = OpenProcess(
			PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
			FALSE, lPId);
	
	if(hProcess==NULL)
	{
		//PHMessage("Unable to open process handle "+ lexical_cast<string>(lPId));
		_bValid=false;
		return;
	}
	/*Retreive the process start time*/
	BOOL bRet = GetProcessTimes(hProcess,&ftCreation,&ftExit,&ftKernel,&ftUser);
	GetProcessUser(hProcess,_User);	
	
	PWSTR pCL = PHiGetCommandLine(hProcess);
	if (pCL != 0)
	{
		_commandline = nowide::narrow(pCL);
		free(pCL);
	}

	CloseHandle(hProcess);	


	if(bRet>0)
	{
		_WIN32Time=ftCreation;
	}
	else
	{
		Win32Error();//
		//PHMessage("Unable to get Process Times");
		_bValid=false;
		return;
	}
	
	_UserID=GetUserID(_User);
}

void CProcessInfo::ConvertStartTime()
{
	_StartTime=from_ftime<ptime>(_WIN32Time);
	/*store as local time*/
	_StartTime=date_time::c_local_adjustor<ptime>::utc_to_local(_StartTime);
}

/*Are the time, PID, user valid*/
bool CProcessInfo::Validate()	{	return _bValid;	}

/*The path to this  processes' module*/
void CProcessInfo::GetExecutableImage()
{
	int i=0;
	string path;
	GetExecutableName(_PId,path);		/*END: TIME CRITICAL*/
	
	if(path.empty())
	{
		//PHMessage("Failed processinfo get executable image "+ lexical_cast<string>(_PId));
		_bValid=false;
		return;
	}
	else
	{
		replace_first(path,"\\??\\","");
			if(find_first(path,"SystemRoot"))
			{
				char*sysroot=getenv("SYSTEMROOT");
				replace_first(path,"\\SystemRoot",sysroot);
			}
		_path=path;
		//cout<<"path: "<<_path<<endl;
	}
		
	DWORD dwCRC;
#if defined (_WIN64)
	CCrc32Static::FileCrc32Win32(_path.c_str(),dwCRC);
#else
	CCrc32Static::FileCrc32Assembly(_path.c_str(),dwCRC);
#endif
		_CRC=dwCRC;
	}

	
extern long getclid(string cl);

/*Log the process starting*/
void CProcessInfo::SaveProcess(ptime ExitTime)
{	
	ostringstream clSQL;
	clSQL.str("");
	
	long PathID=GetPathID(_path);
	if(PathID==-1)
		return;

	clSQL<<"INSERT INTO Process(CreationTime,PathID,CLID,UserID,CRC,Destruction) VALUES (JULIANDAY('" 	<< GetStartTime()
	<<"'),"	<<PathID;
	
	long CLID=getclid(_commandline);
	//if(CLID==-1)
	//	return;
	clSQL<<","<<CLID;
	clSQL<<","<<_UserID;
	clSQL<<","<<_CRC;
	clSQL<<","<<"JULIANDAY('"	<<	BoostToSQLite(ExitTime);
	clSQL<<"'));";
	
	
	//sqlite3 * db;
	//db=OpenDB();
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
	
	//sqlite3_close(db);
	
	return;
}

