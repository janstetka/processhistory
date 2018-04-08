#include "..\resource.h"
#include "..\..\ProcessHistory\MainFrm.h"
#include "..\screen.h"
#include "..\PH.h"
#include "..\query.h"
#include <mutex>
#include "..\..\phshared\phshared.h"

extern PHDisplay phd;
extern PH ph_instance;
using namespace std;
extern PHQuery phq;
extern mutex db_mutex;

LRESULT PHScroll::OnRB(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)//TODO: right click 2 seqentially not updating phti, if scrolled and right click resets scroll and draws selected over
{
	int x=GET_X_LPARAM(lParam);
	int y=GET_Y_LPARAM(lParam);

	POINT pt;
	pt.x=x;
	pt.y=y;
	PHDToL(pt);
	/*Previously selected*/
	RECT rold;
	map<long,RECT>::iterator sit=phd._ProcessAreas.find(phd._selected);
	if(sit!=phd._ProcessAreas.end())
	{
		//CreateScreenBuffer();
		rold=sit->second;
		PHLToD(rold);
		//InvalidateRect(&r);
	}
	
	for(map<long,RECT>::iterator it=phd._ProcessAreas.begin();
		it!=phd._ProcessAreas.end();it++)
	{
		if(PtInRect(&it->second,pt))
		{
			phd._selected=it->first;
			
			CreateScreenBuffer();
			RECT r=it->second;
			PHLToD(r);
			InvalidateRect(&r);
			InvalidateRect(&rold);
			UpdateWindow();
			CMenu menu;
			menu.LoadMenu( IDR_CONTEXT_MENU);
			CMenuHandle mh=menu.GetSubMenu(0);
			// convert to point clicked to screen coordinates/ hwnd needs to handle message
			POINT menupt;
			menupt.x=x;
			menupt.y=y;
			ClientToScreen(&menupt);
			TrackPopupMenu((HMENU)mh, TPM_LEFTALIGN, 
				menupt.x, menupt.y,0, ph_instance._hWnd,NULL);
			return TRUE;
		}
	}
	phd._selected=0;
	CreateScreenBuffer();
	InvalidateRect(&rold);
	UpdateWindow();
	::PostMessage(ph_instance._hWnd, UWM_DOWNLOAD_PROGRESS, 4, 0);
	SetFocus();
	return TRUE;
}
LRESULT PHScroll::OnLB(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SetFocus();
	bHandled = FALSE;
	return TRUE;
}
// wParam = current progress. lParam = total progress.
#define UWM_DOWNLOAD_PROGRESS   (WM_APP)
LRESULT PHScroll::OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	if (!phd._complete)
		return TRUE;
	if (phd._selected > 0)
		return TRUE;
	int x = GET_X_LPARAM(lParam);
	int y = GET_Y_LPARAM(lParam);

	POINT pt;
	pt.x = x;
	pt.y = y;
	PHDToL(pt);
	for (map<long, RECT>::iterator it = phd._ProcessAreas.begin();
		it != phd._ProcessAreas.end(); it++)
	{
		if (PtInRect(&it->second, pt))
		{
			if (phd._mouseover != it->first)// process ID update phti - only send message if different from last time
			{
				phd._mouseover = it->first;
				::PostMessage(ph_instance._hWnd, UWM_DOWNLOAD_PROGRESS, 2, 0);
			}
			return TRUE;
		}
	}
	if (phd._mouseover > 0)
	{
		phd._mouseover = -1;
		::PostMessage(ph_instance._hWnd, UWM_DOWNLOAD_PROGRESS, 3, 0);		
	}
	return TRUE;
}

void PHScroll::SetSize(SIZE m_size)
{		
	SetScrollOffset(0, 0, FALSE);
	SetScrollSize(m_size);
}

void PHScroll::PHLToD(RECT &r)
{	
		/*translate pts*/
	//only microsoft could create this coordinate system
	CWindowDC dc(m_hWnd);
	dc.SetMapMode(MM_TEXT);
	dc.SetViewportOrg(-m_ptOffset.x, -m_ptOffset.y);
	dc.SetViewportExt(m_sizeAll);
	dc.LPtoDP(&r);
}

void PHScroll::PHDToL(POINT &p)
{
		/*translate pts*/
	//only microsoft could create this coordinate system
	CWindowDC dc(m_hWnd);
	dc.SetMapMode(MM_TEXT);
	dc.SetViewportOrg(-m_ptOffset.x, -m_ptOffset.y);
	dc.SetViewportExt(m_sizeAll);
	dc.DPtoLP(&p);
}

LRESULT PHScroll::OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
{
		switch(wParam)
		{
	
		case VK_RIGHT:
		{
			LeftRight(wParam);
		}
		break;
		case VK_LEFT:
		{
			LeftRight(wParam);
		}//case VK_LEFT
		break;
		}//switch vKey
	return true;
}

extern map<string, long> g_clUser;

void CMainFrame::FillUserList()
{
	//remember selected user if any
	char User[300];
	int length = ::GetWindowTextLength(ph_instance._hWndCombo3);
	if (length>0)
		::GetWindowText(ph_instance._hWndCombo3, User, length+1);
	m_Combo3.ResetContent();
	for (map<string, long>::iterator it = g_clUser.begin(); it != g_clUser.end();it++)
			m_Combo3.AddString(it->first.c_str());
	m_Combo3.AddString("All");
	int sel=m_Combo3.FindString(0,User);
	m_Combo3.SetCurSel(sel);
}
LRESULT PHTimeInfo::OnRB(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	int x = GET_X_LPARAM(lParam);
	int y = GET_Y_LPARAM(lParam);
	CMenu menu;
	menu.LoadMenu(IDR_DETAIL_CONTEXT_MENU);

	CMenuHandle mh = menu.GetSubMenu(0);
	// convert to point clicked to screen coordinates/ hwnd needs to handle message
	POINT menupt;
	menupt.x = x;
	menupt.y = y;
	ClientToScreen(&menupt);
	TrackPopupMenu((HMENU)mh, TPM_LEFTALIGN,
		menupt.x, menupt.y, 0, ph_instance._hWnd, NULL);
	return true;
}
LRESULT CMainFrame::CopyDetailsToClipboard(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	//CString strData;
	//_ps.GetWindowText(strData);

	// test to see if we can open the clipboard first before
	// wasting any cycles with the memory allocation
	if (OpenClipboard())
	{
		// Empty the Clipboard. This also has the effect
		// of allowing Windows to free the memory associated
		// with any data that is in the Clipboard
		EmptyClipboard();

		// Ok. We have the Clipboard locked and it's empty. 
		// Now let's allocate the global memory for our data.

		// Here I'm simply using the GlobalAlloc function to 
		// allocate a block of data equal to the text in the
		// "to clipboard" edit control plus one character for the
		// terminating null character required when sending
		// ANSI text to the Clipboard.
		HGLOBAL hClipboardData;

		if (phd._selected>0)
			hClipboardData = GlobalAlloc(GMEM_DDESHARE,
			phti._ps.size() + 1);
		else if (phd._mouseover>0)
			hClipboardData = GlobalAlloc(GMEM_DDESHARE,
			phti._ps2.size() + 1);
		
		if (phd._selected>0)
		hClipboardData = GlobalAlloc(GMEM_DDESHARE,
			phti._ps.size() + 1);
		else
			hClipboardData = GlobalAlloc(GMEM_DDESHARE,
			phti._ps.size() + 1);
		// Calling GlobalLock returns to me a pointer to the 
		// data associated with the handle returned from 
		// GlobalAlloc
		char * pchData;
		pchData = (char*)GlobalLock(hClipboardData);

		// At this point, all I need to do is use the standard 
		// C/C++ strcpy function to copy the data from the local 
		// variable to the global memory.
		if (phd._selected>0)
			strcpy(pchData, LPCSTR(phti._ps.c_str()));
		else if (phd._mouseover>0)
			strcpy(pchData, LPCSTR(phti._ps2.c_str()));
		// Once done, I unlock the memory - remember you 
		// don't call GlobalFree because Windows will free the 
		// memory automatically when EmptyClipboard is next 
		// called. 
		GlobalUnlock(hClipboardData);

		// Now, set the Clipboard data by specifying that 
		// ANSI text is being used and passing the handle to
		// the global memory.
		SetClipboardData(CF_TEXT, hClipboardData);

		// Finally, when finished I simply close the Clipboard
		// which has the effect of unlocking it so that other
		// applications can examine or modify its contents.
		CloseClipboard();
	}
	return 0;
}
#include <atldlgs.h>
#include <atlimage.h>
	LRESULT CMainFrame::OnExport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CFileDialog dlg(FALSE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
			"All Files (*.*)\0*.*\0");
		if (IDOK == dlg.DoModal())
		{
	CImage cimage;
	
	int imageh=(phq._lCount * 42) + phts._HeightTimeLine;
	int imagew = phd._Width;
	if (phd._selected > 0)
	{
		imageh += phti._Height;
		if (phti._Width > imagew)
			imagew = phti._Width;
	}
			
		COLORREF cr;
		cr = 0x00FFFFFF;
		
		CBrush FillBrush;
		FillBrush.CreateSolidBrush(cr);
		RECT rPanel;
		rPanel.top = rPanel.left = 0;
		rPanel.right = imagew;
		rPanel.bottom = imageh;

		CMemoryDC* _MemDC = new CMemoryDC(GetDC(), rPanel);

		if (_MemDC->FillRect(&rPanel, FillBrush) == 0)
			PHTrace(Win32Error(), __LINE__, __FILE__);
	RECT r;
	r.top = 0;
	r.left = 0;
	r.right = phd._Width;
	r.bottom = phq._lCount * 42;
		if(_MemDC->BitBlt(r.left, r.top, r.right-r.left, r.bottom-r.top,
		*phs._MemDC, 0, 0, SRCCOPY)==0)
		PHTrace(Win32Error(), __LINE__, __FILE__);
		r.top = phq._lCount * 42;
		r.bottom = (phq._lCount * 42)+phts._HeightTimeLine;
		if(_MemDC->BitBlt(r.left, r.top, r.right-r.left, r.bottom-r.top,
		*phts._MemDC, 0, 0, SRCCOPY)==0)
		PHTrace(Win32Error(), __LINE__, __FILE__);
		if (phd._selected > 0)
		{
			r.top = (phq._lCount * 42) + phts._HeightTimeLine;
			r.bottom = (phq._lCount * 42) + phts._HeightTimeLine+phti._Height;
			r.right = phti._Width;

			if(_MemDC->BitBlt(r.left, r.top, r.right - r.left, r.bottom - r.top,
				*phti._MemDC, 0, 0, SRCCOPY)==0)
				PHTrace(Win32Error(), __LINE__, __FILE__);
		}
		try{
		cimage.Attach(_MemDC->GetCurrentBitmap());
		cimage.Save(dlg.m_szFileName, GUID_NULL);
		}
		catch(...)
		{
		PHTrace(Win32Error(), __LINE__, __FILE__);
		}
		
		delete _MemDC;
	}
		return 0;
	}

	bool /*CMainFrame::*/FilterUser()
	{
		//char User[300];
		//int length = GetCurrentSel(ph_instance._hWndCombo3);
		//m_Combo3.GetWindowText(User,length);
		char User[300];
		int length = GetWindowTextLength(ph_instance._hWndCombo3);
		if (length>0)
			GetWindowText(ph_instance._hWndCombo3, User, length+1);
		map<string,long>::iterator it = g_clUser.find(string(User));
		if (it != g_clUser.end())
		{
			phd.filterUserID = it->second;
			return true;
		}
		else
		{
			phd.filterUserID = -1;
			return false;
		}
	}
#include "..\..\phshared\Crc32Static.h"
	set<string> PathSet;
	bool /*CMainFrame::*/FilterExec()
	{
		char Path[300];
		int length = GetWindowTextLength(ph_instance._hWndCombo2);
		if (length>0)
			GetWindowText(ph_instance._hWndCombo2, Path, length+1);
		DWORD dwCRC=-1;
#if defined (_WIN64)
		CCrc32Static::FileCrc32Win32(Path, dwCRC);
#else
		CCrc32Static::FileCrc32Assembly(dlg.m_szFileName, dwCRC);
#endif
		
		phd.filterCRC = dwCRC;
		lock_guard<mutex> sl(db_mutex);
		ostringstream os2;
		os2 << "SELECT ID FROM Process WHERE CRC=" << phd.filterCRC << ";";
		sqlite3 *db2 = OpenDB();
		sqlite3_stmt* stmt2;
		if (sqlite3_prepare(db2, os2.str().c_str(), -1, &stmt2, NULL) != SQLITE_OK)
		DBError(sqlite3_errmsg(db2), __LINE__, __FILE__);
		if (sqlite3_step(stmt2) == SQLITE_ROW)
		{
			if (sqlite3_column_int(stmt2, 0) > 0)
			{
				phd.filter_exec = true;
				sqlite3_finalize(stmt2);
				sqlite3_close(db2);
				set<string>::iterator it = PathSet.find(string(Path));
				if (it == PathSet.end())
				{
					::SendMessage(ph_instance._hWndCombo2, CB_ADDSTRING, 0, (LPARAM)Path);
					PathSet.insert(string(Path));
				}
				return true;
			}
			else
			{
				::SetWindowText(ph_instance._hWndStatusBar, "Executable not in database");
				phd.filter_exec = false;
			}
		}
		else
		{
			::SetWindowText(ph_instance._hWndStatusBar, "Executable not in database");
			phd.filter_exec = false;
		}
		sqlite3_finalize(stmt2);
		sqlite3_close(db2);
		return false;
		
	}

	void ReplaceStringInPlace(std::string& subject, const std::string& search,
		const std::string& replace) {
		size_t pos = 0;
		while ((pos = subject.find(search, pos)) != std::string::npos) {
			subject.replace(pos, search.length(), replace);
			pos += replace.length();
		}
	}

#include <boost/filesystem.hpp>

	LRESULT CMainFrame::OnExecRunSelWCL(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		if (phd._selected>0)
		{
			map<long, string>::iterator qpit = phd.qrypaths.find(phd._selected);
			if (qpit != phd.qrypaths.end())
			{
				if (boost::filesystem::exists(qpit->second))
				{
					lock_guard<mutex> sl(db_mutex);
					sqlite3 *db = OpenDB();
					sqlite3_stmt * stmt;
					ostringstream os;
					os <<
						"SELECT CommandLine FROM Process  JOIN CommandLines ON Process.clid=CommandLines.ID WHERE Process.ID=";
					os << phd._selected;
					os << ";";
					if (sqlite3_prepare(db, os.str().c_str(), -1, &stmt, NULL) != SQLITE_OK)
						DBError(sqlite3_errmsg(db), __LINE__, __FILE__);
					const unsigned char *commandline = 0;
					string cltxt;
					if (sqlite3_step(stmt) == SQLITE_ROW)
					{
						commandline = sqlite3_column_text(stmt, 0);
						if (commandline != 0)
						{//strip out path
							cltxt = (char*)commandline;
							ReplaceStringInPlace(cltxt, qpit->second, "");
							if ((int)ShellExecute(NULL, "Open", qpit->second.c_str(), cltxt.c_str(), NULL, SW_SHOWDEFAULT)<33)
								PHTrace(Win32Error(), __LINE__, __FILE__);
						}
					}
					sqlite3_finalize(stmt);
					sqlite3_close(db);

				}
				else
					MessageBox("The executable no longer exists", "Executable no longer exists", MB_OK);
			}
		}
		return 0;
	}
	LRESULT CMainFrame::OnSetUserFilter(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		ostringstream os2;
		os2 << "SELECT UserName FROM Process JOIN PHLogUser ON Process.UserID=PHLogUser.ID WHERE Process.ID= ";

		os2 << phd._selected;
		os2 << ";";
		sqlite3 *db2 = OpenDB();
		sqlite3_stmt* stmt2;
		if (sqlite3_prepare(db2, os2.str().c_str(), -1, &stmt2, NULL) != SQLITE_OK)
			DBError(sqlite3_errmsg(db2), __LINE__, __FILE__);
		const unsigned char *user=0;
		//string usertxt;
		int userid;
		if (sqlite3_step(stmt2) == SQLITE_ROW)
			user = sqlite3_column_text(stmt2, 0);
			
		if (user != 0)
		{
			//phd.filterUserID = userid;
			int sel = m_Combo3.FindString(0,(char*) user);
			m_Combo3.SetCurSel(sel);
			sqlite3_finalize(stmt2);
			sqlite3_close(db2);
			ParseQry();
			phs.SetFocus();
			return 0;
		}
		sqlite3_finalize(stmt2);
		sqlite3_close(db2);
		return 0;
	}

	LRESULT CMainFrame::OnStatusBarDblClick(int /*wParam*/, LPNMHDR lParam, BOOL& bHandled)
	{
		LPNMMOUSE pnmm = (LPNMMOUSE)lParam;
		bHandled = FALSE;
		//PNASSERT(static_cast<int>(pnmm->dwItemSpec) < m_StatusBar.m_nPanes);
		UINT nID = m_sBar.m_pPane[pnmm->dwItemSpec];

		switch (nID)
		{
		case ID_PANE_2:
			m_sBar.SetPaneText(ID_PANE_2, phq.long_processes ?  _T("EVENT") :_T("ALL") );
			phq.long_processes = !phq.long_processes;
			ParseQry();
			bHandled = TRUE;
			return TRUE;
		break;
		}
		return FALSE;
	}
	LRESULT CMainFrame::OnComboChange(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL& /*bHandled*/)
	{
		//TODO: get text here and then check and use if necessary further on?
	//		COMMAND_HANDLER(ID_EXEC_FILTER_COMBO, CBN_SELCHANGE, OnComboChange)
//			COMMAND_HANDLER(ID_USERFILTER_COMBO, CBN_SELCHANGE, OnComboChange)
		int sel=m_Combo.GetCurSel();
		char lbtext[300];
		if (wID == ID_PLACEHOLDER)
			//int LBTlen=m_Combo.GetLBTextLen(sel);
		m_Combo.GetLBText(sel, lbtext);
		ParseQry();//lbtext);
		return 0;
	}
	LRESULT CMainFrame::OnCommandEnter(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		ParseQry();
		return 0;
	}