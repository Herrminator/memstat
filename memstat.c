#include <stdlib.h>
#include <windows.h>
#include <commdlg.h>
#include <time.h>
#include <wchar.h>
#include <winnt.h>

#include "memstat.h"
#include "resource.h"

#include "loadstr.h"

#define MY_PRIORITY IDLE_PRIORITY_CLASS
#define PV_PRIORITY NORMAL_PRIORITY_CLASS

#ifndef ICON_SMALL
  #define ICON_SMALL 0
  #define ICON_BIG   1
#endif

#ifndef UNICODE
  #define lstrrchr strrchr
#else
  #define lstrrchr wcsrchr
#endif

static BOOL bOnTop;
static BOOL bSeconds = FALSE;
 
static LANGID lang = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);

static TCHAR prcView[256];
static TCHAR prcViewExe[256];

static COLORREF bk;
static COLORREF fg;
static COLORREF cust[16];

static HINSTANCE hInst;
static HBRUSH hbr;

static HMENU sysMenu = INVALID_HANDLE_VALUE;

static int updTime = UPD_NORMAL;

static int updWindowText = 1;

static BOOL ModifyStyleEx(HWND hwnd, DWORD unset, DWORD set)
{
  DWORD old = GetWindowLong(hwnd, GWL_EXSTYLE);
  DWORD exs = (old & ~unset) | set;
  return(SetWindowLong(hwnd, GWL_EXSTYLE, exs) != 0);
}


LANGID LoadLanguage(VOID)
{
  HKEY k;
  DWORD t, s;
  DWORD langID;
  
  if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\DITJ\\MemStat"),
                   0, KEY_READ, &k) != ERROR_SUCCESS)
    return(GetUserDefaultUILanguage());
  s = sizeof(langID);
  if (RegQueryValueEx(k, _T("overrideLang"), NULL, &t, (LPBYTE) &langID, &s) != 0)
    langID = GetUserDefaultUILanguage();
  RegCloseKey(k);
  return (LANGID) langID;
}


void updateInfo(HWND hwnd)
{
  #define BUFSZ 512
  struct tm *t;
  time_t lt;
  TCHAR buf[BUFSZ] = { 0 };
  static TCHAR prevBuf[BUFSZ] = { 0 };

  time(&lt);
  t = localtime(&lt);

  memset(buf, 0, sizeof(buf));

#ifdef ONLY_TINY_MEMORY
  {
    MEMORYSTATUS ms;
    long ap, apK, tp, tpK, af, afK, tf, tfK;
    
    ms.dwLength = sizeof(ms);
    GlobalMemoryStatus(&ms);
   
    ap  = (ms.dwAvailPhys / 1024) % 1000;
    apK = (ms.dwAvailPhys / 1024) / 1000;
    tp  = (ms.dwTotalPhys / 1024) % 1000;
    tpK = (ms.dwTotalPhys / 1024) / 1000;
    af  = (ms.dwAvailPageFile / 1024) % 1000;
    afK = (ms.dwAvailPageFile / 1024) / 1000;
    tf  = (ms.dwTotalPageFile / 1024) % 1000;
    tfK = (ms.dwTotalPageFile / 1024) / 1000;
    
    swprintf_s(buf, BUFSZ, _T("PHYS %lu.%03luK/%lu.%03luK  PAGE %lu.%03luK/%lu.%03luK  %02d:%02d"), 
             apK, ap, tpK, tp, afK, af, tfK, tf,
             t->tm_hour, t->tm_min);
  }
#else
  {
    MEMORYSTATUSEX ms;
    DWORDLONG ap, apK, tp, tpK, af, afK, tf, tfK;
    
    ms.dwLength = sizeof(ms);
    GlobalMemoryStatusEx(&ms);
   
    ap  = (ms.ullAvailPhys / 1024) % 1000;
    apK = (ms.ullAvailPhys / 1024) / 1000;
    tp  = (ms.ullTotalPhys / 1024) % 1000;
    tpK = (ms.ullTotalPhys / 1024) / 1000;
    af  = (ms.ullAvailPageFile / 1024) % 1000;
    afK = (ms.ullAvailPageFile / 1024) / 1000;
    tf  = (ms.ullTotalPageFile / 1024) % 1000;
    tfK = (ms.ullTotalPageFile / 1024) / 1000;

    if (ms.ullTotalPhys > ((DWORDLONG) 4 * 1024 * 1024 * 1024)) {
      RECT r;
      GetWindowRect(hwnd, &r);
      SetWindowPos(hwnd, 0, 0, 0, HUGE_WIDTH, r.bottom - r.top, SWP_NOMOVE);
    }

    swprintf_s(buf, BUFSZ, _T("PHYS %I64u.%03I64uK/%I64u.%03I64uK  PAGE %I64u.%03I64uK/%I64u.%03I64uK  %02d:%02d"), 
               apK, ap, tpK, tp, afK, af, tfK, tf,
               t->tm_hour, t->tm_min);
	if (bSeconds) {
		TCHAR sec[4];
		swprintf_s(sec, sizeof(sec), _T(":%02d"), t->tm_sec);
		wcscat_s(buf, BUFSZ, sec);
	}
  }
#endif

  if (wcscmp(buf, prevBuf))
  {
    SetDlgItemText(hwnd, IDS_INFO, buf);
    SetWindowText(hwnd, buf);
    wcscpy(prevBuf, buf);
  }
}

void SaveSettings(HWND hwnd)
{
  RECT r;
  HKEY k;
  DWORD d;
  
  GetWindowRect(hwnd, &r);
  if (RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\DITJ\\MemStat"),
                     0, NULL, 0, KEY_WRITE, NULL, &k, &d) != ERROR_SUCCESS)
    return;
    
  RegSetValueEx(k, _T("onTop"),     0, REG_BINARY, (LPBYTE) &bOnTop,    sizeof(bOnTop));
  RegSetValueEx(k, _T("seconds"),   0, REG_BINARY, (LPBYTE) &bSeconds,  sizeof(bSeconds));
  RegSetValueEx(k, _T("xPos"),      0, REG_BINARY, (LPBYTE) &r.left,    sizeof(r.left));
  RegSetValueEx(k, _T("yPos"),      0, REG_BINARY, (LPBYTE) &r.top,     sizeof(r.top));
  RegSetValueEx(k, _T("fg"),        0, REG_BINARY, (LPBYTE) &fg,        sizeof(fg));
  RegSetValueEx(k, _T("bk"),        0, REG_BINARY, (LPBYTE) &bk,        sizeof(bk));
  RegSetValueEx(k, _T("upd"),       0, REG_BINARY, (LPBYTE) &updTime,   sizeof(updTime));
  RegSetValueEx(k, _T("icon"),      0, REG_BINARY, (LPBYTE) &updWindowText,sizeof(updWindowText));
  RegSetValueEx(k, _T("cust"),      0, REG_BINARY, (LPBYTE) cust,    sizeof(COLORREF)*16);
  RegSetValueEx(k, _T("prcView"),   0, REG_SZ,  (LPBYTE) prcView, lstrlen(prcView)*sizeof(TCHAR));
  RegSetValueEx(k, _T("prcViewExe"),0,REG_SZ,  (LPBYTE) prcViewExe, lstrlen(prcViewExe)*sizeof(TCHAR));
  RegCloseKey(k);
}

void LoadSettings(HWND hwnd)
{
  RECT r;
  HKEY k;
  DWORD t, s;
  int   h, w, sx, sy;
  TCHAR buf1[80], buf2[80];
  
  GetWindowRect(hwnd, &r);

  w  = r.right - r.left;              h  = r.bottom - r.top;
  sx = GetSystemMetrics(SM_CXSCREEN); sy = GetSystemMetrics(SM_CYSCREEN);

  if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\DITJ\\MemStat"),
                   0, KEY_READ, &k) != ERROR_SUCCESS)
    return;
  
  s = sizeof(bOnTop);
  RegQueryValueEx(k, _T("onTop"),   NULL, &t, (LPBYTE) &bOnTop, &s);
  s = sizeof(bSeconds);
  RegQueryValueEx(k, _T("seconds"), NULL, &t, (LPBYTE)&bSeconds, &s);
  s = sizeof(r.left);
  RegQueryValueEx(k, _T("xPos"),    NULL, &t, (LPBYTE) &r.left, &s);
  s = sizeof(r.top);
  RegQueryValueEx(k, _T("yPos"),    NULL, &t, (LPBYTE) &r.top, &s);
  s = sizeof(fg);
  RegQueryValueEx(k, _T("fg"),      NULL, &t, (LPBYTE) &fg, &s);
  s = sizeof(bk);
  RegQueryValueEx(k, _T("bk"),      NULL, &t, (LPBYTE) &bk, &s);
  s = sizeof(updTime);
  RegQueryValueEx(k, _T("upd"),     NULL, &t, (LPBYTE) &updTime, &s);
  s = sizeof(updWindowText);
  RegQueryValueEx(k, _T("icon"),    NULL, &t, (LPBYTE) &updWindowText, &s);
  s = sizeof(COLORREF) * 16;
  RegQueryValueEx(k, _T("cust"),    NULL, &t, (LPBYTE) cust, &s);
  s = sizeof(prcView) * (DWORD) sizeof(TCHAR);
  RegQueryValueEx(k, _T("prcView"), NULL,&t, (LPBYTE) prcView, &s);
  s = sizeof(prcViewExe) * (DWORD) sizeof(TCHAR);
  RegQueryValueEx(k, _T("prcViewExe"),NULL,&t, (LPBYTE) prcViewExe, &s);
  s = sizeof(bk);
  RegCloseKey(k);
  
  r.left = max(0, min(r.left, sx - w));
  r.top  = max(0, min(r.top,  sy - h));

  SetWindowPos(hwnd, bOnTop ? HWND_TOPMOST : HWND_NOTOPMOST, 
               r.left, r.top, 0,0, 
               SWP_NOSIZE);

  CheckMenuItem(sysMenu, IDM_LOW,    updTime == UPD_LOW    ? MF_CHECKED : MF_UNCHECKED);
  CheckMenuItem(sysMenu, IDM_NORMAL, updTime == UPD_NORMAL ? MF_CHECKED : MF_UNCHECKED);
  CheckMenuItem(sysMenu, IDM_HIGH,   updTime == UPD_HIGH   ? MF_CHECKED : MF_UNCHECKED);

  CheckMenuItem(sysMenu, 
                IDM_ONTOP, bOnTop ? MF_CHECKED : MF_UNCHECKED);
  CheckMenuItem(sysMenu, 
                IDM_HIDE,  !updWindowText ? MF_CHECKED : MF_UNCHECKED);
  CheckMenuItem(sysMenu,
	            IDM_SECONDS, bSeconds ? MF_CHECKED : MF_UNCHECKED);

  LoadS(hInst, IDM_PRCVIEW, lang, buf1, sizeof(buf1));
  wsprintf(buf2, buf1, prcView);
  
  ModifyMenu(sysMenu, 
             IDM_PRCVIEW, MF_BYCOMMAND | MF_STRING, IDM_PRCVIEW,
             buf2);
}

VOID SetProcView(HWND hwnd)
{
  OPENFILENAME of;
  static TCHAR dir[256], title[32], filter[256], *p;
  static TCHAR buf1[256], buf2[256], buf3[256];
  TCHAR  nul = _T('\0');
  LPTSTR pathExt = LoadS(hInst, IDS_PATHEXT, lang, buf3, sizeof(buf3)); // getenv("PATHEXT");
  
  if (!lstrcmp(pathExt, _T("(null)")))
    pathExt = _T("*.exe;*.cmd;*.com;*.bat;*.rex");

  wsprintf(title,  _T("%s"), LoadS(hInst, IDS_PROGTITLE, lang, buf1, sizeof(buf1)) );
  wsprintf(filter, _T("%s (%s)%c%s%c%s (*.*)%c*.*%c%c"),
                   LoadS(hInst, IDS_FILTER1, lang, buf1, sizeof(buf1)), 
                     pathExt, nul, pathExt, nul,
                   LoadS(hInst, IDS_FILTER0, lang, buf2, sizeof(buf2)),
                     nul, nul, nul);
                     
  memset(&of, 0, sizeof(of));
  of.lStructSize = sizeof(of);
  of.lpstrTitle = title;
  of.hwndOwner = hwnd;
  of.lpstrFilter = filter;
  of.lpstrFile = prcViewExe;
  of.nMaxFile = 256;
  lstrcpy(dir, prcViewExe);
  if ((p = lstrrchr(dir, _T('\\'))) != NULL)
  {
    p[1] = _T('\0');
    of.lpstrInitialDir = dir;
  }
  of.Flags = OFN_NOCHANGEDIR | OFN_HIDEREADONLY;
  if (!GetOpenFileName(&of))
    return;
  lstrcpy(prcView, &(prcViewExe[of.nFileOffset]));
  if (of.nFileExtension != 0)
    prcView[of.nFileExtension - of.nFileOffset - 1] = _T('\0');
  LoadS(hInst, IDM_PRCVIEW, lang, buf1, sizeof(buf1));
  wsprintf(buf2, buf1, prcView);
  ModifyMenu(sysMenu, 
             IDM_PRCVIEW, MF_BYCOMMAND | MF_STRING, IDM_PRCVIEW,
             buf2);
}

VOID SetColor(HWND hwnd, BOOL bBk)
{
  CHOOSECOLOR cc;
  
  memset(&cc, 0, sizeof(cc));
  cc.lStructSize = sizeof(cc);
  cc.hwndOwner = hwnd;
  cc.rgbResult = bBk ? bk : fg;
  cc.lpCustColors = cust;
  cc.Flags = CC_RGBINIT;
  if (!ChooseColor(&cc))
    return;
  if (bBk)
  {
    LOGBRUSH lbr;
    bk = cc.rgbResult;
    lbr.lbStyle = BS_SOLID;
    lbr.lbColor = bk;
    DeleteObject(hbr);
    hbr = CreateBrushIndirect(&lbr);
  }
  else
    fg = cc.rgbResult;
} 

void StartProcView(VOID)
{
  char buf[80];
  
  SetPriorityClass(GetCurrentProcess(), PV_PRIORITY);
  
  #ifndef UNICODE
    wsprintfA(buf, "start %hs", prcViewExe);
  #else
    wsprintfA(buf, "start %ls", prcViewExe);
  #endif
  
  system(buf);
  
  SetPriorityClass(GetCurrentProcess(), MY_PRIORITY);
}

void InitMain(HWND hwnd)
{
  LOGBRUSH lbr;
  HMENU updMenu = CreatePopupMenu();
  HICON hIco = LoadIcon(hInst, MAKEINTRESOURCE(IDI_MAIN));
  HICON hIcoS= LoadImage(hInst,MAKEINTRESOURCE(IDI_MAIN), 
                         IMAGE_ICON,
                         GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 
                         LR_DEFAULTCOLOR);
  TCHAR buf[80];
  #define LS(id) LoadS(hInst, id, lang, buf, sizeof(buf))

  lang = LoadLanguage();
  
  sysMenu = GetSystemMenu(hwnd, FALSE);
  
  AppendMenu(sysMenu, MF_SEPARATOR, 0, NULL);
  AppendMenu(sysMenu, MF_STRING,              IDM_PRCVIEW, _T("Start TaskMgr"));
  AppendMenu(sysMenu, MF_SEPARATOR, 0, NULL);
  AppendMenu(sysMenu, MF_STRING | MF_CHECKED, IDM_ONTOP,   LS(IDM_ONTOP));
  AppendMenu(sysMenu, MF_STRING | MF_CHECKED, IDM_HIDE,    LS(IDM_HIDE));
  AppendMenu(sysMenu, MF_STRING | MF_CHECKED, IDM_SECONDS, LS(IDM_SECONDS));
  AppendMenu(updMenu, MF_STRING,            IDM_LOW,     LS(IDM_LOW));
    AppendMenu(updMenu, MF_STRING|MF_CHECKED, IDM_NORMAL,  LS(IDM_NORMAL));
    AppendMenu(updMenu, MF_STRING,            IDM_HIGH,    LS(IDM_HIGH));
  AppendMenu(sysMenu,   MF_STRING | MF_POPUP, (UINT_PTR)updMenu,LS(IDM_UPDATE));
  AppendMenu(sysMenu, MF_SEPARATOR, 0, NULL);
  AppendMenu(sysMenu, MF_STRING,              IDM_OPTIONS, LS(IDM_OPTIONS));
  AppendMenu(sysMenu, MF_STRING,              IDM_FGCOLOR, LS(IDM_FGCOLOR));
  AppendMenu(sysMenu, MF_STRING,              IDM_BGCOLOR, LS(IDM_BGCOLOR));
  AppendMenu(sysMenu, MF_STRING,              IDM_DEFPOS,  LS(IDM_DEFPOS));
  AppendMenu(sysMenu, MF_SEPARATOR, 0, NULL);
  AppendMenu(sysMenu, MF_STRING,              IDM_ABOUT,   LS(IDM_ABOUT));
  
  bk = RGB(0x00, 0x00, 0x00);
  fg = RGB(0x00, 0xFF, 0x00);
  lbr.lbStyle = BS_SOLID;
  lbr.lbColor = bk;
  hbr = CreateBrushIndirect(&lbr);
  
  bOnTop = TRUE;
  lstrcpy(prcView, _T("TaskMgr"));
  lstrcpy(prcViewExe, _T("TaskMgr.EXE"));
  CheckMenuItem(sysMenu, IDM_ONTOP, MF_CHECKED);
  CheckMenuItem(sysMenu, IDM_HIDE,  MF_CHECKED);
  SetWindowPos(hwnd, HWND_TOPMOST, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);
  SendMessage(hwnd, WM_SETICON, ICON_BIG,   (LPARAM) hIco);
  SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM) hIcoS);
  
  LoadSettings(hwnd);
  
  updateInfo(hwnd);
  if (!updWindowText)
  {
    SetWindowText(hwnd, _T("MemStat"));
    ShowWindow(hwnd, SW_HIDE);
    ModifyStyleEx(hwnd, 0, WS_EX_TOOLWINDOW);
    ShowWindow(hwnd, SW_SHOWDEFAULT);
  }
  SetPriorityClass(GetCurrentProcess(), MY_PRIORITY);
  SetTimer(hwnd, ID_TIMER1, updTime, NULL);
}

INT_PTR CALLBACK MainDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  static BOOL dragging = FALSE;
  static BOOL closing = FALSE;
  RECT r;
  
  switch (msg)
  {
    case WM_INITDIALOG:
      InitMain(hwnd);
      return(TRUE);
    
    case WM_CTLCOLORSTATIC:
      SetBkColor((HDC) wParam, bk);
      SetTextColor((HDC) wParam, fg);
      return (INT_PTR) hbr;
      
    case WM_TIMER:
      updateInfo(hwnd);
      return(FALSE);
      
    case WM_CONTEXTMENU:
      TrackPopupMenu(sysMenu, 
                     TPM_LEFTALIGN, LOWORD(lParam), HIWORD(lParam), 
                     0, hwnd, NULL);
      return(FALSE);
      
    case WM_COMMAND:
    case WM_SYSCOMMAND:
      switch(LOWORD(wParam))
      {
        case SC_CLOSE:
          closing = TRUE;
          SaveSettings(hwnd);
          KillTimer(hwnd, ID_TIMER1);
          DeleteObject(hbr);
          EndDialog(hwnd, 0);
          return(TRUE);
          
        case SC_MOVE:
          GetWindowRect(hwnd, &r);
          SetCursorPos(r.left, r.top);
          SetCapture(hwnd);
          dragging = TRUE;
          SetCursor(LoadCursor(0, IDC_CROSS));
          return(TRUE);
          
        case IDM_DEFPOS:
          SetWindowPos(hwnd, bOnTop ? HWND_TOPMOST : HWND_NOTOPMOST, 0,0,0,0, 
                       SWP_NOSIZE);
          bOnTop = !TRUE;
          /* FALL THROUGH ! */
        case IDM_ONTOP:
        {
          bOnTop = !bOnTop;
          CheckMenuItem(sysMenu, IDM_ONTOP, bOnTop ? MF_CHECKED : MF_UNCHECKED);
          SetWindowPos(hwnd, bOnTop ? HWND_TOPMOST : HWND_NOTOPMOST, 0,0,0,0, 
                       SWP_NOMOVE | SWP_NOSIZE);
          SaveSettings(hwnd);
          return(TRUE);
        }
		case IDM_SECONDS:
		{
			bSeconds = !bSeconds;
			CheckMenuItem(sysMenu, IDM_SECONDS, bSeconds ? MF_CHECKED : MF_UNCHECKED);
			SaveSettings(hwnd);
			return(TRUE);
		}
		case IDM_HIDE:
        {
          updWindowText = !updWindowText;
          CheckMenuItem(sysMenu, IDM_HIDE, !updWindowText ? MF_CHECKED : MF_UNCHECKED);
          ShowWindow(hwnd, SW_HIDE);
          if (!updWindowText)
            ModifyStyleEx(hwnd, WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
          else
            ModifyStyleEx(hwnd, WS_EX_TOOLWINDOW, WS_EX_APPWINDOW);
          ShowWindow(hwnd, SW_SHOWDEFAULT);
          SaveSettings(hwnd);
          return(TRUE);
        }
        case IDM_PRCVIEW:
          StartProcView();
          return(TRUE);
          
        case IDM_OPTIONS:
          SetProcView(hwnd);
          SaveSettings(hwnd);
          return(TRUE);
          
        case IDM_LOW:
        case IDM_NORMAL:
        case IDM_HIGH:
        {
          HMENU mnu = sysMenu;
          switch (LOWORD(wParam))
          {
            case IDM_LOW:     updTime = UPD_LOW; break;
            case IDM_NORMAL:  updTime = UPD_NORMAL; break;
            case IDM_HIGH:    updTime = UPD_HIGH; break;
          }
          KillTimer(hwnd, ID_TIMER1);
          SetTimer(hwnd, ID_TIMER1, updTime, 0);
          CheckMenuItem(mnu, IDM_LOW,    updTime == UPD_LOW    ? MF_CHECKED : MF_UNCHECKED);
          CheckMenuItem(mnu, IDM_NORMAL, updTime == UPD_NORMAL ? MF_CHECKED : MF_UNCHECKED);
          CheckMenuItem(mnu, IDM_HIGH,   updTime == UPD_HIGH   ? MF_CHECKED : MF_UNCHECKED);
          SaveSettings(hwnd);
          return(TRUE);
        }
        case IDM_BGCOLOR:
        case IDM_FGCOLOR:
          SetColor(hwnd, LOWORD(wParam) == IDM_BGCOLOR);
          SaveSettings(hwnd);
          return(TRUE);
        case IDM_ABOUT:
        {
          TCHAR buf[256];
          MessageBox(hwnd, 
                     LoadS(hInst, IDS_ABOUT, lang, buf, sizeof(buf)),
                     _T("MemStat"),
                     MB_OK | MB_APPLMODAL);
          return(TRUE);
        }
        default:
          return(FALSE);
      }
    case WM_LBUTTONDBLCLK:
      StartProcView();
      dragging = FALSE;
      ReleaseCapture();
      return(TRUE);
        
    case WM_LBUTTONDOWN:
    if (!dragging)
    {
      SetCapture(hwnd);
      dragging = TRUE;
      SetCursor(LoadCursor(0, IDC_CROSS));
    }
    return(TRUE);

    case WM_MOUSEMOVE:
      if (dragging)
      {
        int x, y;
        
        GetWindowRect(hwnd, &r);
        x = r.left + (short int) LOWORD(lParam);
        y = r.top  + (short int) HIWORD(lParam);
        SetWindowPos(hwnd,
                     bOnTop ? HWND_TOPMOST : HWND_NOTOPMOST, 
                     x, y, 0, 0, SWP_NOSIZE);
        SetCursor(LoadCursor(0, IDC_CROSS));
        return(TRUE);
      }
      return(FALSE);

    case WM_LBUTTONUP:
      if (dragging)
      {
        SaveSettings(hwnd);
        SetCursor(LoadCursor(0, IDC_ARROW));
        dragging = FALSE;
        ReleaseCapture();
      }
      return(TRUE);

    case WM_DISPLAYCHANGE:
      LoadSettings(hwnd);
      return TRUE;

    case WM_CLOSE:
      if (!closing)
        MessageBox(NULL, _T("Unexpected Closing of the window!"), _T("MemStat"),
                   MB_OK | MB_ICONERROR | MB_SYSTEMMODAL);
      return(FALSE);
    default:
      break;
  }
  return(FALSE);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, 
                   LPSTR cmdLine, int nCmdShow)
{
  _tzset();
  
  hInst = hInstance;
  
  DialogBox(hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, (DLGPROC) MainDlgProc);
  
  return(0);
}