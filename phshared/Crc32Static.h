#ifndef _CRC32STATIC_H_
#define _CRC32STATIC_H_

#include "Common.h"

class CCrc32Static
{
public:
	CCrc32Static();
	virtual ~CCrc32Static();
#if defined (_WIN64)
static DWORD FileCrc32Win32(LPCTSTR szFilename, DWORD &dwCrc32);
protected:
static inline void CalcCrc32(const BYTE byte, DWORD &dwCrc32);
#else
public:
static	DWORD FileCrc32Assembly(LPCTSTR szFilename, DWORD &dwCrc32);
#endif
protected:
	static bool GetFileSizeQW(const HANDLE hFile, QWORD &qwSize);
	

	static DWORD s_arrdwCrc32Table[256];
};

#endif
