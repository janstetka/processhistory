#define _WIN32_DCOM
#include <atlbase.h>
#include <Wbemidl.h>
#include <sstream>
#include <comutil.h>
#include "..\ProcessInfo.h"
//#include <cassert>

using namespace std;

#pragma comment(lib, "comsupp.lib")
#pragma comment(lib, "comsuppw.lib")


IWbemServices*	m_pSvc;
void SetupCOM()
{
	 //once in the whole process

	m_pSvc=NULL;
	IWbemLocator *pLoc;

    HRESULT hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
            IID_IWbemLocator, (LPVOID *) &pLoc);
	 if (hres != S_OK)
    {

	    PHCOMError(hres,"CoCreateInstance on IWbemLocator failed",__LINE__,__FILE__);
		CoUninitialize();
        return ;
    }	
	//IWbemServices*	m_pSvc=NULL;

	/* Release any existing connection*/
	if ( m_pSvc )
	{
		m_pSvc->Release();
		m_pSvc = NULL;
	}

	/* Connect to the server*/
	hres = pLoc->ConnectServer(
		_bstr_t(L"root\\CIMV2"),
            NULL,
            NULL,
            NULL,
            0,
            0,
            0,
            &m_pSvc
            );

	pLoc->Release();

    if (FAILED(hres))
    {
	    PHCOMError(hres,"Connect Server failed",__LINE__,__FILE__);
		m_pSvc->Release();
		return;
	}    /* Set the proxy so that impersonation of the client occurs.*/
    hres = CoSetProxyBlanket(m_pSvc,
       RPC_C_AUTHN_WINNT,
       RPC_C_AUTHZ_NONE,
       NULL,
       RPC_C_AUTHN_LEVEL_CALL,
       RPC_C_IMP_LEVEL_IMPERSONATE,
       NULL,
       EOAC_NONE
    );
	if(hres!=S_OK)
		{

	    PHCOMError(hres,"CosetProxyBlanket failed",__LINE__,__FILE__);
		m_pSvc->Release();
		return;
	}
}
#include <boost/thread/mutex.hpp>
using namespace boost;
mutex cl_mutex;
void WMIGetProcess(long PID,string & cl,string col,string table)
{

	mutex::scoped_lock cl_lock(cl_mutex);
	CComBSTR Language(L"WQL");
	CComBSTR Query(L"");
	ostringstream os;
	os<<"SELECT ";
	os<<col;
	os<<" FROM "<<
	table;
	if(PID!=-1)
	{
		os<<" WHERE ProcessId=";
		os<<PID;
	}
	Query=os.str().c_str();
	IEnumWbemClassObject* wco=0;
	HRESULT
		hres=m_pSvc->ExecQuery(Language,Query,0,0,&wco);
	if(hres!=S_OK)
		{

	    PHCOMError(hres,"ExecQuery failed",__LINE__,__FILE__);
		m_pSvc->Release();
		return;
	}
	IWbemClassObject*    apObj[10];
	ULONG	uReturned;
	 hres = wco->Next( WBEM_INFINITE, 10, apObj, &uReturned );
	
	if ( SUCCEEDED( hres ) )
        {
		CComVariant var;
		apObj[0]->Get(CComBSTR(_com_util::ConvertStringToBSTR(col.c_str())),0,&var,0,0);
		cl=_com_util::ConvertBSTRToString(var.bstrVal);
	
		for ( ULONG n = 0; n < uReturned; n++ )
			apObj[n]->Release();
        }
	wco->Release();
}
void ReleaseCOM()
{
	m_pSvc->Release();
	
}