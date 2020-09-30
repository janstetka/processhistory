#if !defined (SETTINGS_H)
#define SETTINGS_H

#include <string>
//#include "..\phshared\phshared.h"

class SettingsCtrl : public CDialogImpl<SettingsCtrl>
{
public:
	enum { IDD = IDD_SETTINGS_DLG };
	BEGIN_MSG_MAP(SettingsCtrl)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)

	END_MSG_MAP()
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{


		return TRUE;
	}
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{		
		char q[100];
		int ql = ::GetWindowTextLength(GetDlgItem(IDC_REFRESH));
		GetDlgItemText(IDC_REFRESH, q,ql+1);
		_Refresh = std::string(q);
		EndDialog(wID);
		return 0;
	}
	std::string _Refresh;
};
#endif
