#include <string>
#include <windows.h>
#include "..\PHQuery\screen.h"
#include "..\background\phacker.h"
#include "nowide\convert.hpp"

using namespace std;

void GetVersionInfo(string & Product, string & Description, string path)
{
	wchar_t* pProduct,*pDescription;
	SIZE_T ProductLength=0, DescriptionLength=0;
	if (TRUE == PHackerGetVersionInfo((PWSTR)nowide::widen(path.c_str()).c_str(), &pProduct, &pDescription,&ProductLength,&DescriptionLength))
	{
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