#ifndef UNICODE
  #define UNICODE
#endif
#include <windows.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
  TCHAR buf[256];
  
  
  GetLocaleInfo(LOCALE_USER_DEFAULT,
                LOCALE_SLANGUAGE,
                buf, sizeof(buf) / sizeof(TCHAR));
  printf("SLANGUAGE: %ls\n", buf);
  
  printf("cp: %u\n", GetConsoleOutputCP());
  
  if (argc > 1)
  {
    SetThreadLocale(MAKELCID(MAKELANGID(LANG_GERMAN,SUBLANG_GERMAN_AUSTRIAN),
                             0));
  }
  
  LoadString(GetModuleHandle(NULL), 2012, buf, sizeof(buf));
  printf("IDS_ABOUT: %s\n", buf);
  
  printf("lang (def): 0x%04hX\n", GetUserDefaultLangID());
  printf("lang (ENU): 0x%04hX\n", MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US));
  printf("lang (DEU): 0x%04hX\n", MAKELANGID(LANG_GERMAN,SUBLANG_GERMAN));
  printf("lang (DEA): 0x%04hX\n", MAKELANGID(LANG_GERMAN,SUBLANG_GERMAN_AUSTRIAN));
  printf("lang (DES): 0x%04hX\n", MAKELANGID(LANG_GERMAN,SUBLANG_GERMAN_SWISS));
  printf("lang (DEF): 0x%04hX\n", MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT));
  return(0);
}
