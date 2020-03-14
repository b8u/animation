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
