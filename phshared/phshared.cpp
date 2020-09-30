#include <windows.h>
#include <sstream>
#include "phshared.h"
#include "..\sqlite\sqlite3.h"
#include "boost/date_time/posix_time/posix_time.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <cassert>

using namespace boost::posix_time;
using namespace std;

void GetModulePath(string & Folder)
{
	size_t iSt;
	string ExecImagePath	;
	TCHAR szFileName[MAX_PATH];
	GetModuleFileName(NULL,szFileName,MAX_PATH);
	ExecImagePath = szFileName;
	iSt=ExecImagePath.find_last_of('\\');
	Folder=ExecImagePath.substr(0,iSt+(size_t)1);
}

// create ph.db in this module's directory
 void GetDBPath(string & Folder)
{	
	GetModulePath(Folder);
	
	//default to ph.db
		Folder.append("ph.db");	
}

void DBError(const char * err,int line,string errfile){	PHTrace(err,line,errfile);}
// keep path.

string dbPath;

sqlite3* OpenDB()
{
	sqlite3 *db;
//	char * szErr;
	if(dbPath.empty())
		GetDBPath(dbPath);
	sqlite3_open(dbPath.c_str(),&db);
	return db;
}

/*Win32 Start*/
 string Win32Error()
{
char *sysMsg;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR) &sysMsg,
		0,
		NULL );

	//char* buf;
	// buf = new char [strlen (sysMsg) + strlen (_msg) + strlen (msgFormat) + 1];
       // wsprintf ( buf, msgFormat, _msg, sysMsg );
	string ret(sysMsg);
	// Free the buffer.
        LocalFree (sysMsg);
	return ret;
}
// Useful helper function for enabling a single privilege
/*BOOL EnableTokenPrivilege(HANDLE htok, LPCTSTR szPrivilege, TOKEN_PRIVILEGES& tpOld)
{
   TOKEN_PRIVILEGES tp;
   tp.PrivilegeCount = 1;
   tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
   if (LookupPrivilegeValue(0, szPrivilege, &tp.Privileges[0].Luid))
   {
   // htok must have been opened with the following permissions:
   // TOKEN_QUERY (to get the old priv setting)
   // TOKEN_ADJUST_PRIVILEGES (to adjust the priv)
      DWORD cbOld = sizeof tpOld;
      if (AdjustTokenPrivileges(htok, FALSE, &tp, cbOld, &tpOld, &cbOld))
      // Note that AdjustTokenPrivileges may succeed, and yet
      // some privileges weren't actually adjusted.
      // You've got to check GetLastError() to be sure!
         return(ERROR_NOT_ALL_ASSIGNED != GetLastError());
      else
         return(FALSE);
   }
   else
      return(FALSE);
}
//--------------------------------------------------------------------
// based on Keith Brown (MSJ August 1999 column)
// 
BOOL EnablePrivilege(LPCTSTR szPrivilege)
{
   BOOL bReturn = FALSE;

   HANDLE hToken;
   if (!::OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken))
      return(FALSE);

   TOKEN_PRIVILEGES tpOld;
   bReturn = (EnableTokenPrivilege(hToken, szPrivilege, tpOld));

// don't forget to close the token handle
   ::CloseHandle(hToken);

   return(bReturn);
}

void GetProcessUser(HANDLE hProcess,string & pu)
{*/
	//EnablePrivilege(SE_TCB_NAME);
	/*Determine the user*/
	/*HANDLE hToken;
	if(0!=OpenProcessToken(hProcess,TOKEN_QUERY,&hToken))
	{
		PTOKEN_USER   ptiUser        = NULL;
		DWORD        cbti     = 0;
		SID_NAME_USE snu; 
		if (GetTokenInformation(hToken, TokenUser, NULL, 0, &cbti)==FALSE) 
		{         
			// Call should have failed due to zero-length buffer.
			if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			{   
				// Allocate buffer for user information in the token.
				ptiUser = (PTOKEN_USER) HeapAlloc(GetProcessHeap(), 0, cbti);
			    // Retrieve the user information from the token.
				if (GetTokenInformation(hToken, TokenUser, ptiUser, cbti, &cbti)==TRUE)
				{					
					DWORD dwUser=0,dwDomain=0;
					LPTSTR User;
					LPTSTR Domain;
				    // Retrieve user name and domain name based on user's SID.
					if (LookupAccountSid(NULL, ptiUser->User.Sid, NULL, &dwUser, 
						NULL, &dwDomain, &snu)==FALSE)
					{
						Domain=(char *)GlobalAlloc(GMEM_FIXED,dwDomain);
						User=(char *)GlobalAlloc(GMEM_FIXED,dwUser);
						
						if(LookupAccountSid(NULL, ptiUser->User.Sid, User, &dwUser, 
						Domain, &dwDomain, &snu)==TRUE)
						{				
							ostringstream os;
							os<<Domain;
							os<<"\\";
							os<<User;
							pu=os.str();
							//CloseHandle(hToken); 							
						}	
						GlobalFree(Domain);
						GlobalFree(User);
					}					
				}
				HeapFree(GetProcessHeap(),0,ptiUser);
			}
		}
		CloseHandle(hToken); 
	}	
	
}*//*Win32 End*/

std::string BoostToSQLite(boost::posix_time::ptime p)
{
	string s;
	try{
		s=to_iso_extended_string(p);
	}
	catch (boost::exception & e)
	{
		PHTrace("to_iso_extended_string failed",__LINE__,__FILE__);
	}
	s.replace(10,1," ");
	return s;
}
/*Win32 end*/



void PHTrace(string err,int line,string errfile)//TODO: only create once or leave as is
{
	string file;
	GetModulePath(file);
	file.append("PHTrace.txt");
	// append to end of file and time

	ofstream out(file.c_str(),ios::ate|ios::app);
	out.seekp(0, ios::end);
	ptime p = second_clock::local_time();
	out<<to_simple_string(p)<<" "<< boost::lexical_cast<string>(line) <<" "<< errfile <<" "<< err<< endl;
}


