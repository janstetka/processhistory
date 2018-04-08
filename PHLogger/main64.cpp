#define _WIN32_DCOM

#include "PHLogger.h"
//#include "COM_WMI_Consumer\COM_WMI_Consumer.h"
#include "..\phshared\phshared.h"
#include "ProcessInfo.h"
#include "..\background\PHacker.h"

using namespace std;

CPHLogger logger;

void StartEv(long p){	logger.StartEvent(p);}
void StopEv(long p){	logger.StopEvent(p);}

void StartBackground()
{
	OSVERSIONINFOEX osvi;
	BOOL bOsVersionInfoEx;

	// Try calling GetVersionEx using the OSVERSIONINFOEX structure.
	// If that fails, try using the OSVERSIONINFO structure.

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
	{
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
		{
			PHTrace("Unable to get OS version",__LINE__,__FILE__);
			return;
		}
	}
	
	SYSTEM_INFO si;
	ZeroMemory(&si, sizeof(SYSTEM_INFO));
	GetNativeSystemInfo(&si);

	if(osvi.dwMajorVersion==5 && osvi.dwMinorVersion==1)
	{
		cout<<"Windows XP"<<endl;
		if(si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_INTEL)
			cout<<" 32 bit CPU"<<endl;
	}
	if(osvi.dwMajorVersion==6 && osvi.dwMinorVersion==1 )
	{
		cout<<"Windows 7"<<endl;
		#if defined(_WIN64)
		cout<<"Compiled as 64bit"<<endl;
		#endif
		if(si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
			cout<<" 64 bit CPU"<<endl;
	}

	/*HRESULT hres=CoInitializeEx(NULL,COINIT_MULTITHREADED);
	if (FAILED(hres))
	{
     cout<<"Failed to initialized COM"<<endl;
	 return;
	}
	 hres =  CoInitializeSecurity(
        NULL, 
        -1,                          // COM negotiates service
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities 
        NULL                         // Reserved
        );
	 if (FAILED(hres))
    {
		PHCOMError(hres,"Failed to initialize security.",__LINE__,__FILE__);
		CoUninitialize();
		return;
	 }*/

	 ProcessHackerStart();
		string file;
	GetModulePath(file);
	file.append("PHTrace.txt");
	cout<<"Error logging to: "<<file<<endl;
	logger.StartProcessHistory();
	//Start(StartEv,StopEv);
}

void StopBackground()
	/*Stop the logging gracefully. PH Logger can then be restarted during this instance
of windows*/
	{
		//ReleaseCOM();
	//Stop();  
}

//boost::mutex phm_mutex;

void PHMessage(string s)
{
//	boost::mutex::scoped_lock phmsl(phm_mutex);
	cout<<s<<endl;
}

#include <tchar.h>

int _tmain(int argc, _TCHAR* argv[])
{
	StartBackground();
	cin.get();
	StopBackground();
	return 0;
}
