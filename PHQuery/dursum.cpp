#include "..\phshared\phshared.h"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "..\ProcessHistory\PHScroll.h"
//#include <mutex>

using namespace boost::posix_time;
using namespace std;
using namespace boost;
using namespace boost::gregorian;

//extern mutex db_mutex;

string PHTotals(unsigned long CRC)
{
	//lock_guard<mutex> sl(db_mutex);
	sqlite3 *db;
	sqlite3_stmt* stmt;
	time_duration td;

	db = OpenDB();	
	
	ostringstream SQL;
	SQL<<"SELECT DATETIME(Destruction),DATETIME(CreationTime) from Process WHERE Destruction NOTNULL AND CRC="<<CRC;
	SQL<<";";
	
	if(sqlite3_prepare(db,SQL.str().c_str(),-1,&stmt,NULL)!=SQLITE_OK)
		DBError(sqlite3_errmsg(db),__LINE__,__FILE__);
	
	const unsigned char * ProcessCreation,*ProcessDestruction;	
	ptime start,end;

	while(sqlite3_step(stmt)== SQLITE_ROW )
	{
		ProcessCreation=sqlite3_column_text(stmt,1);
		ProcessDestruction=sqlite3_column_text(stmt,0);
		start=time_from_string((const char *)ProcessCreation);
		end=time_from_string((const char *)ProcessDestruction);
		td+=(end-start);
	}
	date_duration dd;
	if(td.hours()>24)
	{
		float fd=(float)td.hours()/24;
		dd=date_duration((int)fd);
		td-=hours(dd.days()*24);
	}
	sqlite3_finalize(stmt);
	sqlite3_close(db);

	string durtxt;
	if(dd.days()>0)
		durtxt=lexical_cast<string>(dd.days())+" days ";
	durtxt+=to_simple_string(td);
	return durtxt;			
}

