#pragma once

#include <animation/uv.hpp>

#include <chrono>
#include <vector>

struct AnimationFrame
{
  UV uv;
  std::chrono::milliseconds dt;
};

struct Animation
{
  public:
    inline const AnimationFrame& frame() const noexcept
    {
      return frames[frame_index];
    }

    bool Tick(std::chrono::milliseconds elapsed) noexcept;

    void ShiftFrame() noexcept;

    void ResetFrame() noexcept;

  public:
    std::vector<AnimationFrame> frames;
    size_t frame_index;
    std::chrono::milliseconds deadline;
};
