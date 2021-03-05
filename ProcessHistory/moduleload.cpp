#include "..\phshared\phshared.h"
#include <mutex>
//#include "boost/algorithm/string.hpp"
#include "..\phlogger\phlogger.h"

using namespace std;
using namespace boost::algorithm;

extern unordered_map<string, sqlite3_int64> PHPaths;
//boost::mutex paths_mtx,cl_mtx;
extern mutex db_mutex;

sqlite3_int64 GetPathID(string path)
{
	//boost::mutex::scoped_lock psl(paths_mtx);	
	//to_lower(path);
	for (auto& c : path)
	{
		c = tolower(c);
	}
	unordered_map<string, sqlite3_int64>::iterator path_it=PHPaths.find(path);
	sqlite3_int64 PathID=-1;

	if(path_it==PHPaths.end())
	{	
		
		sqlite3 * db;
		db=OpenDB();
		{
		//mutex::scoped_lock lock(db_mutex);
			lock_guard<mutex> sl(db_mutex);
		//%Q to remove apostrophes
		char* SQL=sqlite3_mprintf("INSERT INTO Paths(Directory) VALUES(%Q);",path.c_str());
		if(SQLITE_OK!=sqlite3_exec(db,SQL,NULL,NULL,NULL))
		{
			PHTrace(path,__LINE__,__FILE__);
			DBError(sqlite3_errmsg(db),__LINE__,__FILE__);
		}
		sqlite3_free(SQL);
			
		PathID=sqlite3_last_insert_rowid(db);
		}
		if(PathID<1)
			PHTrace("Invalid PathID",__LINE__,__FILE__);
		sqlite3_close(db);

		
		PHPaths.insert(pair<string, sqlite3_int64>(path,PathID));

	}
	else
		PathID=path_it->second;
		

	return PathID;
}

unordered_map<string, sqlite3_int64> PHCLs;

sqlite3_int64 getclid(string cl)
{
	
	//boost::mutex::scoped_lock psl(cl_mtx);
	to_lower(cl);
	unordered_map<string, sqlite3_int64>::iterator path_it=PHCLs.find(cl);
	sqlite3_int64 PathID=-1;

	if (path_it == PHCLs.end())// if not in memory check disk
	{
		string selectSQL = "SELECT ID FROM CommandLines WHERE CommandLine=" + cl + ";";
		sqlite3* db = OpenDB();
		sqlite3_stmt* stmt;
		if (sqlite3_prepare(db, selectSQL.c_str(), -1, &stmt, NULL) != SQLITE_OK)
			DBError(sqlite3_errmsg(db), __LINE__, __FILE__);//TODO whats the select equivalent of %Q
		if (sqlite3_step(stmt) == SQLITE_ROW)
		{
			sqlite3_int64 ID = sqlite3_column_int(stmt, 1);
			PHCLs.insert(pair<string, sqlite3_int64>(cl, ID));
		}
		else//if not on disk persist
		{
			//db=OpenDB();
			//%Q to remove apostrophes
			{
				//mutex::scoped_lock lock(db_mutex);
				lock_guard<mutex> sl(db_mutex);
				char* SQL = sqlite3_mprintf("INSERT INTO CommandLines(CommandLine) VALUES(%Q)", cl.c_str());

				if (SQLITE_OK != sqlite3_exec(db, SQL, 0, 0, 0))
					DBError(sqlite3_errmsg(db), __LINE__, __FILE__);
				else
					PathID = sqlite3_last_insert_rowid(db);
			}

			sqlite3_close(db);

			if (PathID < 1)
				PHTrace("Invalid Command Line ID", __LINE__, __FILE__);

			PHCLs.insert(pair<string, sqlite3_int64>(cl, PathID));
		}
	}
	else
		PathID=path_it->second;		
	
	return PathID;
}



/*void LoadCommandLines(CProgressBarCtrl m_sBar)
{
	sqlite3 * db=OpenDB();
	sqlite3_stmt* stmt;
	if(sqlite3_prepare(db,"SELECT Count(ID) FROM CommandLines;",-1,&stmt,NULL)!=SQLITE_OK)
		DBError(sqlite3_errmsg(db),__LINE__,__FILE__);
	if( sqlite3_step(stmt) == SQLITE_ROW )
		m_sBar.SetRange(0,(int)sqlite3_column_int(stmt,0));
		sqlite3_finalize(stmt);
	if(sqlite3_prepare(db,"SELECT CommandLine,ID FROM CommandLines;",-1,&stmt,NULL)!=SQLITE_OK)
		DBError(sqlite3_errmsg(db),__LINE__,__FILE__);
	while( sqlite3_step(stmt) == SQLITE_ROW )
	{
		//boost::mutex::scoped_lock psl(cl_mtx);
		const unsigned char * cl=sqlite3_column_text(stmt,0);
		long ID=sqlite3_column_int(stmt,1);
		string cls=(char*)cl;
		PHCLs.insert(pair<string,long>(cls,ID));
		m_sBar.SetPos((int)ID);
	}
	m_sBar.SetPos(0);
	sqlite3_finalize(stmt);
	sqlite3_close(db);
}*/