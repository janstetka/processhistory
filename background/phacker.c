#include "ph.h"	
#include "..\..\ProcessHacker\include\phappres.h"
#include "phacker.h"
#include "..\..\phlib\include\lsasup.h"
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
VOID ProcessHackerStart(HINSTANCE hi)
{
	NTSTATUS status=PhInitializePhLibEx(L"Process History",ULONG_MAX,hi,0,0);
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
PWSTR PHackGetImageFile(HANDLE ProcessId,HANDLE h )
{
PROCESS_EXTENDED_BASIC_INFORMATION basicInfo;
    if (h)
    {
        

        if (NT_SUCCESS(PhGetProcessExtendedBasicInformation(h, &basicInfo)))
        {

        }
    }

// If we're dealing with System (PID 4), we need to get the
  // kernel file name. Otherwise, get the image file name. (wj32)

    if (ProcessId != SYSTEM_PROCESS_ID)
    {
        if (PH_IS_REAL_PROCESS_ID(ProcessId))
        {
            PPH_STRING fileName = NULL;
            PPH_STRING fileNameWin32 = NULL;

            if (NT_SUCCESS(PhGetProcessImageFileNameByProcessId(ProcessId, &fileName)))//harddisk format
            {

            }

            if (h && !basicInfo.IsSubsystemProcess) // only works with a handle so not on other users processes unelevated
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
        }
    }
     return 0;
}


PSYSTEM_PROCESS_INFORMATION process;
PVOID processes;

VOID ProcessHackerInitialGetProcess(phqi* qi)
{

//InheritedFromUniqueProcessId;TODO 2020 could use this
	if (!NT_SUCCESS(PhEnumProcesses(&processes)))
        return ;
		
	process = PH_FIRST_PROCESS(processes);
	if (process)
	{
		qi->ID= process->UniqueProcessId;
        PhLargeIntegerToLocalSystemTime(&qi->st, &process->CreateTime);
        qi->parentID = process->InheritedFromUniqueProcessId;
		}
}

VOID ProcessHackerGetNextProcess(phqi* qi)
{
	process = PH_NEXT_PROCESS(process);
	if (process)
	{
		qi->ID= process->UniqueProcessId;
	PhLargeIntegerToLocalSystemTime(&qi->st, &process->CreateTime);
    qi->parentID = process->InheritedFromUniqueProcessId;
	}
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
/*        0xC0000262

            STATUS_DRIVER_ORDINAL_NOT_FOUND expected unelevated*/
        NTSTATUS nt=NtAdjustPrivilegesToken(
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
BOOLEAN PHackerGetVersionInfo(PWSTR FileNameWin32, PWSTR *Product, PWSTR *Description, SIZE_T *ProductLength, SIZE_T *DescriptionLength)//TODO 2020: make this more like calling a phacker function i.e. pass a reference to a structure
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
PWSTR PHackerGetUser(HANDLE QueryHandle, HANDLE ProcessId)//use existing code apart from the bit that requires globalalloc
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

            PPH_STRING UserName=NULL;

            // User

                    UserName=PhGetTokenUserString(tokenHandle,TRUE);

                if (UserName )
                {
                    wchar_t* retval = calloc((ULONG)UserName->Length, sizeof(wchar_t));

                    wcsncpy(retval, PhpGetStringOrNa(UserName), (ULONG)UserName->Length);

                    PhDereferenceObject(UserName);
                    return retval;
                }
            //}

            // Elevation
           /* if (NT_SUCCESS(PhGetTokenElevationType(tokenHandle, &elevationType)))
            {
                if (processItem->ElevationType != elevationType)
                {
                    processItem->ElevationType = elevationType;
                    processItem->IsElevated = elevationType == TokenElevationTypeFull;
                //    modified = TRUE;
                }
            }*/
        }
    }
    return 0;
}

SYSTEMTIME PHackerGetProcessTimesE(HANDLE h)
{
    KERNEL_USER_TIMES times;
    SYSTEMTIME exit;
    if (NT_SUCCESS(PhGetProcessTimes(h, &times)))
        PhLargeIntegerToLocalSystemTime(&exit, &times.ExitTime);
        return exit;    
}

NTSTATUS PHackerOpenProcess(HANDLE PID, ACCESS_MASK DesiredAccess,PHANDLE ph)
{
    return PhOpenProcess(ph, DesiredAccess, PID);
}
