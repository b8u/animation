#pragma once

#include <libwindow/export.hpp>

#include <optional>

#include <libcommon/minimal_win.hpp>

namespace window
{
// Sample
//  LIBWINDOW_SYMEXPORT void
//  say_hello (std::ostream&, const std::string& name);

  class WndClass
  {
    private:
      static const constexpr char class_name[] = "b8u-Win-Class";

    public:
      WndClass(WNDPROC wnd_proc) noexcept;
      WndClass(const WndClass&) = delete;
      ~WndClass();

      WndClass& operator=(const WndClass&) = delete;

      const char* name() const noexcept;
      HINSTANCE instance() const noexcept;

    private:
      HINSTANCE instance_;
  };

  class Window
  {
    public:
      Window(const WndClass& wnd_class, int width, int height, const char* title);
      virtual ~Window();

      std::optional<int> ProcessMessages() noexcept;
      
    public:
      static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

    protected:
      virtual LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

    private:
      static LRESULT CALLBACK HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;

    private:
      HWND window_handle_;
  };
}
