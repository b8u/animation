#include <libcommon/minimal_win.hpp>

#include "application.hpp"
#include "settings.hpp"

int WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
  try
  {
    g_settings = std::make_unique<Settings>();

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
