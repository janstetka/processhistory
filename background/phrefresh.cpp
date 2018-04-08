#include <map>
#include <set>
#include "..\phlogger\ProcessInfo.h"
#include <tlhelp32.h>

using namespace std;

#include "..\phlogger\PHLogger.h"
#include <algorithm>
#include "..\phquery\PH.h"
#include "..\ProcessHistory\Progress.h"

extern CPHLogger logger;
extern map<long,CProcessInfo> process_map;
extern map< string,long> PHCLs;
extern map< string,long> PHPaths;
extern PH ph_instance;

void RefreshThread()
{
	CProgressBarCtrl m_sBar;
	m_sBar.Attach(ph_instance._hWndProgress);
LoadPathData(m_sBar);
	LoadCommandLines(m_sBar);

	set<long> OldProcesses;
	set<long> ignore;
	while(true)
	{
		Sleep(logger._Refresh);
		
		HANDLE         hProcessSnap = NULL; 
		PROCESSENTRY32 pe32      = {0}; 
 
		/*  Take a snapshot of all processes in the system. */

		hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 

		if (hProcessSnap == INVALID_HANDLE_VALUE) 
			return ;
 
		/*  Fill in the size of the structure before using it. */

		pe32.dwSize = sizeof(PROCESSENTRY32); 
 
		/*  Walk the snapshot of the processes, and for each process, 
		display information. */

		if (Process32First(hProcessSnap, &pe32)) 
		{         
			set<long> Processes;
			while (Process32Next(hProcessSnap, &pe32))
			{
				Processes.insert(pe32.th32ProcessID);
				if (process_map.find(pe32.th32ProcessID) == process_map.end())//if not already got this one - log
						logger.StartEvent(pe32.th32ProcessID);
			}
			
			//result present in first but not second
			set<long> result;
			set_difference(OldProcesses.begin(), OldProcesses.end(), Processes.begin(), Processes.end(),inserter(result, result.end()));
			set<long>::iterator it;
			for (it = result.begin(); it != result.end(); it++)
			{
				logger.StopEvent(*it);
				ignore.erase(*it);
			}
			OldProcesses=Processes;
		} 
 
		/* Do not forget to clean up the snapshot object. */

		CloseHandle (hProcessSnap); 
		//cout << " g_clProcesses" << logger.g_clProcesses.size() << " OldProcesses" << OldProcesses.size() << "Process map" << process_map.size() << "PHPaths" << PHPaths.size() <<"PHCLs"<< PHCLs.size()<<endl;
	}
}