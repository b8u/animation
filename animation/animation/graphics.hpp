#pragma once

#include <libcommon/minimal_win.hpp>

#include <animation/raw_image.hpp>
#include <animation/uv.hpp>
#include <animation/sprite_animation.hpp>

#include <d3d11.h>
#include <wrl/client.h>

#include <array>
#include <vector>
#include <chrono>

struct Point
{
  float x = 0.0f;
  float y = 0.0f;
};


struct Vertex
{
  float x = 0.0f;
  float y = 0.0f;
  float u = 0.0f;
  float v = 0.0f;
};

struct Box
{
  Vertex left_top;
  Vertex right_bottom;

};

enum dino : size_t
{
  idle   = 0,
  move   = 1,
  kick   = 2,
  hurt   = 3,
  crouch = 4,
  sneak  = 5,
  total,
};

std::array<Animation, dino::total> BuildAnimations();

using ContextPtr = Microsoft::WRL::ComPtr<ID3D11DeviceContext>;
using DevicePtr = Microsoft::WRL::ComPtr<ID3D11Device>;

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

class Texture2D
{
  public:
    Texture2D() = default;

    Texture2D(Microsoft::WRL::ComPtr<ID3D11Texture2D> texture, DevicePtr& device_)
      : texture_(std::move(texture))
    {
      D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
      srv_desc.Format                    = DXGI_FORMAT_R8G8B8A8_UNORM;
      srv_desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
      srv_desc.Texture2D.MipLevels       = 1;
      srv_desc.Texture2D.MostDetailedMip = 0;

      if (texture_)
        if (HResult result = device_->CreateShaderResourceView(texture.Get(), &srv_desc, &texture_view_); !result)
          throw "can't create a texture view";
    }

    Texture2D(const b8u::RawImage& image, DevicePtr& device_)
    {
      D3D11_TEXTURE2D_DESC desc = {};
      desc.Width              = image.width; // in texels
      desc.Height             = image.height;
      desc.MipLevels          = 1;
      desc.ArraySize          = 1;
      desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
      desc.SampleDesc.Count   = 1;
      desc.SampleDesc.Quality = 0;
      desc.Usage              = D3D11_USAGE_DEFAULT;
      desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
      desc.CPUAccessFlags     = 0;
      desc.MiscFlags = 0;

      D3D11_SUBRESOURCE_DATA sub_resource;
      sub_resource.pSysMem = image.data.data();
      sub_resource.SysMemPitch = desc.Width * 4;
      sub_resource.SysMemSlicePitch = 0;

      if (HResult result = device_->CreateTexture2D(&desc, &sub_resource, &texture_); !result) {
        throw "can't create a texture";
      }

      // Create texture view
      D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
      srv_desc.Format                    = desc.Format;
      srv_desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
      srv_desc.Texture2D.MipLevels       = desc.MipLevels;
      srv_desc.Texture2D.MostDetailedMip = 0;

      if (HResult result = device_->CreateShaderResourceView(texture_.Get(), &srv_desc, &texture_view_); !result) {
        throw "can't create a texture view";
      }
    }

    Texture2D(const Texture2D&) = default;
    Texture2D(Texture2D&&) = default;

    Texture2D& operator=(const Texture2D&) = default;
    Texture2D& operator=(Texture2D&&) = default;

    operator bool() const { return texture_ && texture_view_; }

    const auto& texture_view() const noexcept { return texture_view_; }

  private:
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture_view_;
};

struct DrawableObject
{
  Microsoft::WRL::ComPtr<ID3D11Buffer> vertices;
  Microsoft::WRL::ComPtr<ID3D11Buffer> indices;
  Microsoft::WRL::ComPtr<ID3D11Buffer> cbuffer;
  Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;

  Texture2D texture;

  int layer_number = 0;
  unsigned draw_list_size = 0;

  void Draw(const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& context) const;
};

class Graphics
{
  public:
    Graphics(UINT w, UINT h, HWND hwnd);
    Graphics(const Graphics&) = delete;
    Graphics(Graphics&&) = delete;

    void LoadShaders();

    static
    std::pair<Microsoft::WRL::ComPtr<ID3D11Texture2D>,
              Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>
    LoadPNG(const std::filesystem::path& path, const Microsoft::WRL::ComPtr<ID3D11Device>& device);

    void SetVertices();

    void DrawAllThisShit();

    void EndFrame();

    inline void BeginFrame(const std::array<float, 3>& color) noexcept
    {
      BeginFrame(color[0], color[1], color[2]);
    }

    void BeginFrame(float red = 0.0f, float green = 0.0f, float blue = 0.0f) noexcept;

  public:
    static DXGI_SWAP_CHAIN_DESC CreateSwapChainDesc(UINT w, UINT h, HWND hwnd) noexcept;

  private:
    std::array<Animation, dino::total> animations_ = BuildAnimations();
    std::chrono::steady_clock::time_point last_frame_ = std::chrono::steady_clock::now();

  private:
    //win32::DxgiInfoManager info_manager_;

    Point dino_position_;

    DrawableObject dino_;
    DrawableObject sky_;
    DrawableObject clouds_;
    DrawableObject trees_back_;
    DrawableObject trees_front_;
    DrawableObject mountains_;
    DrawableObject ground_;
    DrawableObject grass_;

    Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout_;

  private:
    Microsoft::WRL::ComPtr<ID3D11Device> device_;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swapchain_;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> target_view_;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depth_stencil_view_;


    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader_;
    Microsoft::WRL::ComPtr<ID3DBlob> vertex_shader_blob_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader_;
};
