#include "window.hpp"

Window::Window(const window::WndClass& wnd_class, int width, int height, const char* title)
  : window::Window(wnd_class, width, height, title)
  , graphics_(width, height, handle())
{
}
