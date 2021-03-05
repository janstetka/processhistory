#include <string>
#include <unordered_map>
#include "..\phshared\phshared.h"
#include "PHLogUser.h"
//#include <boost/thread/mutex.hpp>

using namespace std;
//using namespace boost;

unordered_map<string, sqlite3_int64> g_clUser;
//extern mutex db_mutex;
//mutex user_mtx;

/*Get the database ID of a windows user*/
sqlite3_int64 GetUserID(string clUserName)
{
	//mutex::scoped_lock sl(user_mtx);
	unordered_map<string, sqlite3_int64>::iterator it;
	it=g_clUser.find(clUserName);
	if (it == g_clUser.end())
		 return CreateUser(clUserName);
	else
		return it->second;
}

/*Read the values in the PHLogUser table */

bool ReadUserTable()
{
	//mutex::scoped_lock sl(user_mtx);
	sqlite3_stmt* stmt;

	char *clSQL="SELECT ID,UserName FROM PHLogUser;";
	sqlite3_int64 lUserID=-1;

	sqlite3 * db=OpenDB();
	if(SQLITE_OK!=sqlite3_prepare(db,clSQL,-1,&stmt,NULL))
		DBError(sqlite3_errmsg(db),__LINE__,__FILE__);

	

	while( sqlite3_step(stmt)== SQLITE_ROW )	
	{
		lUserID=sqlite3_column_int(stmt,0);
		const unsigned char* szuser=sqlite3_column_text(stmt,1);
		string User;
		if(szuser!=0)
		{
			
			User=(char *)szuser;
			g_clUser.insert(pair<string, sqlite3_int64>(User,lUserID));
			//cout<< "loaded user: "<<User<< endl;
		}
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);

	if(lUserID==-1)
		return false;	
	
	return true;
}

sqlite3_int64 CreateUser(string User)
{
	//mutex::scoped_lock lock(db_mutex);
	//mutex::scoped_lock sl(user_mtx);
	sqlite3_int64 lUserID;
	string clSQL;

	
	sqlite3 *db=OpenDB();
	clSQL="INSERT INTO PHLogUser (UserName) VALUES('";

	clSQL.append(User);
	clSQL.append("');");
	char * Err;
	if(SQLITE_OK!=sqlite3_exec(db,clSQL.c_str(),NULL,NULL,&Err))
		DBError(sqlite3_errmsg(db),__LINE__,__FILE__);

	lUserID=sqlite3_last_insert_rowid(db);
	g_clUser.insert(pair<string, sqlite3_int64>(User,lUserID));
	sqlite3_close(db);

	return lUserID;
}