#pragma once 

#include <memory>
#include <functional>

struct Settings
{
  constexpr uint16_t width()  const { return 16 * scale; } // 800
  constexpr uint16_t height() const { return 9 * scale; }  // 450

  static constexpr const uint16_t scale = 50;
  static constexpr const float ratio = 9.0f / 16.0f;

  bool left = false;
  bool right = false;
};


extern std::unique_ptr<Settings> g_settings;
