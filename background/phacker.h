#ifdef __cplusplus
extern "C"
{
#endif
typedef	struct phqi {
	HANDLE ID;
	SYSTEMTIME st;
	HANDLE parentID;
} phqi;
PWSTR PHiGetCommandLine(HANDLE h);
VOID ProcessHackerStart(HINSTANCE hi);
//void Dereference(PPH_STRING str);
NTSTATUS ProcessHackerInitialGetProcess(phqi *qi); //long& PID, SYSTEMTIME& creation, long& parentID);
NTSTATUS ProcessHackerGetNextProcess(phqi *qi); //long& PID, SYSTEMTIME& creation, long& parentID);
void ProcessHackerCleanUp();
PWSTR PHackGetImageFile(HANDLE ProcessId, HANDLE h);
VOID PhpEnablePrivileges(VOID);
BOOLEAN PHackerGetVersionInfo(PWSTR FileNameWin32, PWSTR* Product, PWSTR* Description, SIZE_T *ProductLength, SIZE_T *DescriptionLength);
PWSTR PHackerGetUser(HANDLE QueryHandle, HANDLE ProcessId);
//SYSTEMTIME PHackerGetProcessTimesC(HANDLE h);
SYSTEMTIME PHackerGetProcessTimesE(HANDLE h);
NTSTATUS PHackerOpenProcess(HANDLE PID, ACCESS_MASK DesiredAccess,PHANDLE ph);

#ifdef __cplusplus
}
#endif
