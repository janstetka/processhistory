
#ifndef __CVersion__
#define __CVersion__

#include <winver.h>

#define qvComments				0
#define qvCompanyName			1
#define qvFileDescription		2
#define qvFileVersion			3
#define qvInternalName			4
#define qvLegalCopyright		5
#define qvLegalTrademarks		6
#define qvOriginalFilename		7
#define qvPrivateBuild			8
#define qvProductName			9
#define qvProductVersion		10
#define qvSpecialBuild			11

/////////////////////////////////////////////////////////////////////////////
// CVersion declaration

class CVersion
{
public:
	CVersion();
	CVersion(LPCSTR lpszFilename);
	~CVersion();
	void Init();

// Attributes
public:

private:
	BYTE*				m_lpvData;					// Pointer to VS_FIXEDFILEINFO structure
	TCHAR				m_szCurDir[MAX_PATH];		// Directory where file was found
	UINT 				m_uCurDirLen;				// Length of above directory
	TCHAR				m_szDestDir[MAX_PATH];		// Directory where Windows recommend to install
	UINT				m_uDestDirLen;				// Length of above directory
	UINT				m_uFileFlags;				// File flags
	BOOL				m_bStatus;
	VS_FIXEDFILEINFO*	m_lpFixedFileInfo;
	UINT				m_uFixedFileInfoLen;
	DWORD*				m_lpdwLangArray;
	UINT				m_uLangArrayLen;
	BYTE*				m_lpbStringFileInfo;
	static	LPCSTR		m_ValueStrings[12];
	TCHAR				m_szFirstLanguage[9];
	
// Implementation
public:
  BOOL CheckFile(const LPCSTR lpszFilename, BOOL bShared = FALSE, LPCSTR lpszAppDir = NULL);
  BOOL GetTrueFileVersionString(LPSTR lpszBuf, UINT nMaxLen);
  BOOL GetTrueProductVersionString(LPSTR lpszBuf, UINT nMaxLen);
  BOOL QueryValue(UINT ValueID, LPSTR* lplpszValue, UINT FAR* lpcb, UINT nLangIndex = 0);

  VS_FIXEDFILEINFO FAR* GetFixedFileInfo() {
	//ASSERT(m_bStatus);
	return m_lpFixedFileInfo;
  };

  LPSTR GetCurDir(){
	//ASSERT(m_bStatus);
	return (LPSTR) m_szCurDir;
  };

  LPSTR GetDestDir(){
	//ASSERT(m_bStatus);
	return (LPSTR) m_szDestDir;
  };

  UINT GetFileFindFlags(){
	//ASSERT(m_bStatus);
	return m_uFileFlags;
  };

  BOOL GetStatus(){
	return m_bStatus;
  }

  UINT GetTranslationCount(){
	//ASSERT(m_bStatus);
	return m_uLangArrayLen / 4;
  }

  UINT GetLanguage(UINT nIndex){
	//ASSERT(m_bStatus);
	//ASSERT(nIndex < GetTranslationCount());
	return LOWORD(m_lpdwLangArray[nIndex]);
  }

  UINT GetCodePage(UINT nIndex){
	//ASSERT(m_bStatus);
	//ASSERT(nIndex < GetTranslationCount());
	return HIWORD(m_lpdwLangArray[nIndex]);
  }

  BOOL GetLanguageString(UINT nIndex, LPSTR lpszLanguage, UINT nLen);
  BOOL GetCodePageString(UINT nIndex, LPSTR lpszCodePage, UINT nLen);
};

/////////////////////////////////////////////////////////////////////////////

#endif