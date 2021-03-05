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

extern "C" sqlite3* OpenDB()
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
	out<<to_simple_string(p)<<" "<< to_string(line) <<" "<< errfile <<" "<< err<< endl;
}


