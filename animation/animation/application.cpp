#include "application.hpp"

#include <chrono>
#include <thread>

int Application::Run()
{
  while (true) {
    if (const auto ecode = window_.ProcessMessages()) {
      return *ecode;
    } else {
      using namespace std::chrono_literals;
      std::this_thread::sleep_for(16ms);
    }

    DoFrame();
  }
  return 0;
}

void Application::DoFrame()
{
    constexpr const std::array<float, 3> green = {216.0f / 255.0f, 230.0f / 255.0f, 173.0f / 255.0f};
    constexpr const std::array<float, 3> pink = {230.0f / 255.0f, 173.0f / 255.0f, 216.0f / 255.0f};
    constexpr const std::array<float, 3> blue = {173.0f / 255.0f, 216.0f / 255.0f, 230.0f / 255.0f};

//    const int wheelCount = window_.wheelCount();

//    static int zoom = 0;
//    zoom += wheelCount;

    window_.gfx().BeginFrame(pink);

    window_.gfx().DrawAllThisShit();

//    static float angle = 0.0f;
//    angle += 0.03f;
//	window_.gfx().DrawTriangle(angle, zoom);
//
//
//    static float angle2 = 0.0f;
//    angle2 -= 0.01f;
//    window_.gfx().DrawTriangle(angle2, -2);
//
    window_.gfx().EndFrame();
}
