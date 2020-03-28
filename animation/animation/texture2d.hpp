#pragma once

#include <animation/raw_image.hpp>
#include <animation/hresult.hpp>

#include <d3d11.h>
#include <wrl/client.h>


class Texture2D
{
  public:
    using ContextPtr = Microsoft::WRL::ComPtr<ID3D11DeviceContext>;
    using DevicePtr = Microsoft::WRL::ComPtr<ID3D11Device>;

  public:
    Texture2D() = default;
    Texture2D(const Texture2D&) = default;
    Texture2D(Texture2D&&) = default;

    Texture2D(Microsoft::WRL::ComPtr<ID3D11Texture2D> texture, DevicePtr& device_);
    Texture2D(const b8u::RawImage& image, DevicePtr& device_);

    Texture2D& operator=(const Texture2D&) = default;
    Texture2D& operator=(Texture2D&&) = default;

    operator bool() const { return texture_ && texture_view_; }

    const auto& texture_view() const noexcept { return texture_view_; }

  private:
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture_view_;
};


