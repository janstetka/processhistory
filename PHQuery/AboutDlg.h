#if !defined (ABOUT_H)
#define ABOUT_H

#include <string>
#include "..\phshared\phshared.h"
//#include "..\background\phacker.h"
#include "boost/format.hpp"

std::string ProcessHackerVer;//TODO

class AboutCtrl : public CDialogImpl<AboutCtrl>
{
public:
	enum { IDD = IDD_ABOUT };
	BEGIN_MSG_MAP(AboutCtrl)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		
	END_MSG_MAP()
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
	/*	OSVERSIONINFOEX osvi;
	BOOL bOsVersionInfoEx;

	// Try calling GetVersionEx using the OSVERSIONINFOEX structure.
	// If that fails, try using the OSVERSIONINFO structure.

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
	{
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
		{
			PHTrace("Unable to get OS version",__LINE__,__FILE__);
			//return;
		}
	}
	SYSTEM_INFO si;
	ZeroMemory(&si, sizeof(SYSTEM_INFO));
	GetNativeSystemInfo(&si);*/

	std::ostringstream os;
	/*os << "SQLite : ";
	os << SQLITE_VERSION << "\n"
		<< "Windows Template Library : " << std::hex << _WTL_VER << "\n";
	os << "Boost C++ (datetime,filesystem) : " << BOOST_LIB_VERSION << "\n"  << ProcessHackerVer << "\n";*/
	/*if(osvi.dwMajorVersion==5 && osvi.dwMinorVersion==1)
	{
		os<<"Windows XP";
		if(si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_INTEL)
			os<<" 32 bit CPU";
	}
	if(osvi.dwMajorVersion==6 && osvi.dwMinorVersion==1 )
	{
		os<<"Windows 7";*/
		
	/*	if(si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
			os<<" 64 bit CPU";
	}*/
		/*os<< "Built : " << __DATE__<<" "<<__TIME__;
		os << "\nMicrosoft Visual C++ : " <<std::dec<< _MSC_VER;
#if defined(_WIN64)
		os<<" 64bit ";
#else
		os<<" 32bit";
		#endif*/
		std::string file;
	GetModulePath(file);
	file.append("PHTrace.txt");
	os<<"\nError logging to: "<<file;
	os<< boost::format("SQLite : %s\nWindows Template Library : %h\nBoost C++ (datetime) : %s\nBuilt : %s\nMicrosoft Visual C++ : %d\nError logging to: %s") % SQLITE_VERSION % _WTL_VER % BOOST_LIB_VERSION % (std::string(__DATE__) + " " +std::string( __TIME__)) % _MSC_VER % file;
	SetDlgItemText(IDC_ABOUT_TEXT,os.str().c_str());
		return TRUE;
	}
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
	EndDialog(wID);
	return 0;
	}	
	
};
#endif
