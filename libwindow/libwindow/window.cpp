#include <libwindow/window.hpp>

#include <cassert>

namespace window
{

  WndClass::WndClass(WNDPROC wnd_proc) noexcept
    : instance_(GetModuleHandle(nullptr))
  {
    assert(wnd_proc != nullptr);
    WNDCLASSEX window_class = {};

    window_class.cbSize = sizeof(WNDCLASSEX);

    window_class.style = CS_OWNDC;
    window_class.lpfnWndProc = wnd_proc;
    window_class.cbClsExtra = 0;
    window_class.cbWndExtra = 0;
    window_class.hInstance = instance();
    window_class.hIcon = nullptr;  // LoadIcon(instance, MAKEINTRESOURCE(IDI_WINDOWSPROJECT1));
    window_class.hCursor = nullptr;
    window_class.hbrBackground = nullptr;
    window_class.lpszMenuName = nullptr;
    window_class.lpszClassName = name();
    window_class.hIconSm = nullptr;
    //LoadIcon(instance, MAKEINTRESOURCE(IDI_SMALL));

    const ATOM res = ::RegisterClassEx(&window_class);
    assert(res != 0);
  }

  WndClass::~WndClass()
  {
    const BOOL res = UnregisterClass(name(), instance());
    //assert(res != FALSE);
  }

  const char* WndClass::name() const noexcept { return class_name; }
  HINSTANCE WndClass::instance() const noexcept { return instance_; }


  Window::Window(const WndClass& wnd_class, int width, int height, const char* title)
  {
    assert(width > 0);
    assert(height > 0);
    assert(title != nullptr);
    
    // calculate window size based on desired client region size
    RECT wr;
    wr.left = 100;
    wr.right = width + wr.left;
    wr.top = 100;
    wr.bottom = height + wr.top;

    BOOL res1 = ::AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE);
    assert(res1 != FALSE);

    window_handle_ =
        ::CreateWindow(
            wnd_class.name(), 
            title, 
            WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, CW_USEDEFAULT, 
            CW_USEDEFAULT,
            wr.right - wr.left, 
            wr.bottom - wr.top, 
            nullptr, 
            nullptr, 
            wnd_class.instance(), 
            this);

    assert(window_handle_ != nullptr);

    ::ShowWindow(window_handle_, SW_SHOWDEFAULT);
  }

  Window::~Window()
  {
    DestroyWindow(window_handle_);
  }

  std::optional<int> Window::ProcessMessages() noexcept
  {
      MSG msg;
      while (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
          // check for quit because peekmessage does not signal this via return val
          if (msg.message == WM_QUIT) { return static_cast<int>(msg.wParam); }

          // TranslateMessage will post auxilliary WM_CHAR messages from key msgs
          ::TranslateMessage(&msg);
          ::DispatchMessage(&msg);
      }

      // return empty optional when not quitting app
      return {};
  }

  LRESULT CALLBACK Window::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
  {
      // use create parameter passed in from CreateWindow() to store window class pointer at WinAPI side
      if (msg == WM_NCCREATE) {
          // extract ptr to window class from creation data
          const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
          Window* const pWnd = static_cast<Window*>(pCreate->lpCreateParams);
          // set WinAPI-managed user data to store ptr to window instance
          SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
          // set message proc to normal (non-setup) handler now that setup is finished
          SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMsgThunk));
          // forward message to window instance handler
          return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
      }
      // if we get a message before the WM_NCCREATE message, handle with default handler
      return DefWindowProc(hWnd, msg, wParam, lParam);
  }

  LRESULT Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
  {
      switch (msg) {
          // we don't want the DefProc to handle this message because
          // we want our destructor to destroy the window, so return 0 instead of break
          case WM_CLOSE: PostQuitMessage(0); return 0;

          return 0;
      }
      return DefWindowProc(hWnd, msg, wParam, lParam);
  }

  LRESULT CALLBACK Window::HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
  {
      // retrieve ptr to window instance
      // TODO: Опасное место
      Window* const pWnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
      // forward message to window instance handler
      return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
  }
}
