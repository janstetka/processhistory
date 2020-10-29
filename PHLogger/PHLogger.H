#ifndef PHLOGH
#define PHLOGH

#include <map>
#include "boost/date_time/posix_time/posix_time.hpp"
#include <string>
#include "..\phshared\phshared.h"
#include <windows.h>
//#include <boost/thread/mutex.hpp>

struct PHProcessData
{
	boost::posix_time::ptime time;
	//
	sqlite3_int64 ProcessID;
	HANDLE hP;
};

struct qi
{
HANDLE ID;
SYSTEMTIME st;
HANDLE parentID;
boost::posix_time::ptime pt;
bool operator<(qi const &other) const { return pt< other.pt;}
};

/*Only one instance of this class is allowed, singleton*/
class CPHLogger 
{
public:
	~CPHLogger()
	{
		sqlite3_close(procdb);
	}
		
		

	void StartProcessHistory();
	void StartEvent( qi P);
	void StopEvent(HANDLE lPID);

	/*The processes that are being logged*/

	std::map<HANDLE,PHProcessData> g_clProcesses;

	//boost::mutex processes_mutex;
	
	//bool WinXP;
	//bool Win7;
	int _Refresh;
	sqlite3 * procdb;
};

void CreateTables();
void StartEv(long p);
sqlite3_int64 GetPathID(std::string path);
void StartBackground();
void StopBackground();
void PHMessage(std::string);
//void LoadCommandLines();
//void GetAllProcesses();
void RefreshThread();

#endif