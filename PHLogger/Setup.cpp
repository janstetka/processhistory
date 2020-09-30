#include "PHLogger.h"
#include "PHLogUser.h"
#include "..\phshared\phshared.h"
#include <thread>

using namespace std;

map<string, sqlite3_int64> PHPaths;

//void LoadPathData();

void CPHLogger::StartProcessHistory()
{	
	CreateTables();
	ReadUserTable();
	thread refresh(	RefreshThread);
	refresh.detach();
}

#include "..\ProcessHistory\Progress.h"
void LoadPathData(CProgressBarCtrl m_sBar)
{
	sqlite3 * db=OpenDB();
	sqlite3_stmt* stmt;
	if(sqlite3_prepare(db,"SELECT Count(ID) FROM Paths;",-1,&stmt,NULL)!=SQLITE_OK)
		DBError(sqlite3_errmsg(db),__LINE__,__FILE__);
	if( sqlite3_step(stmt) == SQLITE_ROW )
	m_sBar.SetRange(0,(int)sqlite3_column_int(stmt,0));
	sqlite3_finalize(stmt);
	if(sqlite3_prepare(db,"SELECT Directory,ID FROM Paths;",-1,&stmt,NULL)!=SQLITE_OK)
		DBError(sqlite3_errmsg(db),__LINE__,__FILE__);
	while( sqlite3_step(stmt) == SQLITE_ROW )
	{
		const unsigned char* Path=sqlite3_column_text(stmt,0);
		sqlite3_int64 ID=sqlite3_column_int(stmt,1);
		string p=(char*)Path;
		
		//boost::mutex::scoped_lock psl(paths_mtx);
		PHPaths.insert(pair<string, sqlite3_int64>(p,ID));
		m_sBar.SetPos((int)ID);
	}	
	m_sBar.SetPos(0);
	sqlite3_finalize(stmt);
	sqlite3_close(db);
}