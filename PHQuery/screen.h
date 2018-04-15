#ifndef PH_SCREEN
#define PH_SCREEN

/*#include <atlbase.h>
#include <atlapp.h>
#include <atlgdi.h>*/
#include <set>
#include <map>
#include <string>
#include "boost/date_time/posix_time/posix_time.hpp"
#include <windows.h>

class PHDisplay
{	
	public:
	PHDisplay() 
	{
		_selected=0;

		filter_exec=false;
		_mouseover = -1;

		NONCLIENTMETRICS ncm;
		ncm.cbSize = sizeof(NONCLIENTMETRICS);
		::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);

		_font=ncm.lfMenuFont;
		//scale=1000;
		
	}
	boost::posix_time::ptime _WinLeft, _WinRight;

	/*Display fields*/
	

	/*a list of whats on screen now is kept in ProcessAreas */
	std::map<long,RECT> _ProcessAreas;
	std::map<long,HICON> icons;//lifetime of PH

	bool _complete;
	LOGFONT _font;
	long _mouseover;
	
	private:
	
	RECT CalculateRect(boost::posix_time::ptime Start,boost::posix_time::ptime End ,boost::posix_time::ptime Left,int iLine);		
	
	public:
	
	std::set<long> _clipped_left,_clipped_right;
	std::map<long,std::string> _EXEImages;//version info, lifetime of PH
	
	std::map<long,std::string> qrypaths;

	/*Display methods*/
	void ReadPaths();
	
	void UpdateWindow();
	int _Width;

	boost::posix_time::ptime GetEndTime(boost::posix_time::ptime Left);
	
	unsigned long _selected;

	bool	filter_exec;

	int filterUserID;
	unsigned long filterCRC;
	bool _process_detail;
	//float scale; //ms represented by a pixel
};

void GetVersionInfo(std::string & Product, std::string & Description, std::string path);
void LeftRight(WPARAM wParam);
#endif