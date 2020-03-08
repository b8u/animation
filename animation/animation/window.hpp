#pragma once
#include <libwindow/window.hpp>
#include <animation/Graphics.hpp>

class Window : public window::Window
{
  public:
    Window(const window::WndClass& wnd_class, int width, int height, const char* title);

    inline Graphics& gfx() noexcept { return graphics_; }
  private:
    LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept final
    {
        //case WM_MOUSEWHEEL:
        //{
        //    //const short fwKeys = GET_KEYSTATE_WPARAM(wParam);
        //    const short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);

        //    std::atomic_fetch_add(&wheelDelta, zDelta);
        //}

      return window::Window::HandleMsg(hWnd, msg, wParam, lParam);
    }

  private:
    Graphics graphics_;
};
