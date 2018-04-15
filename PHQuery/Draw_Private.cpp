#include "boost/date_time/posix_time/posix_time.hpp"
#include "..\phshared\phshared.h"
#include "screen.h"
#include "query.h"
#include <sstream>

using namespace boost::posix_time;
using namespace std;
using namespace boost;

extern PHQuery phq;
extern PHDisplay phd;

/*Draws a time axis.
the DC to draw onto
Left - the time at the lh end of the page
*/


RECT  PHDisplay::CalculateRect(ptime Start,ptime End ,ptime Left,int iLine)
{
	if (End - Start < seconds(1))
	{
		End += seconds(1);
		Start -= seconds(1);
	}

	time_duration clEventTime=Start-Left;
		/*Calculate the total minutes*/
		long lxStartPos=
			(clEventTime.hours()*3600)+(clEventTime.minutes()*60) 
			+ (clEventTime.seconds());

		long lxEndPos;

			clEventTime=End-Left;
			lxEndPos=(clEventTime.hours()*3600)+(clEventTime.minutes()*60)
				+ (clEventTime.seconds());

		RECT rcEvent;
		rcEvent.left=lxStartPos;
		rcEvent.right=lxEndPos;
		rcEvent.top=(iLine*41)-40;
		rcEvent.bottom=iLine*41;
		
		if (rcEvent.left == rcEvent.right)
		{
			rcEvent.left--;
			rcEvent.right++;
		}

		return rcEvent;
}

