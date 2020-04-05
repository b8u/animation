#pragma once

#include <array>

#include <animation/texture2d.hpp>
#include <animation/graphics_primitives.hpp>
#include <animation/graphics.hpp>

struct SpriteFrame : Box2Df
{
};

class Sptite
{
  public:
   Sptite(Texture2D sprite_sheet, SpriteFrame frame = kNormalizedBox)
     : sprite_sheet_(std::move(sprite_sheet))
   {
   }

   const SpriteFrame& frame() const { return current_frame_; }
   void frame(const SpriteFrame& newFrame) { current_frame_ = newFrame; }

  private:
    Texture2D sprite_sheet_;
    SpriteFrame current_frame_ = kNormalizedBox;
};
