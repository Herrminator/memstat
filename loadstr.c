#include <stdio.h>

#ifdef UNICODE
  #include <wchar.h>
#endif

#include "loadstr.h"

// #define VERY_SIMPLE_NO_LANGUAGE
#ifdef VERY_SIMPLE_NO_LANGUAGE

LPTSTR LoadS(HINSTANCE hInst, UINT id, LANGID langID, LPTSTR buf, int sz)
{
  LCID old;
  INT n;
  
  n = LoadString(hInst, id, buf, sz / sizeof(TCHAR));
  if (n != 0)
    return(buf);
  else
    return(_T("(null)"));
  
}

#else

#ifndef CP_THREAD_ACP
  #define CP_THREAD_ACP 3
#endif

LPTSTR LoadS(HINSTANCE hInst, UINT uStringID, LANGID langID, LPTSTR buf, int sz)
{
  /* (c) Mickeysoft
  MORE INFORMATION
  String resources are stored as UNICODE strings in the string table. The strings in the string table are arranged into segments or blocks of 16 strings each. String IDs are indexed from one. 

  The segment a string belongs to is determined by the value of the string's identifier. For example, strings with identifiers of 0 to 15 are in one segment, strings with identifiers of 16 to 31 are in a second segment, and so on. To move a string from one segment to another you need to change its identifier. 

  Below is a sample function, GetStringFromStringTable, that demonstrates how to load a string from an application's string table that matches the default language of the operating system. 
  */

  
  wchar_t   *pwchMem, *pwchCur;
  UINT      idRsrcBlk = uStringID / 16 + 1;
  int       strIndex  = uStringID % 16;
  HINSTANCE hModule = hInst;
  HRSRC     hResource = NULL;

  hResource = FindResourceEx( hInst, RT_STRING, MAKEINTRESOURCE(idRsrcBlk),
                              langID );

  if( hResource != NULL )
  {
    pwchMem = (wchar_t *)LoadResource( hModule, hResource );

    if( pwchMem != NULL )
    {
      int i;
      pwchCur = pwchMem;
      for(i = 0; i < 16; i++ )
      {
        if( *pwchCur )
        {
          int cchString = *pwchCur;  // String size in characters.
          pwchCur++;
          if( i == strIndex )
          {
            // The string has been found in the string table.
           #ifdef UNICODE
            wcsncpy( buf, pwchCur, min(sz, cchString) );
           #else
            if (WideCharToMultiByte(GetConsoleOutputCP() == 0 ? CP_ACP : CP_OEMCP, 0,
                                    pwchCur, cchString,
                                    buf, sz,
                                    NULL, NULL) == 0)
            {
              printf("WcTOMB: %d 0x%X\n", GetLastError(), GetLastError());
              fflush(stdout);
              return _T("(null)");
            }
           #endif
            buf[ min(sz, cchString) ] = '\0';
            return buf;
          }
          pwchCur += cchString;
        }
        else
          pwchCur++;
      }
    }
  }
  return _T("(null)");

} 
   
#endif

