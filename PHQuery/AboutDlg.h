#if !defined (ABOUT_H)
#define ABOUT_H

#include <string>
#include "..\phshared\phshared.h"

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
		OSVERSIONINFOEX osvi;
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
	GetNativeSystemInfo(&si);

	std::ostringstream os;
	os<<"The remaining portion of the software may be distributed under the terms of the General Public License \n Contains SQLite,WTL, Boost and Process Hacker which have their own licenses and authors\n";
	if(osvi.dwMajorVersion==5 && osvi.dwMinorVersion==1)
	{
		os<<"Windows XP";
		if(si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_INTEL)
			os<<" 32 bit CPU";
	}
	if(osvi.dwMajorVersion==6 && osvi.dwMinorVersion==1 )
	{
		os<<"Windows 7";
		#if defined(_WIN64)
		os<<" 64bit client";
		#endif
		if(si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
			os<<" 64 bit CPU";
	}
		std::string file;
	GetModulePath(file);
	file.append("PHTrace.txt");
	os<<"\nError logging to: "<<file;
	
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
