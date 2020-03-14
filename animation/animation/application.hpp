#pragma once
#include <animation/window.hpp>
#include <animation/settings.hpp>

class Application
{
  public:
    Application() = default;

    int Run();
    void DoFrame();

  private:
    window::WndClass wnd_class_{&Window::HandleMsgSetup};
    Settings settings_;
    Window window_{ wnd_class_, "Animation" };
};
