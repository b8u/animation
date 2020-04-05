#pragma once

template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>
struct Point2D
{
  using element_type = std::remove_const_t<T>;
  element_type x{};
  element_type y{};


  Point2D& operator+=(const Point2D& other) noexcept
  {
    x += other.x;
    y += other.y;

    return *this;
  }

  void operator-() noexcept
  {
    x = -x;
    y = -y;
  }
};

template <typename T>
struct Box2D
{
  using point_type = Point2D<T>;
  using element_type = point_type::element_type;

  point_type lt;
  point_type rb;

  Box2D& Shift(const point_type& offset) noexcept
  {
    lt += offset;
    rb += offset;

    return *this;
  }
};

using Box2Df = Box2D<float>;
constexpr const Box2Df kNormalizedBox = { { 0.0f, 0.0f }, { 1.0f, 1.0f } };
