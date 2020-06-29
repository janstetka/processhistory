#if defined (_WIN64)
#include "ph.h"	
#include "..\..\ProcessHacker\include\phappres.h"
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
// DPCs, Interrupts and System Idle Process are not real.
// Non-"real" processes can never be opened.
#define PH_IS_REAL_PROCESS_ID(ProcessId) ((LONG_PTR)(ProcessId) > 0)
PWSTR PHackGetImageFile(int ProcessId,HANDLE h )
{
PROCESS_EXTENDED_BASIC_INFORMATION basicInfo;
    if (h)
    {
        

        if (NT_SUCCESS(PhGetProcessExtendedBasicInformation(h, &basicInfo)))
        {
            //ProcessItem->IsProtectedProcess = basicInfo.IsProtectedProcess;
           // ProcessItem->IsSecureProcess = basicInfo.IsSecureProcess;
          //  ProcessItem->IsSubsystemProcess = basicInfo.IsSubsystemProcess;
          //  ProcessItem->IsWow64 = basicInfo.IsWow64Process;
          //  ProcessItem->IsWow64Valid = TRUE;
        }
    }
		//NTSTATUS status;
//PPH_STRING fileName2; //harddisk format

// If we're dealing with System (PID 4), we need to get the
  // kernel file name. Otherwise, get the image file name. (wj32)

    if (ProcessId != SYSTEM_PROCESS_ID)
    {
        if (PH_IS_REAL_PROCESS_ID(ProcessId))
        {
            PPH_STRING fileName = NULL;
            PPH_STRING fileNameWin32 = NULL;

            if (NT_SUCCESS(PhGetProcessImageFileNameByProcessId(ProcessId, &fileName)))
            {
                // fileName2 = fileName;
            }

            if (h && !basicInfo.IsSubsystemProcess)
            {
                PhGetProcessImageFileNameWin32(h, &fileNameWin32); // PhGetProcessImageFileName (dmex)
            }


            if (fileName && !fileNameWin32)
            {
                fileNameWin32 = PhGetFileName(fileName);
            }
            if (fileNameWin32)
            {
                wchar_t* retval = calloc((ULONG)fileNameWin32->Length, sizeof(wchar_t));

                wcsncpy(retval, PhpGetStringOrNa(fileNameWin32), (ULONG)fileNameWin32->Length);

                PhDereferenceObject(fileNameWin32);
                if (fileName)
                    PhDereferenceObject(fileName);
                return retval;
            }
else
return 0;
        }
    }
}


PSYSTEM_PROCESS_INFORMATION process;
PVOID processes;

long ProcessHackerInitialGetProcess()
{

    //PVOID processes;
    //PSYSTEM_PROCESS_INFORMATION process;InheritedFromUniqueProcessId;could use this
	if (!NT_SUCCESS(PhEnumProcesses(&processes)))
        return -1;
		
	process = PH_FIRST_PROCESS(processes);
	if (process)
		return (long)process->UniqueProcessId;
	else
		return -1;
}

long ProcessHackerGetNextProcess()
{
	process = PH_NEXT_PROCESS(process);
	if (process)
		return (long)process->UniqueProcessId;
	else
		return -1;
}
void ProcessHackerCleanUp()
{
	PhFree(processes);
}

VOID PhpEnablePrivileges(
    VOID
)
{
    HANDLE tokenHandle;

    if (NT_SUCCESS(PhOpenProcessToken(
        NtCurrentProcess(),
        TOKEN_ADJUST_PRIVILEGES,
        &tokenHandle
    )))
    {
        CHAR privilegesBuffer[FIELD_OFFSET(TOKEN_PRIVILEGES, Privileges) + sizeof(LUID_AND_ATTRIBUTES) * 9];
        PTOKEN_PRIVILEGES privileges;
        ULONG i;

        privileges = (PTOKEN_PRIVILEGES)privilegesBuffer;
        privileges->PrivilegeCount = 9;

        for (i = 0; i < privileges->PrivilegeCount; i++)
        {
            privileges->Privileges[i].Attributes = SE_PRIVILEGE_ENABLED;
            privileges->Privileges[i].Luid.HighPart = 0;
        }

        privileges->Privileges[0].Luid.LowPart = SE_DEBUG_PRIVILEGE;
        privileges->Privileges[1].Luid.LowPart = SE_INC_BASE_PRIORITY_PRIVILEGE;
        privileges->Privileges[2].Luid.LowPart = SE_INC_WORKING_SET_PRIVILEGE;
        privileges->Privileges[3].Luid.LowPart = SE_LOAD_DRIVER_PRIVILEGE;
        privileges->Privileges[4].Luid.LowPart = SE_PROF_SINGLE_PROCESS_PRIVILEGE;
        privileges->Privileges[5].Luid.LowPart = SE_BACKUP_PRIVILEGE;
        privileges->Privileges[6].Luid.LowPart = SE_RESTORE_PRIVILEGE;
        privileges->Privileges[7].Luid.LowPart = SE_SHUTDOWN_PRIVILEGE;
        privileges->Privileges[8].Luid.LowPart = SE_TAKE_OWNERSHIP_PRIVILEGE;

        NtAdjustPrivilegesToken(
            tokenHandle,
            FALSE,
            privileges,
            0,
            NULL,
            NULL
        );

        NtClose(tokenHandle);
    }
}
BOOLEAN PHackerGetVersionInfo(PWSTR FileNameWin32, PWSTR *Product, PWSTR *Description, int *ProductLength,int *DescriptionLength)
{
    PH_IMAGE_VERSION_INFO VersionInfo;// = 0;
    // Version info.
    if (TRUE == PhInitializeImageVersionInfo(&VersionInfo, FileNameWin32)) {//, FALSE);

       
        
        if (VersionInfo.FileDescription) {
            *Description = calloc((ULONG)VersionInfo.FileDescription->Length, sizeof(wchar_t));
*DescriptionLength = VersionInfo.FileDescription->Length;
            wcsncpy(*Description, PhpGetStringOrNa(VersionInfo.FileDescription), (ULONG)VersionInfo.FileDescription->Length);
        }
        if (VersionInfo.ProductName) {
            *Product = calloc((ULONG)VersionInfo.ProductName->Length, sizeof(wchar_t));
 *ProductLength = VersionInfo.ProductName->Length;
            wcsncpy(*Product, PhpGetStringOrNa(VersionInfo.ProductName), (ULONG)VersionInfo.ProductName->Length);

        }
        PhDeleteImageVersionInfo(&VersionInfo);
        return TRUE;
    }
    else
        return FALSE;
}
PWSTR PHackerGetUser(HANDLE QueryHandle,long ProcessId)//use existing code apart from the bit that requires globalalloc
{ 
 if (
                QueryHandle &&
                ProcessId != SYSTEM_PROCESS_ID // System token can't be opened (dmex)
                )
            {
                HANDLE tokenHandle;

                if (NT_SUCCESS(PhOpenProcessToken(
                    QueryHandle,
                    TOKEN_QUERY,
                    &tokenHandle
                    )))
                {
                    PTOKEN_USER tokenUser;
                    TOKEN_ELEVATION_TYPE elevationType;
                    MANDATORY_LEVEL integrityLevel;
                    PWSTR integrityString;

                    // User
                    if (NT_SUCCESS(PhGetTokenUser(tokenHandle, &tokenUser)))
                    {
                        if (!RtlEqualSid(processItem->Sid, tokenUser->User.Sid))
                        {
                            PSID processSid;

                            // HACK (dmex)
                            processSid = processItem->Sid;
                            processItem->Sid = PhAllocateCopy(tokenUser->User.Sid, RtlLengthSid(tokenUser->User.Sid));
                            PhFree(processSid);

                            PhMoveReference(&processItem->UserName, PhpGetSidFullNameCachedSlow(processItem->Sid));

                            modified = TRUE;
                        }

                        PhFree(tokenUser);
                    }

                    // Elevation
                    if (NT_SUCCESS(PhGetTokenElevationType(tokenHandle, &elevationType)))
                    {
                        if (processItem->ElevationType != elevationType)
                        {
                            processItem->ElevationType = elevationType;
                            processItem->IsElevated = elevationType == TokenElevationTypeFull;
                            modified = TRUE;
                        }
                    }
					}
