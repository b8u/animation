#pragma once

#include <libcommon/minimal_win.hpp>
#include <d3d11.h>
#include <wrl/client.h>

#include <array>
#include <vector>


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
    //win32::DxgiInfoManager info_manager_;

    RawImage sprite_;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture_view_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> buffer_;
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
