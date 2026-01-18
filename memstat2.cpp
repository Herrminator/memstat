#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <iframe.hpp>
#include <istattxt.hpp>
#include <itimer.hpp>

#include "memstat.h"

class MemStatWin 
  : public IFrameWindow
{
  public:
    MemStatWin();
    
  private:
    void updateInfo();
    ITimer timer;
    IStaticText info_;
};

MemStatWin::MemStatWin() 
  : IFrameWindow(IDD_MAIN, IWindow::desktopWindow()),
    info_(IDS_INFO, this),
    timer( new ITimerMemberFn0<MemStatWin>(*this, updateInfo), 1000)
{
  updateInfo();
  setBackgroundColor(IColor(255,255,255));
}

void MemStatWin::updateInfo()
{
  MEMORYSTATUS ms;
  CHAR buf[80];
  ms.dwLength = sizeof(ms);
  GlobalMemoryStatus(&ms);
  
  sprintf(buf, "PHYS %luK/%luK  PAGE %luK/%luK", 
          ms.dwAvailPhys / 1024, ms.dwTotalPhys / 1024,
          ms.dwAvailPageFile / 1024, ms.dwTotalPageFile / 1024);
  
  info_.setText(buf);
}

int main(void)
{
  MemStatWin mainWin;
  
  mainWin.show();
  IApplication::current().run();
  return(0);
}