#pragma once

#include <libcommon/minimal_win.hpp>
#include <d3d11.h>
#include <wrl/client.h>

#include <array>
#include <vector>
#include <chrono>

struct UV
{
  float u;
  float v;
};

struct UVAligned
{
    UV uv;
    float padding[2] = {};
};

struct AnimationFrame
{
  using dt_t = std::chrono::duration<uint64_t, std::milli>;

  UV uv;
  dt_t dt;
};

struct Animation
{
  std::vector<AnimationFrame> frames;

  size_t frame_index;
  AnimationFrame::dt_t deadline;

  bool Tick(AnimationFrame::dt_t elapsed);

  const AnimationFrame& frame() const noexcept { return frames[frame_index]; }

  void ShiftFrame()
  {
    frame_index = (frame_index + 1) % frames.size();
    deadline = frames[frame_index].dt; 
  }
};


struct Vertex
{
  float x;
  float y;
  float u;
  float v;
};

struct RawImage
{
  size_t width = 0;
  size_t height = 0;

  // for debug
  size_t row_size{};

  std::vector<uint8_t> data;
};


class Graphics
{
  public:
    Graphics(UINT w, UINT h, HWND hwnd);
    Graphics(const Graphics&) = delete;
    Graphics(Graphics&&) = delete;

    void LoadShaders();
    void LoadPNG();
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
    Animation dino_animation_;
    std::chrono::steady_clock::time_point last_frame_ = std::chrono::steady_clock::now();

  private:
    //win32::DxgiInfoManager info_manager_;

    RawImage sprite_;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture_view_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> buffer_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> uv_buffer_;
    size_t draw_size_ = 0;
    Microsoft::WRL::ComPtr<ID3D11Buffer> index_buffer_;


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
    Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_;
};
