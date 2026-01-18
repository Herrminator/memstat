#ifndef _LOADSTR_H
#define _LOADSTR_H

#include <windows.h>

LPTSTR LoadS(HINSTANCE hInst, UINT uStringID, LANGID lang, LPTSTR buf, int sz);

#ifndef _T
  #ifdef UNICODE
    #define _T(x) L##x 
  #else
    #define _T(x) x
  #endif
#endif

#endif // _LOADSTR_H
