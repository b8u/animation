#pragma once
#include <animation/window.hpp>

class Application
{
  public:
    Application() = default;

    int Run();

  private:
    window::WndClass wnd_class_{&Window::HandleMsgSetup};
    Window window_{wnd_class_, 640, 480, "Animation"};
};
