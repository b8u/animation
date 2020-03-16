#pragma once
#include <libwindow/window.hpp>
#include <animation/graphics.hpp>
#include <animation/settings.hpp>

class Window : public window::Window
{
  public:
    Window(const window::WndClass& wnd_class, const char* title);

    inline Graphics& gfx() noexcept { return graphics_; }

  private:
    LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept final;

  private:
    Graphics graphics_;
};
