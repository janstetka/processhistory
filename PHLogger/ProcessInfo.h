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
	CProcessInfo(long lPId);
	void GetExecutableImage();	
	void SaveProcess(boost::posix_time::ptime);
	bool Validate();
	std::string GetStartTime()	{	return BoostToSQLite(_StartTime);	}
	std::string GetUser()	{	return _User;	}
	boost::posix_time::ptime GetStart()	{	return _StartTime;	}
	void StartEvent();
	/*Windows PID*/
	long _PId,_ProcessID;	
	void ConvertStartTime();
	void LoadProcessData();
	long _UserID;
	std::string _path;
	std::string _commandline;
	bool _bUserFail;
private:	
	
	/*Database Id. The map stores the windows PID*/

	boost::posix_time::ptime _StartTime;
	bool _bValid ;
	std::string _User;
	FILETIME _WIN32Time;
	//unsigned long _CRC;
};

#endif