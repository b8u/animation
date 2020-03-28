#pragma once

#include <libcommon/minimal_win.hpp>

#include <animation/raw_image.hpp>
#include <animation/uv.hpp>
#include <animation/sprite_animation.hpp>
#include <animation/texture2d.hpp>
#include <animation/hresult.hpp>

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

struct DrawableObject
{
  Microsoft::WRL::ComPtr<ID3D11Buffer> vertices;
  Microsoft::WRL::ComPtr<ID3D11Buffer> indices;
  Microsoft::WRL::ComPtr<ID3D11Buffer> cbuffer;
  Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;

  Texture2D texture;

  unsigned draw_list_size = 0;

  void Draw(const ContextPtr& context) const;
};

struct CharacterObject : DrawableObject
{
  enum class Direction
  {
    left,
    right,
  };

  enum class State
  {
    normal,
    jump,
    run_start,
    run,
  };

  Direction direction = Direction::right;
  State state = State::normal;
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

    CharacterObject dino_;
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


    // Unique stuff
    Microsoft::WRL::ComPtr<ID3D11SamplerState> repeat_sampler_;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> mirror_sampler_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> static_offset_;

};
