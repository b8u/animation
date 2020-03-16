#include "sprite_animation.hpp"

#include <cassert>

bool Animation::Tick(std::chrono::milliseconds elapsed) noexcept
{
  if (deadline > elapsed) {
    deadline -= elapsed;
    return false;
  } 

  elapsed -= deadline;

  ShiftFrame();
  Tick(elapsed);
  return true;
}

void Animation::ShiftFrame() noexcept
{
  frame_index = (frame_index + 1) % frames.size();
  deadline = frames[frame_index].dt; 
}

void Animation::ResetFrame() noexcept
{
  assert(!frames.empty());
  frame_index = frames.size() - 1;
  ShiftFrame();
}
