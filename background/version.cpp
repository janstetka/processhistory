#include <string>
#include <windows.h>
//#include "cversion.h"
#include "..\PHQuery\screen.h"
#include "..\background\phacker.h"
#include "nowide\convert.hpp"

using namespace std;

void GetVersionInfo(string & Product, string & Description, string path)
{
	/*LPSTR p=0;//= new char[500];
	LPSTR p2=0;//=new char[500];
	unsigned int  nLen;

	CVersion pVersion(path.c_str());

		//if (pVersion = new CVersion){
	
			//VS_FIXEDFILEINFO FAR* lpFixedFileInfo = pVersion.GetFixedFileInfo();
			//if (lpFixedFileInfo)
			//{
			//	You now have access to a valid FIXEDFILEINFO structure
			//	...
			//}

			if (!pVersion.QueryValue(qvProductName,&p, &nLen))
				p="Module not found, or version info not found";
			else
				Product=p;
			if (!pVersion.QueryValue(qvFileDescription	,&p2,&nLen))
			{	
				p2="Module not found, or version info not found";
			
			}
			else
				Description=p2;		*/

	wchar_t* pProduct,*pDescription;
	SIZE_T ProductLength=0, DescriptionLength=0;
	if (TRUE == PHackerGetVersionInfo((PWSTR)nowide::widen(path.c_str()).c_str(), &pProduct, &pDescription,&ProductLength,&DescriptionLength))
	{
		//CloseHandle(hProcess);
		if (ProductLength>0 )
		{
			Product = nowide::narrow(pProduct);
			free(pProduct);
		}
		if (DescriptionLength>0)
		{
			Description = nowide::narrow(pDescription);
			free(pDescription);
		}
	}
}