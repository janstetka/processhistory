/////////////////////////////////////////////////////////////////////////////////////////
// 
//	MultiPaneStatusBarEx.h
//
//  Author: Pablo Aliskevicius.
//  Copyright (C) 2004 Pablo Aliskevicius.
//
//  The code and information  is provided  by the  author and  copyright  holder 'as-is',
//  and any express or implied  warranties,  including,  but not  limited to, the implied
//  warranties of  merchantability and  fitness for a particular purpose  are disclaimed.
//  In no event shall the author or copyright holders be liable for any direct, indirect,
//  incidental, special, exemplary, or consequential damages (including,  but not limited
//  to, procurement of substitute goods or services;  loss of use,  data, or profits;  or
//  business  interruption)  however caused  and on any  theory of liability,  whether in
//  contract, strict liability,  or tort  (including negligence or otherwise)  arising in
//  any way out of the use of this software,  even if advised of the  possibility of such
//  damage.
//
/////////////////////////////////////////////////////////////////////////////////////////
//
// Classes in this file:
//
// CProgressBarInPaneImpl<T> - CMPSBarWithProgress, 
//							   CMPSBarWithProgressAndBMP, 
//                            [CMPSBarWithProgressAndAnimation - not implemented],
//							   CMPSBarWithAll

// MultiPaneStatusBarWithProgress.h: interface for the CMPSBarWithProgress class.
//
// This class extends WTL::CMultiPaneStatusBarCtrl to support creating and updating a 
// progress bar in one of its panes, and moving that progress bar around.
//
// Usage:
// ======
//
//    Wherever you'd create a CMultiPaneStatusBarCtrl, create one of the CMPSBarWithXXXXX
//    classes instead (the one which provides the functionality you'll use).
//
//    Showing progress bars:
//	  ----------------------
//
//	     Show a progress bar using ProgCreate()	
//		 Hide it using ProgDestroyWindow()
//		 Display progress using ProgSetPos(), or ProgStepIt() after	ProgSetStep()
//		 All other functions of WTL::CProgressBarCtrl, version 7.1, are also exposed.
//
//
//    Showing anything (I don't like it, so there's an #ifdef):
//	  ---------------------------------------------------------
//
//       #define MULTI_PANE_WITH_ANY before including this file.
//
//		 Use CMPSBarWithAll as your member variable.
//
//		 Use AttachWindow() to attach an HWND to a pane.
//		 Use DetachWindow() to detach that HWND from that pane.
//
//
/////////////////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MULTIPANESTATUSBARWITHPROGRESS_H__D2F37B4C_6E3D_450D_94B5_B14D377226FA__INCLUDED_)
#define AFX_MULTIPANESTATUSBARWITHPROGRESS_H__D2F37B4C_6E3D_450D_94B5_B14D377226FA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __ATLCTRLS_H__
#error You have to include <ATLCTRLS.H> for CProgressBarCtrl, CStatic and CAnimateCtrl.
#endif
#ifndef __ATLCTRLX_H__
#error You have to include <ATLCTRLX.H> for CMultiPaneStatusBarCtrlImpl<T>.
#endif

/////////////////////////////////////////////////////////////////////////////////////////
// Implementation classes, meant to be inherited from (mixins).
/////////////////////////////////////////////////////////////////////////////////////////

template <class T>
class CProgressBarInPaneImpl
{
	public:
		typedef CProgressBarInPaneImpl<T> CThisClass;
		// Constructor, destructor...
		CProgressBarInPaneImpl<T>(): m_iProgressPane(-1) 
		{
		}
		virtual ~CProgressBarInPaneImpl<T>() 
		{
			ProgDestroyWindow();
		}
		// UpdatePanesLayout() override, to handle resizing the progress bar whenever relevant
		BOOL UpdatePanesLayout(void)
		{
			if (m_iProgressPane != -1)
			{
				T* pt = static_cast<T*>(this);
				RECT rc;
				pt->GetRect(m_iProgressPane, &rc);
				// ::InflateRect(&rc, -1, -1); 
				m_Progress.MoveWindow(&rc);
			}
			return TRUE; // Mixed function.
		}
		
		BEGIN_MSG_MAP(CProgressBarInPaneImpl<T>)
			MESSAGE_HANDLER(SB_SIMPLE, OnSimple)
		END_MSG_MAP()
			
		LRESULT OnSimple(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			if (::IsWindow(m_Progress.m_hWnd))
				m_Progress.ShowWindow(wParam ? SW_HIDE: SW_SHOW);

			bHandled = FALSE;
			return 0;
		}
		//////////////////////////////////////////////////////////////////////////////////
		// Prog* functions enable access to the contained CProgressBarCtrl. 
		// We'll expose most of its functionality, and a few additional bits.
		//////////////////////////////////////////////////////////////////////////////////

		BOOL ProgCreate(int iPane,         // Status pane where we'll create the progress bar.
			int nMin = 0, int nMax = 100,  // Progress bar initial range
			DWORD dwStyle = WS_CHILD | WS_VISIBLE | PBS_SMOOTH, // Progress bar styles
			DWORD dwExStyle = 0
			) 
		{
			// Check there is such a pane
			T* pt = static_cast<T*>(this);
			ATLASSERT(::IsWindow(pt->m_hWnd));
			if (iPane >= pt->m_nPanes)
				return FALSE;
			// Check there is not a progress bar already open.
			if (::IsWindow(m_Progress.m_hWnd))
				return FALSE;
			// Get the pane's rectangle
			RECT rc;
			pt->GetRect( iPane, &rc );
			// ::InflateRect(&rc, -1, -1); 
			
			// Create the window, using the status bar (this) as a parent.
			m_Progress.Create ( pt->m_hWnd, rc, NULL, dwStyle,  dwExStyle);
			// Set the progress bar's range and position
			m_Progress.SetRange ( nMin, nMax ); 
			m_Progress.SetPos ( nMin );   
			m_Progress.SetStep ( 1 );
			// Hold this, we'll need it to move around.
			m_iProgressPane = iPane;
			return TRUE;
		}
		// This function can be used to close a progress bar, after ending whatever 
		// lengthy operation justified opening it to begin with.
		void ProgDestroyWindow(void)
		{
			if (::IsWindow(m_Progress.m_hWnd))
			{
				m_Progress.ShowWindow(SW_HIDE);
				m_Progress.DestroyWindow();
			}
			m_iProgressPane = -1;
			m_Progress.m_hWnd = NULL;
		}
		// Just in case. 
		int   ProgGetPane() const                 { return m_iProgressPane; }
		//////////////////////////////////////////////////////////////////////////////////
		// CProgressBarCtrl functionality (WTL version 7.1):
		// CWindow functionality in CProgressBarCtrl is hidden by design.
		//
		DWORD ProgSetRange(int nLower, int nUpper)  { return m_Progress.SetRange(nLower, nUpper); }
		int   ProgSetPos(int nPos)                  { return m_Progress.SetPos(nPos); }
		int   ProgOffsetPos(int nPos)               { return m_Progress.OffsetPos(nPos); }
		int   ProgSetStep(int nStep)                { return m_Progress.SetStep(nStep); }
		UINT  ProgGetPos() const                    { return m_Progress.GetPos(); }
		
		void  ProgGetRange(PPBRANGE pPBRange) const { m_Progress.GetRange(pPBRange); } 
		int   ProgGetRangeLimit(BOOL bLimit)  const { return m_Progress.GetRangeLimit(bLimit); }
		DWORD ProgSetRange32(int nMin, int nMax)    { return m_Progress.SetRange32(nMin, nMax); }
		
#if (_WIN32_IE >= 0x0400) && !defined(_WIN32_WCE)
		COLORREF ProgSetBarColor(COLORREF clr)      { return m_Progress.SetBarColor(clr); }
		COLORREF ProgSetBkColor(COLORREF clr)       { return m_Progress.SetBkColor(clr); }
#endif //(_WIN32_IE >= 0x0400) && !defined(_WIN32_WCE)
		
#if (_WIN32_WINNT >= 0x0501) && defined(PBM_SETMARQUEE)
		BOOL ProgSetMarquee(BOOL bMarquee, UINT uUpdateTime = 0U)
													{ return m_Progress.SetMarquee(bMarquee, uUpdateTime); }
#endif //(_WIN32_WINNT >= 0x0501) && defined(PBM_SETMARQUEE)
		int ProgStepIt()                            { return m_Progress.StepIt(); }
		
	//protected:
	//////////////////////////////////////////////////////////////////////////////////
	// Member variables (of course, protected)
	//
		CProgressBarCtrl m_Progress;  // This is the contained control.
		int m_iProgressPane;          // Pane ordinal where the progress bar resides, or -1 when off.
	
}; // CProgressBarInPaneImpl

/////////////////////////////////////////////////////////////////////////////////////////
// Concrete classes, you can use them as member variables.
/////////////////////////////////////////////////////////////////////////////////////////

// This class adds progress bar functionality to a multi pane status bar
class CMPSBarWithProgress: 
      public CMultiPaneStatusBarCtrlImpl<CMPSBarWithProgress>,
	  public CProgressBarInPaneImpl<CMPSBarWithProgress>

{
public:

    DECLARE_WND_SUPERCLASS(_T("CMPSBarWithProgress"), GetWndClassName())

	BEGIN_MSG_MAP(CMPSBarWithProgress)
		CHAIN_MSG_MAP(CProgressBarInPaneImpl<CMPSBarWithProgress>)
		CHAIN_MSG_MAP(CMultiPaneStatusBarCtrlImpl<CMPSBarWithProgress>)
	END_MSG_MAP()

	BOOL UpdatePanesLayout(void)
	{
		BOOL ret = CMultiPaneStatusBarCtrlImpl<CMPSBarWithProgress>::UpdatePanesLayout();
		CProgressBarInPaneImpl<CMPSBarWithProgress>::UpdatePanesLayout();
		return ret;
	}
};	// class CMPSBarWithProgress

#endif // !defined(AFX_MULTIPANESTATUSBARWITHPROGRESS_H__D2F37B4C_6E3D_450D_94B5_B14D377226FA__INCLUDED_)
