#ifndef PHLOGH
#define PHLOGH

#include <map>
#include "boost/date_time/posix_time/posix_time.hpp"
#include <string>
#include "..\phshared\phshared.h"
//#include <boost/thread/mutex.hpp>

struct PHProcessData
{
	boost::posix_time::ptime time;
	//
	long ProcessID;
	HANDLE hP;
};

/*Only one instance of this class is allowed, singleton*/
class CPHLogger 
{

public:		

	void StartProcessHistory();
	void StartEvent( long lPId);
	void StopEvent(long lPID);

	/*The processes that are being logged*/

	std::map<long,PHProcessData> g_clProcesses;

	//boost::mutex processes_mutex;
	//void InitialProcessStart(long id);
	//bool WinXP;
	//bool Win7;
	int _Refresh;
	sqlite3 * procdb;
};

void CreateTables();
void StartEv(long p);
long GetPathID(std::string path);
void StartBackground();
void StopBackground();
void PHMessage(std::string);
//void LoadCommandLines();
//void GetAllProcesses();
void RefreshThread();

#endif