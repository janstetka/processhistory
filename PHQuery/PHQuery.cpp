#include <list>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "..\phshared\phshared.h"
//#include <cassert>
#include <sstream>
#include "query.h"
#include "screen.h"
//#include "PH.h"
#include <mutex>

using namespace boost::posix_time;
using namespace std;
using namespace boost;

PHQuery phq;
extern PHDisplay phd;
extern mutex db_mutex;

/*2 dimensional vector that may have rows and columns of differing lengths i.e. ragged
ordered extents by time*/
class ProcessLayout
{
	public:
	ProcessLayout()	{		m_count=0;	}
 
	vector<vector<CPHExtent > >  al ;

	int	m_count ;
	bool Insert(CPHExtent e,int line)
	{
		/*Is the line empty*/
		if(line>al.size())
		{		
			vector<CPHExtent > line_vector;
			line_vector.push_back(e);
			al.push_back(line_vector);
			m_count++;
			return true; 
		}			

		
		/*Is this process later than the last on the line*/
		vector<CPHExtent > & line_vector=al.at(line-1);
		if(line_vector.back().tEnd < e.tStart)
		{
			line_vector.push_back(e);
			m_count++;
			return true;
		}
		
		return false;
		
			
		
	}/* method insert*/
	int Count()
	{
		
		return m_count;
	}
};

	
	/*places all processes to one line at a time
	input: sorted on process start time*/
void PHQuery::Layout(list<PHProcess> clOrdered ) 
{
	ProcessLayout pl;
	
	_lCount = 1;
	for(list<PHProcess>::iterator oi=clOrdered.begin(); oi!=clOrdered.end();oi++)
	{
		CPHExtent clExtent;
		PHProcess php=*oi;
		clExtent.tStart=php.start;
		clExtent.tEnd=php.end;
		int line=1;
		while (!pl.Insert(clExtent, line))
			
			line++;
					/*record which line the process will be drawn on*/
		_Lines.insert(pair<long,int>(php.lProcessID,line));		
				
		if (line > _lCount)
			_lCount = line;
	}
}/*method layout*/

int PHQuery::Query()
{
	string clSQL;
	sqlite3 *db;
	sqlite3_stmt* stmt;
	int count_res=0;
	lock_guard<mutex> sl(db_mutex);
	db = OpenDB();

	
	clSQL="SELECT Process.ID,DATETIME(CreationTime),DATETIME(Destruction) FROM Process";
	
	clSQL+=_SQL;

	clSQL+=" ORDER BY CreationTime;";
	//PHTrace(clSQL,__LINE__,__FILE__);
	if(sqlite3_prepare(db,clSQL.c_str(),-1,&stmt,NULL)!=SQLITE_OK)
		DBError(sqlite3_errmsg(db),__LINE__,__FILE__);
	long  ProcessID;
	const unsigned char * ProcessCreation,*ProcessDestruction;	
	
	
	//clear();
	while(sqlite3_step(stmt)== SQLITE_ROW )
	{	
		/*TODO: tidy up logic here on failure conditions*/
		
		/*Values that will show nulls*/

		ProcessID=-1;
		ProcessCreation=0;
		ProcessDestruction=0;

		ptime clStart,clEnd; 
		ProcessID=sqlite3_column_int(stmt,0);
		ProcessCreation=sqlite3_column_text(stmt,1);
		ProcessDestruction=sqlite3_column_text(stmt,2);
		
		/* check  for null, the events */
		if(ProcessDestruction==0)
			continue;
		if(count_res==0)
			clear();

		count_res++;	
		
		PHProcess pclProcess;
		
		try
		{
				pclProcess.start=time_from_string((const char *)ProcessCreation);
				pclProcess.end=time_from_string((const char *)ProcessDestruction);
		}
		catch(std::exception &)
		{
			
			PHTrace("time_from_string failed",__LINE__,__FILE__);
		}
			pclProcess.lProcessID=ProcessID;
		
		if(ProcessID<1)
			PHTrace("Invalid ProcessID",__LINE__,__FILE__);
		process_list.push_back(pclProcess);
		_Processes.insert(pair<long,PHProcess>(ProcessID,pclProcess));
	}

	sqlite3_finalize(stmt);

	sqlite3_close(db);

	//PHTrace(lexical_cast<string>(process_list.size()),__LINE__,__FILE__);
	if(count_res>0)
		Layout(process_list);
	process_list.clear();
	list<PHProcess>().swap(process_list);
	return count_res;
}

void PHQuery::clear()
{
	/* clear the old collections*/
	_Lines.clear();
	map<long,int>().swap(_Lines);/*deallocate memory*/
	_Processes.clear();
	map<long,PHProcess>().swap(_Processes);
	_PData.clear();
	map<long,long>().swap(_PData);
}
/*select processes that start,end or overlap the times*/
string PHQuery::Construct(ptime Begin)
{
	/*string SQL=" WHERE ";	*/ //refactor
	ptime end=phd.GetEndTime(Begin);
	return ModifiedConstruct(Begin,end);
	/*ostringstream os;
	string begin_txt=BoostToSQLite(Begin),end_txt=BoostToSQLite(end);

	//assert(!begin_txt.empty() || !end_txt.empty());
	os<<"SELECT JULIANDAY('"	<< begin_txt	<<"'),JULIANDAY('"
	<<end_txt	<<"');";

	sqlite3 *db;
	sqlite3_stmt* stmt;
	string ejtxt,bjtxt;
	lock_guard<mutex> sl(db_mutex);
	db = OpenDB();
	if(sqlite3_prepare(db,os.str().c_str(),-1,&stmt,NULL)!=SQLITE_OK)
		DBError(sqlite3_errmsg(db),__LINE__,__FILE__);

	const unsigned char *bjul=0;
	const unsigned char *ejul=0;
	
	if(sqlite3_step(stmt)== SQLITE_ROW )
	{	
		bjul =sqlite3_column_text(stmt,0);		
		ejul =sqlite3_column_text(stmt,1);
	}
	//assert(!bjul==0 || !ejul==0);
	if(bjul==0 || ejul==0)
		PHTrace("Error constructing query",__LINE__,__FILE__);
	ejtxt=(char*)ejul;
	bjtxt=(char*)bjul;
	sqlite3_finalize(stmt);
	sqlite3_close(db);

	os.str("");
	if (phd.filter_exec || phd.filterUserID > 0)
		os << " ( ";*/ //refactor
	/*Starts within times - events too*/
	/*os<<"( CreationTime>"	<<bjtxt	<<" AND CreationTime<"	<<ejtxt*/ //refactor

	/*Ends within times*/
	/*<<") OR ( Destruction<"	<<ejtxt	<<" AND Destruction>"	<<bjtxt
	<<" AND Destruction NOTNULL)";

	if(phq.long_processes)
	{*/ //refactor
		/*Overlaps times*/
	/*	os<<" OR ( CreationTime<"	<<bjtxt	<<" AND Destruction>"	<<ejtxt
		<<" AND Destruction NOTNULL) ";
	}
		if (phd.filter_exec || phd.filterUserID > 0)
			os << " ) ";
		if (phd.filter_exec)
			os << " AND CRC=" << phd.filterCRC << " ";
		if (phd.filterUserID>0)
			os << " AND USERID=" << phd.filterUserID << " ";
	SQL+=os.str();
	return SQL;*/ //refactor
}