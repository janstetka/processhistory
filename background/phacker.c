#if defined (_WIN64)
#include "c:\processhacker-2.33-src\phlib\include\ph.h"	
#include "C:\processhacker-2.33-src\ProcessHacker\include\phappres.h"
#else
#include "c:\processhacker-2.28-src\phlib\include\ph.h"	
#include "C:\processhacker-2.28-src\ProcessHacker\include\phappres.h"
#endif
#include "phacker.h"
#include <wchar.h>


PWSTR PhpGetStringOrNa(
    __in PPH_STRING String
    )
{
    if (String)
        return String->Buffer;
    else
        return L"N/A";
}
PWSTR ProcessHackerStart()
{
	PhInitializePhLib();
	PPH_STRING appName;

#if (PHAPP_VERSION_REVISION != 0)
	appName = PhFormatString(
		L"Process Hacker %u.%u (r%u)",
		PHAPP_VERSION_MAJOR,
		PHAPP_VERSION_MINOR,
		PHAPP_VERSION_REVISION
		);
#else
	appName = PhFormatString(
		L"Process Hacker %u.%u",
		PHAPP_VERSION_MAJOR,
		PHAPP_VERSION_MINOR
		);
#endif
	wchar_t * retval = calloc((ULONG)appName->Length, sizeof(wchar_t));

	wcsncpy(retval, PhpGetStringOrNa(appName), (ULONG)appName->Length);
	PhDereferenceObject(appName);
	return retval;
}

PWSTR PHiGetCommandLine(HANDLE h)
{
	{
		NTSTATUS status;
            BOOLEAN isPosix = FALSE;
            PPH_STRING commandLine;
            ULONG i;
          
                status = PhGetProcessCommandLine(h, &commandLine);

                if (NT_SUCCESS(status))
                {
                    // Some command lines (e.g. from taskeng.exe) have nulls in them.
                    // Since Windows can't display them, we'll replace them with
                    // spaces.
                    for (i = 0; i < (ULONG)commandLine->Length / 2; i++)
                    {
                        if (commandLine->Buffer[i] == 0)
                            commandLine->Buffer[i] = ' ';
                    }
                }
           

			if (NT_SUCCESS(status))
			{

				wchar_t * retval = calloc((ULONG)commandLine->Length, sizeof(wchar_t));

				wcsncpy(retval, PhpGetStringOrNa(commandLine), (ULONG)commandLine->Length);
				PhDereferenceObject(commandLine);
				return retval;
			}
			else
				return 0;
	}
}

/*PWSTR PHackGetImageFile(HANDLE h)
{
		NTSTATUS status;
PPH_STRING fileName;
if (WINDOWS_HAS_IMAGE_FILE_NAME_BY_PROCESS_ID)
                status = PhGetProcessImageFileNameByProcessId(hProcessItem->ProcessId, &fileName);
            else if (h)
                status = PhGetProcessImageFileName(h, &fileName);
				return PhpGetStringOrNa(fileName);
}

#ifdef __cplusplus
}
#endif*/