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
			(clEventTime.hours()*3600000/phd.scale)+(clEventTime.minutes()*60000/phd.scale) 
			+ (clEventTime.seconds()*1000/phd.scale);

		long lxEndPos;

			clEventTime=End-Left;
			lxEndPos=(clEventTime.hours()*3600000/phd.scale)+(clEventTime.minutes()*60000/phd.scale)
				+ (clEventTime.seconds()*1000/phd.scale);

			//if (clEventTime<seconds(1))

			//scale 3000 would get 60 minutes in 1200 pixels - limit scale between 1 and 1000?
			//scale 1000 would be 1 sec / pixel
				//scale 50 would get 60 seconds in 1200 pixels
				//scale 1 = ms per pixel
				//scale 0.8 would get 1 second in 1250 pixels

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

