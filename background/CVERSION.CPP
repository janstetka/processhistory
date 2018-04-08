//
//                               CVersion
//                 A VERSIONINFO helper class by MainSoft sarl
//							version 1.3	(Win32)
//
//
// This class provides easier and simplified access to the WINVER API functions.
// This API is, to say the least, clumsy. CVersion will make the process of
// retrieving VERSIONINFO information about a file much easier.
//
// Two important member functions are provided: CheckFile() and QueryValue().
//
// BOOL CVersion::CheckFile(LPCSTR lpszFilename, BOOL bShared, LPCSTR lpszAppDir);
//
// This function accepts 3 parameters:
//
// - a filename (without any path specification)
// - a boolean specifying whether the file is shared by several applications
//	 (defaults to FALSE)
// - an optional path specifying the directory where the file is installed
//   (not mandatory - defaults to NULL).
//
// If the function succeeds it returns TRUE and the other class member functions
// can then be used to access the VERSION INFORMATION located in the ressource
// area of the specified file:
//
// There's a CVersion constructor directly accepting a filename and calling
// CheckFile with default parameters.
//
// BOOL QueryValue(UINT ValueID, LPSTR* lplpszValue, UINT FAR* lpcb);
//
// Accepts 4 parameters:
//
// - an integer value specifying the type of the requested information
//   (look in version.h for idXXXXX constants)
//	Possible values are
//
// 		qvComments			
// 		qvCompanyName		
// 		qvFileDescription	
// 		qvFileVersion		
// 		qvInternalName		
// 		qvLegalCopyright		
// 		qvLegalTrademarks	
// 		qvOriginalFilename	
// 		qvPrivateBuild		
// 		qvProductName		
// 		qvProductVersion		
// 		qvSpecialBuild			
//
// - a pointer to a pointer to a character string. This pointer will receive
//   the address of the information string. Nothing is copied!
// - a pointer to an integer that will receive the length of the information string.
// - the language index to use (defaults to 0 since most VERSIONINFO resource 
//   support only one language).
//
// The function returns TRUE if it succeeds.
//
// The following functions should be called only after a successful call 
// to CheckFile().

// GetCurDir() returns a pointer to a character string representing the directory
// where the file has been found.
//
// Ditto for GetDestDir() but the character strings represents the recommended
// installation directory.
//
// GetFileFindFlags() returns an integer containing the return flags of the
// the VerFindFile API which is executed for you when you call CheckFile().
// (see version.h).
//
// GetFixedFileInfo returns a pointer to a VS_FIXEDFILEINFO structure. 
//
// GetStatus() returns TRUE when CheckFile has been called successfully.
//
// GetTranslationCount() returns the number of Language/CodePage pairs
// supported in this VERSIONINFO.
//
// GetLanguage returns the Language Number part of LangArray for a given
// index. The index must be lower than GetTranslationCount().
//
// GetCodePage returns the CodePage Number part of LangArray for a given
// index. The index must be lower than GetTranslationCount().
//
// GetLanguageString returns a string describing the language for a
// given index in LangArray.The index must be lower than GetTranslationCount().
//
// GetCodePageString returns a string describing the code page for a
// given index in LangArray.The index must be lower than GetTranslationCount().
//
// All the information provided by CVersion remains valid until you call
// CheckFile() again. You may check whether GetFindFileFlags() returns
// a value containing VVF_FILEINUSE which means that the checked file is
// currently in use: 
//		if (m_pVersion->GetFileFindFlags() & VFF_FILEINUSE)
//			file_is_currently_in_use
//
// Here is a typical use of CVersion

/*
	if (pVersion = new CVersion(Filename))
	{
		if (!pVersion->GetStatus())
		{
			VersionInfo_could_not_be_retrieved;
			return;
		}

		if (pVersion->GetFileFindFlags() & VFF_FILEINUSE)
		    Checked_file_is_in_use; // Doesn't prevent from accessing version info

		Display_CurDir(pVersion->GetCurDir());
		Display_Recommended_Dir(pVersion->GetDestDir());

		VS_FIXEDFILEINFO FAR* lpFixedFileInfo = pVersion->GetFixedFileInfo();
		if (lpFixedFileInfo)
		{
			You now have access to a valid FIXEDFILEINFO structure
			...
		}

		// Get comments
		if (pVersion->QueryValue(qvComments, &lpszComments, &nLen))
			Display_Comments(szComments);

		if (pVersion->QueryValue(qvCompanyName, &lpszCompanyName, &nLen))
		 	Display_CompanyName(szCompanyName);

		...

		// delete pVersion;
	}
*/

// Changes in version 1.2
//
// Added GetTrueFileVersionString and GetProductFileVersionString
// 		Both functions return a string representing the numerical 
//		version number (not the string version number)
//
// Modified CheckFile to circumvent a bug in the Windows 98
// implementation of VerFindFile.

// Changes in version 1.3
//
// Bug fix. Crash if CheckFile called with NULL pointer in lpszAppDir

//#include "StdAfx.h"
#include <stdio.h>
#include <tchar.h>
#include <windows.h>
#include <winver.h>
#include "cversion.h"


LPCSTR CVersion::m_ValueStrings[12] =
     {_T("Comments"),
      _T("CompanyName"),
      _T("FileDescription"),
      _T("FileVersion"),
      _T("InternalName"),
      _T("LegalCopyright"),
      _T("LegalTrademarks"),
      _T("OriginalFilename"),
      _T("PrivateBuild"),
      _T("ProductName"),
      _T("ProductVersion"),
      _T("SpecialBuild"),
     };

wchar_t szStringFileInfow[] = L"StringFileInfo";
wchar_t szVersionInfow[] = L"VS_VERSION_INFO";
char 	szStringFileInfoa[] = "StringFileInfo";
char 	szVersionInfoa[] = "VS_VERSION_INFO";

/////////////////////////////////////////////////////////////////////////////
// CVersion

CVersion::CVersion()
{
	Init();
}

CVersion::CVersion(LPCSTR lpszFilename)
{
	Init();
	CheckFile(lpszFilename);
}

void CVersion::Init()
{
	m_uCurDirLen = sizeof(m_szCurDir);
	m_uDestDirLen = sizeof(m_szDestDir);
	m_szCurDir[0] = 0;
	m_bStatus = FALSE;
	m_lpvData = NULL;
	m_uFixedFileInfoLen = 0;
	m_uLangArrayLen = 0;
	m_uFileFlags = 0;
	m_lpFixedFileInfo = NULL;
	m_lpdwLangArray = NULL;
}

CVersion::~CVersion()
{
	if (m_lpvData != NULL)
		delete m_lpvData;
}

/////////////////////////////////////////////////////////////////////////////
// Implementation

BOOL CVersion::CheckFile(LPCSTR lpszFilename, BOOL bShared, LPCSTR lpszAppDir)
{
	m_bStatus = FALSE;

	WIN32_FIND_DATA FindFileData;
	HANDLE	hFind;
	TCHAR	szFullName[MAX_PATH];

	if ((hFind = FindFirstFile(lpszFilename, &FindFileData)) == INVALID_HANDLE_VALUE)
	// We didn't get a full pathname or file doesn't exist - use VerFindFile)
	{
		// These variables must be initialized, otherwise VerFindFile will fail
		m_szCurDir[0] = '\0';
		m_szDestDir[0] = '\0';
		m_uCurDirLen = sizeof(m_szCurDir) - 1;
		m_uDestDirLen = sizeof(m_szDestDir) - 1;

		// Locate the specified file if possible
		m_uFileFlags = VerFindFile(bShared ? VFFF_ISSHAREDFILE : 0,
								   (LPSTR) lpszFilename,
								   NULL,
								   (LPSTR) lpszAppDir,
								   m_szCurDir,
								   &m_uCurDirLen,
								   m_szDestDir,
								   &m_uDestDirLen);
		
		if ((lpszAppDir != NULL) && (_tcslen(m_szCurDir) == 0)) // The file has not been found
			_tcscpy(m_szCurDir, lpszAppDir);
		
		if ((lpszAppDir != NULL) && (m_szCurDir[_tcslen(m_szCurDir) - 1] != '\\'))
			_tcscat(m_szCurDir, "\\"); // Concatenate directory and filename
		_tcscat(_tcscpy(szFullName, m_szCurDir), lpszFilename);
	}
	else
	{
		FindClose(hFind);
		_tcscpy(szFullName, lpszFilename);
	}

	DWORD	dwBufSize;
	DWORD	dwHandle;

	// Get the size of the version information block
	dwBufSize = GetFileVersionInfoSize(szFullName, (DWORD FAR*) &dwHandle);
	if (dwBufSize != NULL)
	{
		if (m_lpvData != NULL) // delete the existing block (previous call)
			delete m_lpvData;
		m_lpvData = new BYTE[dwBufSize]; // allocate space for the block
		// Get the information block itself
		if (GetFileVersionInfo(szFullName, dwHandle, dwBufSize, m_lpvData) != NULL)
		{
			m_bStatus = TRUE;
			// Get the pointer to the fixed fileinfo block
			if (!VerQueryValue(m_lpvData,
				               "\\",
				               (void FAR* FAR*) (&m_lpFixedFileInfo),
				               &m_uFixedFileInfoLen))
				m_lpFixedFileInfo = NULL;
			// Get the pointer to the translation information block
			if (!VerQueryValue(m_lpvData,
				               "\\VarFileInfo\\Translation",
				               (void FAR* FAR*) (&m_lpdwLangArray),
				               &m_uLangArrayLen))
				m_lpdwLangArray = NULL;

			// This code is to circumvent a problem with malformed
			// resources where the listed language id are not
			// reflected in the actual blocks
			// Moreover, the SDK documentation is not correct. GetFileVersionInfo 
			// does not always contain UNICODE data. If the data is ANSI the wType
			// field is always missing in all structures. We have to check this
			// UNICODE : szVersionInfow present at offset 6
			// ANSI : szVersionInfoa present at offset 4

			UINT nOffset;
			BOOL bIsUnicode = (_stricmp(szVersionInfoa, ((char*) m_lpvData) + 4 /* 4 bytes */) != 0)
							  &&
							  (_wcsicmp(szVersionInfow, ((wchar_t*) m_lpvData) + 3 /* 6 bytes */) == 0); 


			// Compute offset of first LangId/Codepage UNICODE string
			int nWords 	= bIsUnicode ? 3 : 2;
			int nSize  	= bIsUnicode ? sizeof(wchar_t) : sizeof(char);
			int nLen 	= (bIsUnicode ? wcslen(szVersionInfow) : strlen(szVersionInfoa)) * nSize;

			nOffset = nLen
					  + nSize						// skip terminating 0
					  + (sizeof (WORD) * nWords);	// skip 2/3 WORDS at the beginning of VS_VERSIONINFO structure
			// Adjust to 32-bit boundary
			while (nOffset % 4 != 0)
				nOffset++;
			// Get the length of the Value member in second WORD of 
			// VS_VERSIONINFO structure. Skip Value member
			nOffset += *(((WORD*) m_lpvData) + 1);
			// Adjust to 32-bit boundary again
			while (nOffset % 4 != 0)
				nOffset++;
			// Now jump to first langid string
			nOffset += (sizeof (WORD) * nWords);			// Skip 2/3 WORDS at the beginning of StringFileInfo structure */
			// Check for the presence of the ANSI / UNICODE key 
			if ((bIsUnicode && (wcsicmp(szStringFileInfow, (wchar_t*) (m_lpvData + nOffset)) != 0))
				||
				(!bIsUnicode && (_tcsicmp(szStringFileInfoa, (char*) (m_lpvData + nOffset)) != 0)))
				nOffset = 0;
			else
			{
				if (bIsUnicode)	
					nOffset += (wcslen(szStringFileInfow) * nSize); 
				else
					nOffset += (_tcslen(szStringFileInfoa) * nSize);

				nOffset += nSize;

				// Adjust to 32-bit boundary again
				while (nOffset % 4 != 0)
					nOffset++;
							   
				nOffset += (sizeof (WORD) * nWords);	// Skip 3 WORDS at the beginning of StringTable structure */
			}
  					  
			if (nOffset != 0)
			{
				if (bIsUnicode)
				{
#ifdef _UNICODE
					wcscpy(m_szFirstLanguage, (wchar_t*) (m_lpvData + nOffset));
#else
					wcstombs(m_szFirstLanguage, (wchar_t*) (m_lpvData + nOffset), sizeof(m_szFirstLanguage));
#endif
				}
				else
#ifdef _UNICODE
					mbstowcs(m_szFirstLanguage, (TCHAR*) (m_lpvData + nOffset), INT_MAX);
#else				
					strcpy(m_szFirstLanguage, (TCHAR*) (m_lpvData + nOffset));
#endif					
			}
			else
				m_szFirstLanguage[0] = (TCHAR) '\0';
		}
		else // failed ** free the allocated memory
		{
			delete m_lpvData;
			m_lpvData = NULL;
		}
	}

	return m_bStatus;
}

BOOL CVersion::GetTrueFileVersionString(LPSTR lpszBuf, UINT nMaxLen)
{
	if (m_lpFixedFileInfo && (lpszBuf) &&(nMaxLen >= 19))
	{
		wsprintf(lpszBuf,
				 "%d.%d.%d.%d", 
				 HIWORD(m_lpFixedFileInfo->dwFileVersionMS),
				 LOWORD(m_lpFixedFileInfo->dwFileVersionMS),
				 HIWORD(m_lpFixedFileInfo->dwFileVersionLS),
				 LOWORD(m_lpFixedFileInfo->dwFileVersionLS));
		return TRUE;
	}
	else
		return FALSE;
}	

BOOL CVersion::GetTrueProductVersionString(LPSTR lpszBuf, UINT nMaxLen)
{
	if (m_lpFixedFileInfo && (lpszBuf) &&(nMaxLen >= 19))
	{
		wsprintf(lpszBuf,
				 "%d.%d.%d.%d", 
				 HIWORD(m_lpFixedFileInfo->dwProductVersionMS),
				 LOWORD(m_lpFixedFileInfo->dwProductVersionMS),
				 HIWORD(m_lpFixedFileInfo->dwProductVersionLS),
				 LOWORD(m_lpFixedFileInfo->dwProductVersionLS));
		return TRUE;
	}
	else
		return FALSE;
}

BOOL CVersion::QueryValue(UINT ValueID, LPSTR* lplpszValue, UINT FAR* lpcb, UINT nLangIndex)
// Return a versioninfo string value for a given LangArray index
// Look in version.h for valid ValueId's
{
	BOOL bResult;

	if (!m_bStatus)
		return FALSE;
	if ((m_bStatus) && (m_lpdwLangArray))
	{
		TCHAR szBuf[256];

		// Prepare the request string 
		wsprintf(szBuf,
		         _T("\\StringFileInfo\\%04x%04x\\%s"),
		         LOWORD(m_lpdwLangArray[nLangIndex]), HIWORD(m_lpdwLangArray[nLangIndex]),
		         (LPSTR) m_ValueStrings[ValueID]);

		if (!(bResult = VerQueryValue(m_lpvData, szBuf, (void FAR* FAR*) lplpszValue, lpcb))
				&&
			 (m_szFirstLanguage[0] != (TCHAR) '\0'))
		{
			wsprintf(szBuf, _T("\\StringFileInfo\\%s\\%s"), m_szFirstLanguage, (LPSTR) m_ValueStrings[ValueID]);
			return VerQueryValue(m_lpvData, szBuf, (void FAR* FAR*) lplpszValue, lpcb);
			// lplpszValue now points to a pointer to the information string           
		}
		else
			return bResult;
	}
	else
		return FALSE;
}

BOOL CVersion::GetLanguageString(UINT nIndex, LPSTR lpszLanguage, UINT nLen)
// Return the string describing the corresponding language in LangArray
{
	if ((!m_bStatus) || (m_lpdwLangArray == NULL) || (nIndex >= GetTranslationCount()))
		return FALSE;

	return(VerLanguageName(LOWORD(*(m_lpdwLangArray + nIndex)), lpszLanguage, nLen) != 0);
}

BOOL CVersion::GetCodePageString(UINT nIndex, LPSTR lpszCodePage, UINT nLen)
// Return the string describing the corresponding code page in LangArray
{
	if ((!m_bStatus) || (nIndex < GetTranslationCount()))
		return FALSE;

	UINT nCodePage = GetCodePage(nIndex);

	// These strings should normally be stored as STRING TABLE resources having
	// a string id equal to the codepage number
	// I suggest to associate a .RC with this class and to load each relevant
	// string with a call to LoadString in the code below.
	switch (nCodePage)
	{
		case 0:
			_tcscpy(lpszCodePage, _T("7-bit ASCII"));
			break;
		case 932:
			_tcscpy(lpszCodePage, _T("Windows, Japan (Shift - JIS X-0208)"));
			break;
		case 949:
			_tcscpy(lpszCodePage, _T("Windows, Korea (Shift - KSC 5601)"));
			break;
		case 950:
			_tcscpy(lpszCodePage, _T("Windows, Taiwan (GB5)"));
			break;
		case 1200:
			_tcscpy(lpszCodePage, _T("Unicode"));
			break;
		case 1250:
			_tcscpy(lpszCodePage, _T("Windows, Latin-2 (Eastern Europe)"));
			break;
		case 1251:
			_tcscpy(lpszCodePage, _T("Windows, Cyrillic"));
			break;
		case 1252:
			_tcscpy(lpszCodePage, _T("Windows, Multilingual"));
			break;
		case 1253:
			_tcscpy(lpszCodePage, _T("Windows, Greek"));
			break;
		case 1254:
			_tcscpy(lpszCodePage, _T("Windows, Turkish"));
			break;
		case 1255:
			_tcscpy(lpszCodePage, _T("Windows, Hebrew"));
			break;
		case 1256:
			_tcscpy(lpszCodePage, _T("Windows, Arabic"));
			break;
		default:
			return(FALSE);
			break;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
