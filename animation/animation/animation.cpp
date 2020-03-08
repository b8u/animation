#include <libcommon/minimal_win.hpp>

#include "application.hpp"

int WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
  try
  {
    Application app;
    return app.Run();
  }
  catch(...)
  {
    return -1;
  }
}

int main()
{
  return WinMain(GetModuleHandle(NULL), NULL, GetCommandLineA(), SW_SHOWNORMAL);
}
