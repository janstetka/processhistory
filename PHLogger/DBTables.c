//#include "..\phshared\phshared.h"
//#include <mutex>
#include "..\sqlite\sqlite3.h"
//using namespace std;
#include "library.h"

//TODOmake this c
extern sqlite3* OpenDB();
void CreateTables()
{
	sqlite3 *db;
	//string SQL;
	char * szError;
	//lock_guard<mutex> sl(db_mutex);
	
	db = OpenDB();

	sqlite3_exec(db,"PRAGMA auto_vacuum=on;",NULL,NULL,&szError);
	sqlite3_exec(db,"BEGIN;",NULL,NULL,&szError);
	//SQL="CREATE TABLE  Process(ID INTEGER PRIMARY KEY ,CreationTime REAL,Destruction REAL,PathID INTEGER,CLID INTEGER,UserID INTEGER, ParentID INTEGER );";
	sqlite3_exec(db,"CREATE TABLE  Process(ID INTEGER PRIMARY KEY ,CreationTime REAL,Destruction REAL,PathID INTEGER,CLID INTEGER,UserID INTEGER, ParentID INTEGER );",NULL,NULL,&szError);
	sqlite3_exec(db,"CREATE TABLE  Paths(ID INTEGER PRIMARY KEY,Directory varchar(300) UNIQUE);",NULL,NULL,&szError);
	sqlite3_exec(db,"CREATE TABLE  CommandLines(ID INTEGER PRIMARY KEY,CommandLine VARCHAR(255));",NULL,NULL,&szError);
	sqlite3_exec(db,"CREATE TABLE PHLogUser(ID INTEGER PRIMARY KEY,UserName varchar(50) UNIQUE);",NULL,NULL,&szError);
	sqlite3_exec(db,"COMMIT;",NULL,NULL,&szError);
	sqlite3_close(db);
}