/*Renders the query results using 
GDI+ */
#include "..\ProcessHistory\MainFrm.h"
#include "screen.h"
#include "query.h"
#include "..\phshared\phshared.h"
#include "..\ProcessHistory\PHScroll.h"
#include "PH.h"
#include "gdiplus.h"

using namespace std;

PHDisplay phd;
extern PHQuery phq;
extern PH ph_instance;

void DrawLines(HDC hDC,RECT rcEvent,long lID)
{
	Gdiplus::Graphics graphics(hDC);
	/*Process Start*/
		
	Gdiplus::Color c;
	c.SetFromCOLORREF(0x00CC33);
	Gdiplus::Pen pen(c);
	pen.SetWidth(3);
	if(phd._clipped_left.find(lID)==phd._clipped_left.end() )
		graphics.DrawLine(&pen,rcEvent.left,rcEvent.top,rcEvent.left,rcEvent.bottom);
		
	//Process End
	c.SetFromCOLORREF(0x1111EE);
	pen.SetColor(c);

	//Don't draw end if clipped
	if(phd._clipped_right.find(lID)==phd._clipped_right.end() )
		graphics.DrawLine(&pen,rcEvent.right,rcEvent.top,rcEvent.right,rcEvent.bottom);			 
}

/*text lines up with lhs of window*/
void PHScroll::DoPaint(CDCHandle dc)
{
	if (!phd._complete || _MemDC==NULL)
		return ;	

	if (dc.BitBlt(0, 0, phd._Width, phq._lCount * 41,*_MemDC, 0, 0, SRCCOPY)==0)
		PHTrace(Win32Error(), __LINE__, __FILE__);
}

void PHScroll::CreateScreenBuffer()
{
	RECT rPanel;
	rPanel.left = rPanel.top = 0;
	rPanel.right = phd._Width;
	rPanel.bottom = phq._lCount * 41;
	if (_MemDC)
		delete _MemDC;
	_MemDC = new CMemoryDC(GetDC(), rPanel);
	CBrush FillBrush;
	FillBrush.CreateSolidBrush(RGB(GetRValue(0x00FFFFFF), GetGValue(0x00FFFFFF), GetBValue(0x00FFFFFF)));//TODO: set background rather than draw this

	if (_MemDC->FillRect(&rPanel, FillBrush) == 0)
		PHTrace(Win32Error(), __LINE__, __FILE__);


	/*TODO:  tidy up logic*/
	/*Draw the processes*/
	for (map<long, RECT>::iterator it = phd._ProcessAreas.begin(); it != phd._ProcessAreas.end(); it++)
		DrawProcess(it->first, it->second);
}

void PHScroll::DrawProcess(long lID, RECT rcEvent)
{
	CFont hFont;
	
						 hFont.CreateFontIndirectA(&phd._font);
						 CFont oldFont = _MemDC->SelectFont(hFont);
	
	/*From the event map */
	COLORREF old_cr;
	old_cr = _MemDC->SetBkColor(0x00CA6002);
	
	CBrush ProcessBrush;
	ProcessBrush.CreateSolidBrush (RGB (GetRValue(0x00CA6002), GetGValue(0x00CA6002), GetBValue(0x00CA6002)));
	CBrush SelectedBrush;
	SelectedBrush.CreateSolidBrush (RGB(GetRValue(0x00CA6002),GetGValue(0x00CA6002),GetBValue(0x00606002)));
	COLORREF old_txt = _MemDC->SetTextColor(0x00FFFFFF);
		map<long,PHProcess>::iterator proc_it=		phq._Processes.find(lID);
		PHProcess pclProcess=proc_it->second;
					
		/* draw the process rectangle */
		
		if(phd._selected==lID)
		{
			if (_MemDC->FillRect(&rcEvent, SelectedBrush) == 0)
				PHTrace(Win32Error(),__LINE__,__FILE__);
		}
		else
		{
			if (_MemDC->FillRect(&rcEvent, ProcessBrush) == 0)
				PHTrace(Win32Error(),__LINE__,__FILE__);
		}
		
		DrawLines(_MemDC->m_hDC, rcEvent, lID);
			
		/*map<long,int>::iterator line_it=phq._Lines.find(lID);

		POINT pt;
		pt.x=rcEvent.left + 10;
		pt.y=(line_it->second*40)-20; TODO reinstate if causes problems*/
		map<long, long>::iterator crc_it=phq._PData.find(lID);
		map<long,string>::iterator image_it;
		if(crc_it!=phq._PData.end())
			image_it=phd._EXEImages.find(crc_it->second);

		string SystemName;
		if(image_it!=phd._EXEImages.end())
			SystemName=image_it->second;
		else
		{			
			map<long,string>::iterator qpit=phd.qrypaths.find(lID);
			if(qpit!=phd.qrypaths.end())
				SystemName=qpit->second;
		}
		/*Write system name at the correct location*/

		map<long,HICON>::iterator iconit;
		if(crc_it!=phq._PData.end())
			iconit=phd.icons.find(crc_it->second);
		bool drawn_icon = false;
		if(iconit!=phd.icons.end())
		{
			if(rcEvent.right-rcEvent.left>32)
			{
				HICON hIcon=iconit->second;
				if (hIcon != NULL)
				{
					POINT pt;
					pt.x = rcEvent.left + 4;
					pt.y = rcEvent.top + 4;
					SIZE sz;
					sz.cx = 32;
					sz.cy = 32;
					if (_MemDC->DrawIconEx(pt, hIcon, sz) == 0)
						PHTrace(Win32Error(), __LINE__, __FILE__);
					else
						drawn_icon = true;
				}
			}
			
		}
	
		if(SystemName.length()>0 )
		{
			/*fit the text to the rectangle at a constant font size*/
			COLORREF sel_cr;
			if(phd._selected==lID)
				sel_cr = _MemDC->SetBkColor(0x00606002);
			if (drawn_icon)
				rcEvent.left += 37;
			else 
				rcEvent.left += 4;

			if (rcEvent.right - rcEvent.left>0)
				/*if (*/_MemDC->DrawText(SystemName.c_str(), static_cast<int>(SystemName.length()), &rcEvent, DT_LEFT | DT_VCENTER | DT_SINGLELINE); //== 0)
					//PHTrace(Win32Error(),__LINE__,__FILE__);
			
			if(phd._selected==lID)
				_MemDC->SetBkColor(sel_cr);
		}			
_MemDC->SetTextColor(old_txt);
	_MemDC->SetBkColor(old_cr);
	_MemDC->SelectFont(oldFont);
	hFont.DeleteObject();
	}/*end process*/

#include <filesystem>
#include <mutex>

extern mutex db_mutex;

void PHDisplay::ReadPaths()// base on paths.id avoids duplication 
{
	qrypaths.clear();
	lock_guard<mutex> sl(db_mutex);
	unsigned long count=0;
	sqlite3 *db=OpenDB();
	for(map<long,RECT>::iterator it=phd._ProcessAreas.begin(); it!=phd._ProcessAreas.end(); it++)
	{	
		count++;
		//::PostMessage (ph_instance._hWnd, UWM_DOWNLOAD_PROGRESS, count, _ProcessAreas.size() );
		sqlite3_stmt* stmt;
		
		long lID=it->first;
		string os="SELECT Paths.Directory,Paths.ID FROM Process JOIN Paths ON Process.PathID=Paths.ID WHERE Process.ID="+to_string(lID)+";";
		if(sqlite3_prepare(db,os.c_str(),-1,&stmt,NULL)!=SQLITE_OK)
			DBError(sqlite3_errmsg(db),__LINE__,__FILE__);
			
		if(sqlite3_step(stmt)== SQLITE_ROW )
		{
			string FileName=(char*)sqlite3_column_text(stmt,0);
			//if(FileName.length()==0)
			//	continue;
			qrypaths.insert(pair<long,string>(lID,FileName));

			long PathID= sqlite3_column_int(stmt,1);
			phq._PData.insert(pair<long,long>(lID,PathID));
			if ( filesystem::exists(FileName))
			{
			map<long,HICON>::iterator icon_it= icons.find(PathID);
			if(icon_it==icons.end())
			{				
					HICON hIcon;
					BOOLEAN LargeIcon = TRUE;
					ExtractIconEx(
					FileName.c_str(),
						0,
						LargeIcon ? &hIcon : NULL,
						!LargeIcon ? &hIcon : NULL,
						1
						);
							if(hIcon!=NULL)
								icons.insert(pair< long,HICON>(PathID,hIcon));							
			 }
		
			 map<long,string>::iterator ver_it = _EXEImages.find(PathID);
			 if(ver_it==_EXEImages.end())
			 {
				string Product,Description;
				GetVersionInfo(Product,Description,FileName);	 // PHacker 
				if(!Product.empty())// || !Description.empty())
				{
					_EXEImages.insert(pair< long,string>(PathID,Product));
				}
			 }
			}
		}//endif
		sqlite3_finalize(stmt);
			
	}//for
	sqlite3_close(db);	
}
