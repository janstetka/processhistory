#ifndef PHSHAREH
#define PHSHAREH

#include <windows.h>//win32
#include "..\sqlite\sqlite3.h"
#include <string>
#include "boost/date_time/posix_time/posix_time.hpp"

//void GetExecutableName(long lPID, std::string & ExecPath);
sqlite3* OpenDB();
void DBError(const char * err,int line,std::string file);
std::string Win32Error();
void GetProcessUser(HANDLE hProcess,std::string &);//win32
void GetModulePath(std::string &);
std::string BoostToSQLite(boost::posix_time::ptime p);
BOOL EnablePrivilege(LPCTSTR szPrivilege);
void PHTrace(std::string err,int line,std::string errfile);
//void PHCOMError(HRESULT hr,std::string s,int line, std::string file);


#endif