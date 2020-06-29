#ifdef __cplusplus
extern "C"
{
#endif
PWSTR PHiGetCommandLine(HANDLE h);
PWSTR ProcessHackerStart();
//void Dereference(PPH_STRING str);
long ProcessHackerInitialGetProcess();
long ProcessHackerGetNextProcess();
void ProcessHackerCleanUp();
PWSTR PHackGetImageFile(int ProcessId, HANDLE h);
VOID PhpEnablePrivileges(VOID);
BOOLEAN PHackerGetVersionInfo(PWSTR FileNameWin32, PWSTR* Product, PWSTR* Description, int *ProductLength, int *DescriptionLength);
#ifdef __cplusplus
}
#endif
