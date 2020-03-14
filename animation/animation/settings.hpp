#pragma once 

#include <memory>

struct Settings
{
  constexpr uint16_t width()  const { return 16 * scale; }
  constexpr uint16_t height() const { return 9 * scale; }

  static constexpr const uint16_t scale = 50;
  static constexpr const float ratio = 9.0f / 16.0f;
};


extern std::unique_ptr<Settings> g_settings;
