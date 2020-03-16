#include "window.hpp"

Window::Window(
    const window::WndClass& wnd_class,
    const char* title)
  : window::Window(
      wnd_class,
      g_settings->width(),
      g_settings->height(),
      title)
  , graphics_(
      g_settings->width(),
      g_settings->height(),
      handle())
{
}

LRESULT Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
{
  if (msg == WM_KEYDOWN) {
    switch (wParam) {
      case VK_LEFT : if (g_settings) { g_settings->left  = true; } break;
      case VK_RIGHT: if (g_settings) { g_settings->right = true; } break;
      //case VK_UP   : break;
      //case VK_DOWN : break;
    }
  } else if (msg == WM_KEYUP) {
    switch (wParam) {
      case VK_LEFT : if (g_settings) { g_settings->left  = false; } break;
      case VK_RIGHT: if (g_settings) { g_settings->right = false; } break;
    }
  }

  return window::Window::HandleMsg(hWnd, msg, wParam, lParam);
}
