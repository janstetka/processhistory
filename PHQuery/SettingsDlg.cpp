#include "SettingsDlg.h"
#include "resource.h"
#include "PH.h"
#include <sstream>
#include "..\phshared\phshared.h"
#include <mutex>
#include "..\PHLogger\PHLogUser.h"

using namespace std;
using namespace boost::posix_time;
using namespace boost;

long CreateUser(HWND);
void DeleteUser(long lID);

extern PH ph_instance;
extern mutex db_mutex;

LRESULT SettingsDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	if(LookUpUser()<1)
	{
		m_bUser=false;
		SetDlgItemText(IDC_USERNAME,"Not a PH user.");
		SetDlgItemText(IDC_USER_BTN,"Create");
	}
	else
	{
		m_bUser=true;
		if(SetDlgItemText(IDC_USERNAME,ph_instance.GetUser())==0)
			PHTrace(Win32Error(),__LINE__,__FILE__);
		SetDlgItemText(IDC_USER_BTN,"Delete");
	}

		return TRUE;
}



			LRESULT  SettingsDlg::OnUser(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
			{
			if(m_bUser)
			{
				DeleteUser(ph_instance.GetUserID());
				SetDlgItemText(IDC_USERNAME,"Not a PH user.");
				m_bUser=false;
			}
			else
			{
				::EnableWindow(GetDlgItem(IDC_USER_BTN),FALSE);
				ph_instance.SetUserID(CreateUser(m_hWnd));
				m_bUser=true;
				ReadUserTable();
				SetDlgItemText(IDC_USER_BTN,"Delete");
				::EnableWindow(GetDlgItem(IDC_USER_BTN),TRUE);
			}

				return TRUE;
		}
		

long LookUpUser()
{
	lock_guard<mutex> sl(db_mutex);
	char * szErr=0;
	sqlite3 *db;
	db = OpenDB();
	string clSQL;
	clSQL="CREATE TABLE PHLogUser(ID INTEGER PRIMARY KEY,UserName varchar(50) UNIQUE);";/*check exists*/

	sqlite3_exec(db,clSQL.c_str(),NULL,NULL,&szErr);
	
	clSQL="SELECT ID FROM PHLogUser WHERE UserName='";
	clSQL.append(ph_instance.GetUser());
	clSQL.append("';");

	sqlite3_stmt* stmt;
	int iErr;
		
	iErr=sqlite3_prepare(db,clSQL.c_str(),-1,&stmt,NULL);
		
	if(iErr!=SQLITE_OK)
		DBError(sqlite3_errmsg(db),__LINE__,__FILE__);
		
	long lUserID=-1;
	
	iErr=sqlite3_step(stmt);
	lUserID=sqlite3_column_int(stmt,0);
	if(iErr==SQLITE_ROW){}
	
	sqlite3_finalize(stmt);
	sqlite3_close(db);
				
	return lUserID;
}

long CreateUser(HWND h)
{
	lock_guard<mutex> sl(db_mutex);
	long lUserID;
	string clSQL;
	
	sqlite3 *db=OpenDB();
	clSQL="INSERT INTO PHLogUser (UserName) VALUES('";

	clSQL.append(ph_instance.GetUser());
	clSQL.append("');");
	char * Err;

	if(SQLITE_OK!=sqlite3_exec(db,clSQL.c_str(),NULL,NULL,&Err))
		DBError(sqlite3_errmsg(db),__LINE__,__FILE__);
	
	lUserID=sqlite3_last_insert_rowid(db);
	
	sqlite3_close(db);
	if(SetDlgItemText(h,IDC_USERNAME,ph_instance.GetUser())==0)
		PHTrace(Win32Error(),__LINE__,__FILE__);
	
	return lUserID;
}
void DeleteUser(long lID)
{
	lock_guard<mutex> sl(db_mutex);
	ostringstream os;
	os<<"DELETE FROM PHLogUser WHERE ID=";
	os<<lID<<";";
	sqlite3 *db;
	char * szErr=0;
	db = OpenDB();

		if(SQLITE_OK!=sqlite3_exec(db,os.str().c_str(),NULL,NULL,&szErr))
			DBError(sqlite3_errmsg(db),__LINE__,__FILE__);
	sqlite3_close(db);
}

