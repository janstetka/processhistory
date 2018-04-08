#ifndef PH_QUERY
#define PH_QUERY

#include "boost/date_time/posix_time/posix_time.hpp"
#include <map>
#include <list>

struct CPHExtent
{
	boost::posix_time::ptime tStart,tEnd;
};

struct PHProcess
{
	boost::posix_time::ptime start,end;
	long lProcessID;
};

class PHQuery
{
private:
	
	std::string _SQL;
	void Layout(std::list<PHProcess> Ordered);
	
public:	
	/*processes*/
	std::map<long,int> _Lines;
	std::map<long,PHProcess> _Processes;
	/*both*/
	std::map<long,long> _PData;
	void clear();
	long _lCount;

	void SetSQL(std::string s){_SQL=s;}
	int Query();
	std::string Construct(	boost::posix_time::ptime Begin);
	std::list<PHProcess> process_list;
	bool long_processes;

};
std::string ModifiedConstruct(boost::posix_time::ptime Begin, boost::posix_time::ptime end);
#endif