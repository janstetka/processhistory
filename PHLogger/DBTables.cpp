#include "..\phshared\phshared.h"
#if defined (_WIN64)
#include <mutex>
#else
#include "boost\thread\mutex.hpp"
#include <boost\thread\lock_guard.hpp> 
using namespace boost;
#endif


using namespace std;

extern mutex db_mutex;

void CreateTables()
{
	sqlite3 *db;
	string SQL;
	char * szError;
	lock_guard<mutex> sl(db_mutex);
	
	db = OpenDB();

	sqlite3_exec(db,"PRAGMA auto_vacuum=on;",NULL,NULL,&szError);
	sqlite3_exec(db,"BEGIN;",NULL,NULL,&szError);
	SQL="CREATE TABLE  Process(ID INTEGER PRIMARY KEY ,CreationTime REAL,Destruction REAL,PathID INTEGER,CLID INTEGER,UserID INTEGER,CRC INTEGER );";
	sqlite3_exec(db,SQL.c_str(),NULL,NULL,&szError);
	sqlite3_exec(db,"CREATE TABLE  Paths(ID INTEGER PRIMARY KEY,Directory varchar(300) UNIQUE);",NULL,NULL,&szError);
	sqlite3_exec(db,"CREATE TABLE  CommandLines(ID INTEGER PRIMARY KEY,CommandLine VARCHAR(255));",NULL,NULL,&szError);
	sqlite3_exec(db,"CREATE TABLE PHLogUser(ID INTEGER PRIMARY KEY,UserName varchar(50) UNIQUE);",NULL,NULL,&szError);
	sqlite3_exec(db,"COMMIT;",NULL,NULL,&szError);
	sqlite3_close(db);
}