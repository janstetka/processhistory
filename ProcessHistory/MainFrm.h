// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <atlbase.h>
#include <atlstr.h>
#include <atlapp.h>

extern CAppModule _Module;

#include <atlframe.h>
//#define _WTL_NO_CSTRING
#include "atlctrls.h"
#include "atlctrlw.h"


#include <atlmisc.h>
#include "ToolbarHelper.h"

#include "..\PHQuery\resource.h"
#include "PHScroll.h"
#include <atlsplit.h>
//
#include <atlcrack.h>
#include "gdiplus.h"

#include <atlctrlx.h>
#include "MultiPaneStatusBarEx.h"

// wParam = current progress. lParam = total progress.
#define UWM_DOWNLOAD_PROGRESS   (WM_APP)

class CMainFrame : public CFrameWindowImpl<CMainFrame>,
	public CMessageFilter, public CToolBarHelper<CMainFrame>/*, public CIdleHandler*/
{
public:
	
	CComboBox m_Combo, m_Combo2, m_Combo3;
	CCommandBarEdit m_cmdTextBox, m_cmdTextBox2;
	CCommandBarCtrl m_CmdBar;
	PHScroll phs;
	PHTimeInfo phti;
	CHorSplitterWindow m_wndHorzSplit, m_wndHorzSplit2;
	PHTimeScale phts;

	CMPSBarWithProgress m_sBar; //additonal progress for XP

	ULONG_PTR           gdiplusToken;
	DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		if (CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
			return TRUE;

		return phs.PreTranslateMessage(pMsg);
	}

	void OnToolBarCombo(HWND hWndCombo, UINT nID, int nSel, LPCTSTR lpszText, DWORD dwItemData){}
	
	BEGIN_MSG_MAP(CMainFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_ID_HANDLER(ID_FILE_EXIT, OnFileExit)
		COMMAND_ID_HANDLER(ID_HELP_ABOUT, OnHelpAbout)
		COMMAND_ID_HANDLER(ID_PH_FONT, OnFont)
		COMMAND_ID_HANDLER(ID_VIEW_KEY, OnViewKey)
		COMMAND_ID_HANDLER(ID_BUTTON40092, GoToMostRecent)
		COMMAND_ID_HANDLER(ID_PROCESS_VIEWSELECTED, OnProcessViewSelected)
		COMMAND_ID_HANDLER(ID_IMAGES_RUNSELECTED, OnExecRunSel)
		COMMAND_ID_HANDLER(ID_IMAGES_OPENCONTAININGFOLDER, OnExecOCF)
		COMMAND_ID_HANDLER(ID_CONTEXT_GOTOSTART, OnGotoBegin)
		COMMAND_ID_HANDLER(ID_CONTEXT_GOTOEND, OnGotoEnd)
		COMMAND_ID_HANDLER(ID_BUTTON40069, PHTBQry)
		COMMAND_ID_HANDLER(ID_BUTTON40071, PHTBLeft)
		COMMAND_ID_HANDLER(ID_BUTTON40070, PHTBRight)
		MESSAGE_HANDLER(UWM_DOWNLOAD_PROGRESS, NotifyGUI)
		COMMAND_ID_HANDLER(ID_PH_EXECUTABLE, OnFilterExecutable)
		COMMAND_ID_HANDLER(ID_PH_EXPORT, OnExport)
		COMMAND_ID_HANDLER(ID_CONTEXT_FILTEREXECUTABLE, OnSetExecFilter)
		COMMAND_ID_HANDLER(ID_CONTEXT_RUNWITHCOMMANDLINE, OnExecRunSelWCL)
		COMMAND_ID_HANDLER(ID__COPYTEXT, CopyDetailsToClipboard)
		COMMAND_ID_HANDLER(ID_CONTEXT_SETUSERFILTER, OnSetUserFilter)
		NOTIFY_HANDLER(ATL_IDW_STATUS_BAR, NM_DBLCLK, OnStatusBarDblClick)
		COMMAND_HANDLER(ID_PLACEHOLDER, CBN_SELCHANGE, OnComboChange)
		COMMAND_HANDLER(ID_EXEC_FILTER_COMBO, CBN_SELCHANGE, OnComboChange)
		COMMAND_HANDLER(ID_USERFILTER_COMBO, CBN_SELCHANGE, OnComboChange)
		MESSAGE_HANDLER(BXT_WM_ENTER, OnCommandEnter)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
		CHAIN_MSG_MAP(CToolBarHelper<CMainFrame>)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		Gdiplus::GdiplusShutdown(gdiplusToken);
		// unregister message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->RemoveMessageFilter(this);

		bHandled = FALSE;
		return 1;
	}

	LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		PostMessage(WM_CLOSE);
		return 0;
	}

	LRESULT NotifyGUI(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnHelpAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFont(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT PHTBQry(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT PHTBLeft(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT PHTBRight(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnViewKey(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT GoToMostRecent(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnProcessViewSelected(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnExecRunSel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnExecOCF(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnGotoBegin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnGotoEnd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnFilterExecutable(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSetExecFilter(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnExecRunSelWCL(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnExport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT CopyDetailsToClipboard(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnSetUserFilter(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
	LRESULT OnStatusBarDblClick(int /*wParam*/, LPNMHDR lParam, BOOL& bHandled);
	LRESULT OnComboChange(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/);
	LRESULT OnCommandEnter(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	void ParseQry();// char * lbtext = NULL);
	void DateTimeQry();
	void FillUserList();
};
 



class KeyCtrl : public CDialogImpl<KeyCtrl>
{
public:
	enum { IDD = IDD_KEY };
	BEGIN_MSG_MAP(KeyCtrl)
		COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
	END_MSG_MAP()
	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}	
};