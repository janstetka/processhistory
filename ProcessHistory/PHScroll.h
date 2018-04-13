#ifndef PH_SCROLL
#define PH_SCROLL
#include <atlbase.h>
#include <atlapp.h>
#include <atlscrl.h>
#include <atlcrack.h>

class PHScroll: public CScrollWindowImpl<PHScroll>
{
public:
	PHScroll()
	{
		_MemDC = NULL;
	}
	
	DECLARE_WND_CLASS(NULL)
	BEGIN_MSG_MAP(PHScroll)
		MESSAGE_HANDLER(WM_RBUTTONDOWN, OnRB)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLB)
		 MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		 MESSAGE_HANDLER(WM_MOUSEMOVE,OnMouseMove)
		 CHAIN_MSG_MAP(CScrollWindowImpl<PHScroll>);
	END_MSG_MAP()

	void DoPaint(CDCHandle hDC);
	
	LRESULT OnRB(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnLB(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void SetSize(SIZE);
	void PHLToD(RECT &r);
	void PHDToL(POINT &p);
	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled);

	BOOL PreTranslateMessage(MSG* pMsg)
	{
		pMsg;
		return FALSE;
	}

	void CreateScreenBuffer();
	void DrawProcess(long lID, RECT rcEvent);
	CMemoryDC* _MemDC;
};

#include <string>
#include "..\sqlite\sqlite3.h"

class PHTimeInfo : public CScrollWindowImpl<PHTimeInfo>
{
public:
	PHTimeInfo()
	{
		_MemDC = NULL;
		_Height=500;
	}
	~PHTimeInfo()
	{
		sqlite3_close(tidb);
	}
	
	DECLARE_WND_CLASS(NULL)
	BEGIN_MSG_MAP(PHTimeInfo)
		MESSAGE_HANDLER(WM_RBUTTONDOWN, OnRB)
		CHAIN_MSG_MAP(CScrollWindowImpl<PHTimeInfo>);
	END_MSG_MAP()

	LRESULT OnRB(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	void DoPaint(CDCHandle dc);
	
	void CreateScreenBuffer();
	void SetSize(SIZE);

	void DisplayInfo();
	BOOL PreTranslateMessage(MSG* pMsg)
	{
		pMsg;
		return FALSE;
	}
	long _ID;
	HICON _hIcon;
	std::string _ps;// , _ps2;
	int _Height,_Width;
	CMemoryDC* _MemDC;

	void DrawMemDC();
	void CalculateRect();
	sqlite3 * tidb;
};

class PHTimeScale : public CWindowImpl<PHTimeScale>
{
public:
	PHTimeScale()
	{
		_MemDC = NULL;
		_HeightTimeLine = 10;
	}
	DECLARE_WND_CLASS(_T("My Window Class"))
	BEGIN_MSG_MAP(PHTimeScale)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
	END_MSG_MAP()

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	CMemoryDC* _MemDC;

	void DrawTimeAxis();
	int _HeightTimeLine;
	void CalcTimeScale();
};

#endif