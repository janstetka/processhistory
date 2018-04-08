#if !defined (USERDLG_H)
#define USERDLG_H

#include "resource.h"
#include "atlbase.h"
#include "atlapp.h"
#include "atldlgs.h"

class SettingsDlg : public CDialogImpl<SettingsDlg>
{
public:
	enum { IDD = IDD_SETTINGS_DLG };
	BEGIN_MSG_MAP(SettingsDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
		COMMAND_ID_HANDLER(IDC_USER_BTN,OnUser)
	END_MSG_MAP()

			LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT  OnUser(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
	EndDialog(wID);
	return 0;
	}
	//long _ID;
	bool m_bUser;
};
#endif