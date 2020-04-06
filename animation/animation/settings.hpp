#pragma once 

#include <memory>
#include <functional>

struct Settings
{
  static constexpr uint16_t width  = 688;
  static constexpr uint16_t height = 432;

  static constexpr const float ratio = static_cast<float>(height) / static_cast<float>(width);

  constexpr float pxToX(uint16_t px)
  {
    const float halfWidth = width / 2;
    if (px == halfWidth) { 
      return 0.0f;
    } else {
      const float relative = px / halfWidth;
      if (px < halfWidth) {
        return -(1.0f - relative);
      } else {
        return relative - 1.0f;
      }
    }
  }

  constexpr float pxToY(uint16_t px)
  {
    const float halfHeight = height / 2;
    if (px == halfHeight) { 
      return 0.0f;
    } else {
      const float relative = px / halfHeight;
      if (px < halfHeight) {
        return 1.0f - relative;
      } else {
        return -(relative - 1.0f);
      }
    }
  }

  bool left = false;
  bool right = false;
};


extern std::unique_ptr<Settings> g_settings;
