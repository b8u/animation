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
  }
  return 0;
}
