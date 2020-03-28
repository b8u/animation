#pragma once
#include <d3d11.h>

class HResult
{
  public:
    constexpr HResult() noexcept = default;
    constexpr HResult(const HResult&) = default;

    constexpr HResult(HRESULT value) noexcept : value_(value) {}
    constexpr HRESULT value() const noexcept { return value_; }

    inline constexpr operator bool() const noexcept { return SUCCEEDED(value()); }
    inline constexpr explicit operator HRESULT() const noexcept { return value(); }

    inline HResult& operator=(HRESULT value) noexcept
    {
      value_ = value;
      return *this;
    }

    inline bool operator==(HRESULT value) const noexcept { return value == value_; }

  private:
    HRESULT value_{};
};
