#include <Windows.h>
#include "icons.h"
//#include "../phshared/phshared.h"
//extern "C" {

#define WIDTHBYTES(bits)      ((((bits) + 31)>>5)<<2)

LPTSTR	g_lpID;
BOOL first;
// Resource Position info - size and offset of a resource in a file
typedef struct
{
    DWORD	dwBytes;
    DWORD	dwOffset;
} RESOURCEPOSINFO, *LPRESOURCEPOSINFO;

LPICONRESOURCE ReadIconFromEXEFile( LPCTSTR szFileName );
BOOL AdjustIconImagePointers( LPICONIMAGE lpImage );
DWORD BytesPerLine( LPBITMAPINFOHEADER lpBMIH );
WORD PaletteSize( LPSTR lpbi );
LPSTR FindDIBBits( LPSTR lpbi );
WORD DIBNumColors( LPSTR lpbi );
BOOL CALLBACK EnumResNameProc( HANDLE  hModule, LPCTSTR  lpszType, LPTSTR  lpszName, LONG_PTR  lParam );
BOOL WriteICOHeader( HANDLE hFile, UINT nNumEntries );
DWORD CalculateImageOffset( LPICONRESOURCE lpIR, UINT nIndex );
/****************************************************************************
*
*     FUNCTION: WriteIconToICOFile
*
*     PURPOSE:  Writes the icon resource data to an ICO file
*
*     PARAMS:   LPICONRESOURCE lpIR       - pointer to icon resource
*               LPCTSTR        szFileName - name for the ICO file
*
*     RETURNS:  BOOL - TRUE for success, FALSE for failure
*
* History:
*                July '95 - Created
*
\****************************************************************************/
BOOL WriteIconToICOFile( LPICONRESOURCE lpIR, LPCTSTR szFileName )
{
    HANDLE    	hFile;
    UINT        i;
    DWORD    	dwBytesWritten;

    // open the file
    if( (hFile = CreateFile( szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL )) == INVALID_HANDLE_VALUE )
    {
       // MessageBox(  "Error Opening File for Writing"+CString( szFileName));
        return FALSE;
    }
    // Write the header
    if( ! WriteICOHeader( hFile, lpIR->nNumImages ) )
    {
       // MessageBox(  "Error Writing ICO File"+CString( szFileName) );
        CloseHandle( hFile );
        return FALSE;
    }
    // Write the ICONDIRENTRY's
    for( i=0; i<lpIR->nNumImages; i++ )
    {
        ICONDIRENTRY    ide;

        // Convert internal format to ICONDIRENTRY
        ide.bWidth = lpIR->IconImages[i].Width;
        ide.bHeight = lpIR->IconImages[i].Height;
        ide.bReserved = 0;
        ide.wPlanes = lpIR->IconImages[i].lpbi->bmiHeader.biPlanes;
        ide.wBitCount = lpIR->IconImages[i].lpbi->bmiHeader.biBitCount;
        if( (ide.wPlanes * ide.wBitCount) >= 8 )
            ide.bColorCount = 0;
        else
            ide.bColorCount = 1 << (ide.wPlanes * ide.wBitCount);
        ide.dwBytesInRes = lpIR->IconImages[i].dwNumBytes;
        ide.dwImageOffset = CalculateImageOffset( lpIR, i );
        // Write the ICONDIRENTRY out to disk
        if( ! WriteFile( hFile, &ide, sizeof( ICONDIRENTRY ), &dwBytesWritten, NULL ) )
            return FALSE;
        // Did we write a full ICONDIRENTRY ?
        if( dwBytesWritten != sizeof( ICONDIRENTRY ) )
            return FALSE;
    }
    // Write the image bits for each image
    for( i=0; i<lpIR->nNumImages; i++ )
    {
        DWORD dwTemp = lpIR->IconImages[i].lpbi->bmiHeader.biSizeImage;

        // Set the sizeimage member to zero
        lpIR->IconImages[i].lpbi->bmiHeader.biSizeImage = 0;
        // Write the image bits to file
        if( ! WriteFile( hFile, lpIR->IconImages[i].lpBits, lpIR->IconImages[i].dwNumBytes, &dwBytesWritten, NULL ) )
            return FALSE;
        if( dwBytesWritten != lpIR->IconImages[i].dwNumBytes )
            return FALSE;
        // set it back
        lpIR->IconImages[i].lpbi->bmiHeader.biSizeImage = dwTemp;
    }
    CloseHandle( hFile );
    return FALSE;
}
/* End WriteIconToICOFile() **************************************************/
/****************************************************************************
*
*     FUNCTION: ReadIconFromEXEFile
*
*     PURPOSE:  Load an Icon Resource from a DLL/EXE file
*
*     PARAMS:   LPCTSTR szFileName - name of DLL/EXE file
*
*     RETURNS:  LPICONRESOURCE - pointer to icon resource
*
* History:
*                July '95 - Created
*
\****************************************************************************/

typedef struct
{
    LPCTSTR    	szFileName;
    HINSTANCE	hInstance;
    LPTSTR    	lpID;
} EXEDLLICONINFO, *LPEXEDLLICONINFO;

LPICONRESOURCE ReadIconFromEXEFile( LPCTSTR szFileName )
{
    LPICONRESOURCE    	lpIR = NULL, lpNew = NULL;
    HINSTANCE        	hLibrary;
    LPTSTR            	lpID;
    EXEDLLICONINFO    	EDII;
        HRSRC        	hRsrc = NULL;
        HGLOBAL        	hGlobal = NULL;
        LPMEMICONDIR    lpIcon = NULL;
        UINT            i;
    // Load the DLL/EXE - NOTE: must be a 32bit EXE/DLL for this to work
    if( (hLibrary = LoadLibraryEx( szFileName, NULL, LOAD_LIBRARY_AS_DATAFILE )) == NULL )
    {
        // Failed to load - abort
        //MessageBox(  "Error Loading File - Choose a 32bit DLL or EXE"+CString( szFileName));
        return NULL;
    }
    // Store the info
    EDII.szFileName = szFileName;
    EDII.hInstance = hLibrary;

first=TRUE;
	EnumResourceNames( EDII.hInstance, RT_GROUP_ICON,(ENUMRESNAMEPROC) EnumResNameProc, (LPARAM)0 );



        // Find the group icon resource
        if( (hRsrc = FindResource( hLibrary, g_lpID, RT_GROUP_ICON )) == NULL )
        {
            FreeLibrary( hLibrary );
            return NULL;
        }
        if( (hGlobal = LoadResource( hLibrary, hRsrc )) == NULL )
        {
            FreeLibrary( hLibrary );
            return NULL;
        }
        if( (lpIcon = (LPMEMICONDIR)LockResource(hGlobal)) == NULL )
        {
            FreeLibrary( hLibrary );
            return NULL;
        }
        // Allocate enough memory for the images
        if( (lpIR =(LPICONRESOURCE)( malloc( sizeof(ICONRESOURCE) + ((lpIcon->idCount-1) * sizeof(ICONIMAGE)) ))) == NULL )
        {
//            MessageBox( hWndMain, "Error Allocating Memory", szFileName, MB_OK );
            FreeLibrary( hLibrary );
            return NULL;
        }
        // Fill in local struct members
        lpIR->nNumImages = lpIcon->idCount;
        lstrcpy( lpIR->szOriginalDLLFileName, szFileName );
        lstrcpy( lpIR->szOriginalICOFileName, "" );
        // Loop through the images
        for( i = 0; i < lpIR->nNumImages; i++ )
        {
            // Get the individual image
            if( (hRsrc = FindResource( hLibrary, MAKEINTRESOURCE(lpIcon->idEntries[i].nID), RT_ICON )) == NULL )
            {
                free( lpIR );
                FreeLibrary( hLibrary );
                return NULL;
            }
            if( (hGlobal = LoadResource( hLibrary, hRsrc )) == NULL )
            {
                free( lpIR );
                FreeLibrary( hLibrary );
                return NULL;
            }
            // Store a copy of the resource locally
            lpIR->IconImages[i].dwNumBytes = SizeofResource( hLibrary, hRsrc );
            lpIR->IconImages[i].lpBits =(LPBYTE) malloc( lpIR->IconImages[i].dwNumBytes );
            memcpy( lpIR->IconImages[i].lpBits, LockResource( hGlobal ), lpIR->IconImages[i].dwNumBytes );
            // Adjust internal pointers
            if( ! AdjustIconImagePointers( &(lpIR->IconImages[i]) ) )
            {
//                MessageBox( hWndMain, "Error Converting to Internal Format", szFileName, MB_OK );
                free( lpIR );
                FreeLibrary( hLibrary );
                return NULL;
            }
        //}
    }
    FreeLibrary( hLibrary );
    return lpIR;
}

/* End ReadIconFromEXEFile() ************************************************/
/****************************************************************************
*
*     FUNCTION: WriteICOHeader
*
*     PURPOSE:  Writes the header to an ICO file
*
*     PARAMS:   HANDLE hFile       - handle to the file
*               UINT   nNumEntries - Number of images in file
*
*     RETURNS:  BOOL - TRUE for success, FALSE for failure
*
* History:
*                July '95 - Created
*
\****************************************************************************/
BOOL WriteICOHeader( HANDLE hFile, UINT nNumEntries )
{
    WORD    Output;
    DWORD	dwBytesWritten;

    // Write 'reserved' WORD
    Output = 0;
    if( ! WriteFile( hFile, &Output, sizeof( WORD ), &dwBytesWritten, NULL ) )
        return FALSE;
    // Did we write a WORD?
    if( dwBytesWritten != sizeof( WORD ) )
        return FALSE;
    // Write 'type' WORD (1)
    Output = 1;
    if( ! WriteFile( hFile, &Output, sizeof( WORD ), &dwBytesWritten, NULL ) )
        return FALSE;
    // Did we write a WORD?
    if( dwBytesWritten != sizeof( WORD ) )
        return FALSE;
    // Write Number of Entries
    Output = (WORD)nNumEntries;
    if( ! WriteFile( hFile, &Output, sizeof( WORD ), &dwBytesWritten, NULL ) )
        return FALSE;
    // Did we write a WORD?
    if( dwBytesWritten != sizeof( WORD ) )
        return FALSE;
    return TRUE;
}
/* End WriteICOHeader() ****************************************************/
/****************************************************************************
*
*     FUNCTION: CalculateImageOffset
*
*     PURPOSE:  Calculates the file offset for an icon image
*
*     PARAMS:   LPICONRESOURCE lpIR   - pointer to icon resource
*               UINT           nIndex - which image?
*
*     RETURNS:  DWORD - the file offset for that image
*
* History:
*                July '95 - Created
*
\****************************************************************************/
DWORD CalculateImageOffset( LPICONRESOURCE lpIR, UINT nIndex )
{
    DWORD	dwSize;
    UINT    i;

    // Calculate the ICO header size
    dwSize = 3 * sizeof(WORD);
    // Add the ICONDIRENTRY's
    dwSize += lpIR->nNumImages * sizeof(ICONDIRENTRY);
    // Add the sizes of the previous images
    for(i=0;i<nIndex;i++)
        dwSize += lpIR->IconImages[i].dwNumBytes;
    // we're there - return the number
    return dwSize;
}
/* End CalculateImageOffset() ***********************************************/
/****************************************************************************
*
*     FUNCTION: AdjustIconImagePointers
*
*     PURPOSE:  Adjusts internal pointers in icon resource struct
*
*     PARAMS:   LPICONIMAGE lpImage - the resource to handle
*
*     RETURNS:  BOOL - TRUE for success, FALSE for failure
*
* History:
*                July '95 - Created
*
\****************************************************************************/
BOOL AdjustIconImagePointers( LPICONIMAGE lpImage )
{
    // Sanity check
    if( lpImage==NULL )
        return FALSE;
    // BITMAPINFO is at beginning of bits
    lpImage->lpbi = (LPBITMAPINFO)lpImage->lpBits;
    // Width - simple enough
    lpImage->Width = lpImage->lpbi->bmiHeader.biWidth;
    // Icons are stored in funky format where height is doubled - account for it
    lpImage->Height = (lpImage->lpbi->bmiHeader.biHeight)/2;
    // How many colors?
    lpImage->Colors = lpImage->lpbi->bmiHeader.biPlanes * lpImage->lpbi->bmiHeader.biBitCount;
    // XOR bits follow the header and color table
    lpImage->lpXOR =(LPBYTE) FindDIBBits((LPSTR)lpImage->lpbi);
    // AND bits follow the XOR bits
    lpImage->lpAND = lpImage->lpXOR + (lpImage->Height*BytesPerLine((LPBITMAPINFOHEADER)(lpImage->lpbi)));
    return TRUE;
}
/* End AdjustIconImagePointers() *******************************************/
/****************************************************************************
*
*     FUNCTION: FindDIBits
*
*     PURPOSE:  Locate the image bits in a CF_DIB format DIB.
*
*     PARAMS:   LPSTR lpbi - pointer to the CF_DIB memory block
*
*     RETURNS:  LPSTR - pointer to the image bits
*
* History:
*                July '95 - Copied <g>
*
\****************************************************************************/
LPSTR FindDIBBits( LPSTR lpbi )
{
      return ( lpbi + *(LPDWORD)lpbi + PaletteSize( lpbi ) );
}
/****************************************************************************
*
*     FUNCTION: BytesPerLine
*
*     PURPOSE:  Calculates the number of bytes in one scan line.
*
*     PARAMS:   LPBITMAPINFOHEADER lpBMIH - pointer to the BITMAPINFOHEADER
*                                           that begins the CF_DIB block
*
*     RETURNS:  DWORD - number of bytes in one scan line (DWORD aligned)
*
* History:
*                July '95 - Created
*
\****************************************************************************/
DWORD BytesPerLine( LPBITMAPINFOHEADER lpBMIH )
{
    return WIDTHBYTES(lpBMIH->biWidth * lpBMIH->biPlanes * lpBMIH->biBitCount);
}
/* End BytesPerLine() ********************************************************/
/****************************************************************************
*
*     FUNCTION: PaletteSize
*
*     PURPOSE:  Calculates the number of bytes in the color table.
*
*     PARAMS:   LPSTR lpbi - pointer to the CF_DIB memory block
*
*     RETURNS:  WORD - number of bytes in the color table
*
*
* History:
*                July '95 - Copied <g>
*
\****************************************************************************/
WORD PaletteSize( LPSTR lpbi )
{
    return ( DIBNumColors( lpbi ) * sizeof( RGBQUAD ) );
}
WORD DIBNumColors( LPSTR lpbi )
{
    WORD wBitCount;
    DWORD dwClrUsed;

    dwClrUsed = ((LPBITMAPINFOHEADER) lpbi)->biClrUsed;

    if (dwClrUsed)
        return (WORD) dwClrUsed;

    wBitCount = ((LPBITMAPINFOHEADER) lpbi)->biBitCount;

    switch (wBitCount)
    {
        case 1: return 2;
        case 4: return 16;
        case 8:	return 256;
        default:return 0;
    }
    return 0;
}
/****************************************************************************
*
*     FUNCTION: MyEnumProcedure
*
*     PURPOSE:  Callback for enumerating resources in a DLL/EXE
*
*     PARAMS:   HANDLE  hModule  - Handle of the module
*               LPCTSTR lpszType - Resource Type
*               LPTSTR  lpszName - Resource Name
*               LONG    lParam   - Handle of ListBox to add name to
*
*     RETURNS:  BOOL - TRUE to continue, FALSE to stop
*
* History:
*                July '95 - Created
*
\****************************************************************************/
BOOL CALLBACK EnumResNameProc( HANDLE  hModule, LPCTSTR  lpszType, LPTSTR  lpszName, LONG  lParam )	
{
    TCHAR	szBuffer[256];
    LONG    nIndex = LB_ERR;
    LPTSTR	lpID = NULL;

    // Name is from MAKEINTRESOURCE()
    if( HIWORD(lpszName) == 0 )
    {
        wsprintf( szBuffer, "Icon [%d]", (DWORD)lpszName );
        lpID = lpszName;
    }
    else
    {
        // Name is string
        lpID = strdup( lpszName );
        wsprintf( szBuffer, "Icon [%s]", lpID );
    }
    // Add it to the listbox

	if(first==TRUE)
		g_lpID=lpID;
first=FALSE;
    return TRUE;
}
/****************************************************************************
*
*     FUNCTION: ReadICOHeader
*
*     PURPOSE:  Reads the header from an ICO file
*
*     PARAMS:   HANDLE hFile - handle to the file
*
*     RETURNS:  UINT - Number of images in file, -1 for failure
*
* History:
*                July '95 - Created
*
\****************************************************************************/
UINT ReadICOHeader( HANDLE hFile )
{
    WORD    Input;
    DWORD	dwBytesRead;

    // Read the 'reserved' WORD
    if( ! ReadFile( hFile, &Input, sizeof( WORD ), &dwBytesRead, NULL ) )
        return (UINT)-1;
    // Did we get a WORD?
    if( dwBytesRead != sizeof( WORD ) )
        return (UINT)-1;
    // Was it 'reserved' ?   (ie 0)
    if( Input != 0 )
        return (UINT)-1;
    // Read the type WORD
    if( ! ReadFile( hFile, &Input, sizeof( WORD ), &dwBytesRead, NULL ) )
        return (UINT)-1;
    // Did we get a WORD?
    if( dwBytesRead != sizeof( WORD ) )
        return (UINT)-1;
    // Was it type 1?
    if( Input != 1 )
        return (UINT)-1;
    // Get the count of images
    if( ! ReadFile( hFile, &Input, sizeof( WORD ), &dwBytesRead, NULL ) )
        return (UINT)-1;
    // Did we get a WORD?
    if( dwBytesRead != sizeof( WORD ) )
        return (UINT)-1;
    // Return the count
    return Input;
}
/* End ReadICOHeader() ****************************************************/
/****************************************************************************
*
*     FUNCTION: ReadIconFromICOFile
*
*     PURPOSE:  Reads an Icon Resource from an ICO file
*
*     PARAMS:   LPCTSTR szFileName - Name of the ICO file
*
*     RETURNS:  LPICONRESOURCE - pointer to the resource, NULL for failure
*
* History:
*                July '95 - Created
*
\****************************************************************************/
LPICONRESOURCE ReadIconFromICOFile( LPCTSTR szFileName )
{
    LPICONRESOURCE    	lpIR = NULL, lpNew = NULL;
    HANDLE            	hFile = NULL;
    LPRESOURCEPOSINFO	lpRPI = NULL;
    UINT                i;
    DWORD            	dwBytesRead;
    LPICONDIRENTRY    	lpIDE = NULL;


    // Open the file
    if( (hFile = CreateFile( szFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL )) == INVALID_HANDLE_VALUE )
    {
        //MessageBox( hWndMain, "Error Opening File for Reading", szFileName, MB_OK );
        return NULL;
    }
    // Allocate memory for the resource structure
    if( (lpIR = malloc( sizeof(ICONRESOURCE) )) == NULL )
    {
        //MessageBox( hWndMain, "Error Allocating Memory", szFileName, MB_OK );
        CloseHandle( hFile );
        return NULL;
    }
    // Read in the header
    if( (lpIR->nNumImages = ReadICOHeader( hFile )) == (UINT)-1 )
    {
        //MessageBox( hWndMain, "Error Reading File Header", szFileName, MB_OK );
        CloseHandle( hFile );
        free( lpIR );
        return NULL;
    }
    // Adjust the size of the struct to account for the images
    if( (lpNew = realloc( lpIR, sizeof(ICONRESOURCE) + ((lpIR->nNumImages-1) * sizeof(ICONIMAGE)) )) == NULL )
    {
        //MessageBox( hWndMain, "Error Allocating Memory", szFileName, MB_OK );
        CloseHandle( hFile );
        free( lpIR );
        return NULL;
    }
    lpIR = lpNew;
    // Store the original name
    lstrcpy( lpIR->szOriginalICOFileName, szFileName );
    lstrcpy( lpIR->szOriginalDLLFileName, "" );
    // Allocate enough memory for the icon directory entries
    if( (lpIDE = malloc( lpIR->nNumImages * sizeof( ICONDIRENTRY ) ) ) == NULL )
    {
        //MessageBox( hWndMain, "Error Allocating Memory", szFileName, MB_OK );
        CloseHandle( hFile );
        free( lpIR );
        return NULL;
    }
    // Read in the icon directory entries
    if( ! ReadFile( hFile, lpIDE, lpIR->nNumImages * sizeof( ICONDIRENTRY ), &dwBytesRead, NULL ) )
    {
        //MessageBox( hWndMain, "Error Reading File", szFileName, MB_OK );
        CloseHandle( hFile );
        free( lpIR );
        return NULL;
    }
    if( dwBytesRead != lpIR->nNumImages * sizeof( ICONDIRENTRY ) )
    {
       // MessageBox( hWndMain, "Error Reading File", szFileName, MB_OK );
        CloseHandle( hFile );
        free( lpIR );
        return NULL;
    }
    // Loop through and read in each image
    for( i = 0; i < lpIR->nNumImages; i++ )
    {
        // Allocate memory for the resource
        if( (lpIR->IconImages[i].lpBits = malloc(lpIDE[i].dwBytesInRes)) == NULL )
        {
         //   MessageBox( hWndMain, "Error Allocating Memory", szFileName, MB_OK );
            CloseHandle( hFile );
            free( lpIR );
            free( lpIDE );
            return NULL;
        }
        lpIR->IconImages[i].dwNumBytes = lpIDE[i].dwBytesInRes;
        // Seek to beginning of this image
        if( SetFilePointer( hFile, lpIDE[i].dwImageOffset, NULL, FILE_BEGIN ) == 0xFFFFFFFF )
        {
           // MessageBox( hWndMain, "Error Seeking in File", szFileName, MB_OK );
            CloseHandle( hFile );
            free( lpIR );
            free( lpIDE );
            return NULL;
        }
        // Read it in
        if( ! ReadFile( hFile, lpIR->IconImages[i].lpBits, lpIDE[i].dwBytesInRes, &dwBytesRead, NULL ) )
        {
         //   MessageBox( hWndMain, "Error Reading File", szFileName, MB_OK );
            CloseHandle( hFile );
            free( lpIR );
            free( lpIDE );
            return NULL;
        }
        if( dwBytesRead != lpIDE[i].dwBytesInRes )
        {
          //  MessageBox( hWndMain, "Error Reading File", szFileName, MB_OK );
            CloseHandle( hFile );
            free( lpIDE );
            free( lpIR );
            return NULL;
        }
        // Set the internal pointers appropriately
        if( ! AdjustIconImagePointers( &(lpIR->IconImages[i]) ) )
        {
         //   MessageBox( hWndMain, "Error Converting to Internal Format", szFileName, MB_OK );
            CloseHandle( hFile );
            free( lpIDE );
            free( lpIR );
            return NULL;
        }
    }
    // Clean up	
    free( lpIDE );
    free( lpRPI );
    CloseHandle( hFile );
    return lpIR;
}
/* End ReadIconFromICOFile() **********************************************/
//}

/****************************************************************************
*
*     FUNCTION: MakeIconFromResource
*
*     PURPOSE:  Makes an HICON from an icon resource
*
*     PARAMS:   LPICONIMAGE	lpIcon - pointer to the icon resource
*
*     RETURNS:  HICON - handle to the new icon, NULL for failure
*
* History:
*                July '95 - Created
*
\****************************************************************************/
HICON MakeIconFromResource( LPICONIMAGE lpIcon )
{
    HICON        	hIcon = NULL;

    // Sanity Check
    if( lpIcon == NULL )
        return NULL;
    if( lpIcon->lpBits == NULL )
        return NULL;
    // Let the OS do the real work :)
    hIcon = CreateIconFromResourceEx( lpIcon->lpBits, lpIcon->dwNumBytes, TRUE, 0x00030000, 
            (*(LPBITMAPINFOHEADER)(lpIcon->lpBits)).biWidth, (*(LPBITMAPINFOHEADER)(lpIcon->lpBits)).biHeight/2, 0 );
    
    // It failed, odds are good we're on NT so try the non-Ex way
    if( hIcon == NULL )
    {
        // We would break on NT if we try with a 16bpp image
        if(lpIcon->lpbi->bmiHeader.biBitCount != 16)
        {	
            hIcon = CreateIconFromResource( lpIcon->lpBits, lpIcon->dwNumBytes, TRUE, 0x00030000 );
        }
    }
    return hIcon;
}
/* End MakeIconFromResource() **********************************************/