#ifndef PHPIH
#define PHPIH

#include "boost/date_time/posix_time/posix_time.hpp"
#include <windows.h>
#include "..\phshared\phshared.h"

class CProcessInfo
{
public:
	/*Default constructor for variable declaration*/
	CProcessInfo(){}
	/*Constructor that needs to be called to populate the class*/
	CProcessInfo(HANDLE lPId, SYSTEMTIME ftCreation);
	void GetExecutableImage();	
	void SaveProcess(boost::posix_time::ptime, bool);
	bool Validate();
	std::string GetStartTime()	{	return BoostToSQLite(_StartTime);	}
	std::string GetUser()	{	return _User;	}
	boost::posix_time::ptime GetStart()	{	return _StartTime;	}
	void StartEvent();
	sqlite3_int64 GetParentID();
	/*Windows PID*/
	HANDLE _PId,ParentPID;
	sqlite3_int64 _ProcessID;
	void ConvertStartTime();
	void LoadProcessData();
	sqlite_int64 _UserID;
	std::string _path;
	std::string _commandline;
	bool _bUserFail;
private:	
	
	/*Database Id. The map stores the windows PID*/

	boost::posix_time::ptime _StartTime;
	bool _bValid ;
	std::string _User;
	SYSTEMTIME _WIN32Time;
};

#endif