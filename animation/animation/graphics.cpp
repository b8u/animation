#include "graphics.hpp"
#include "settings.hpp"

#include <d3dcompiler.h>

#include <sstream>
#include <memory>
#include <optional>
#include <filesystem>
#include <iostream>
#include <cassert>

namespace fs = std::filesystem;
namespace MS = Microsoft;

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


void DrawableObject::Draw(const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& context) const
{
  const UINT stride = sizeof Vertex;
  const UINT offset = 0u;
  context->IASetVertexBuffers(0u, 1u, vertices.GetAddressOf(), &stride, &offset);
  context->IASetIndexBuffer(indices.Get(), DXGI_FORMAT_R16_UINT, 0u);
  context->VSSetConstantBuffers(0u, 1u, cbuffer.GetAddressOf());
  context->PSSetShaderResources(0u, 1u, texture_view.GetAddressOf());
  context->PSSetSamplers(0u, 1u, sampler.GetAddressOf());

  context->DrawIndexed(draw_list_size , 0u, 0u);
}

std::array<Animation, dino::total> BuildAnimations()
{
  using namespace std::chrono_literals;

  std::array<Animation, dino::total> res;

  size_t index = 0;
  int count = 0;
  for (int i = 0; i < 576; i += 24) {
    if (count >= 4) {
      ++index;
      count = 0;
      assert(index < static_cast<size_t>(dino::total));
    }
    ++count;

    AnimationFrame frame{ { i / 576.0f, 1.0f }, 150ms };
    res[index].frames.push_back(frame);
    res[index].ResetFrame();
  }

  return res;
}

Graphics::Graphics(UINT w, UINT h, HWND hwnd)
{

    const DXGI_SWAP_CHAIN_DESC swapDesc = CreateSwapChainDesc(w, h, hwnd);

    UINT swapCreateFlags = 0u;
#ifndef NDEBUG
    swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif  // !NDEBUG

    D3D_FEATURE_LEVEL feature_level{};

    if (HResult res = D3D11CreateDeviceAndSwapChain(
            nullptr,                   // _In_opt_                            IDXGIAdapter*         pAdapter,
            D3D_DRIVER_TYPE_HARDWARE,  //                                     D3D_DRIVER_TYPE       DriverType,
            nullptr,                   //                                     HMODULE               Software,
            swapCreateFlags,           //                                     UINT                  Flags,
            nullptr,                   // _In_reads_opt_(FeatureLevels) CONST D3D_FEATURE_LEVEL*    pFeatureLevels,
            0,                         //                                     UINT FeatureLevels,
            D3D11_SDK_VERSION,         //                                     UINT                  SDKVersion,
            &swapDesc,                 // _In_opt_                      CONST DXGI_SWAP_CHAIN_DESC* pSwapChainDesc,
            &swapchain_,               // _COM_Outptr_opt_                    IDXGISwapChain**      ppSwapChain,
            &device_,                  // _COM_Outptr_opt_                    ID3D11Device**        ppDevice,
            &feature_level,            // _Out_opt_                           D3D_FEATURE_LEVEL*    pFeatureLevel,
            &context_);                // _COM_Outptr_opt_                    ID3D11DeviceContext** ppImmediateContext
        !res) {
        //throw Exception(res, __LINE__, __FILE__);
    }

    {
        std::stringstream oss;
        oss << "Feature Level: 0x" << std::hex << feature_level << "\n";
        std::string feature_level_str = oss.str();
        OutputDebugString(feature_level_str.c_str());
    }

    // gain access to texture subresource in swap chain (back buffer)
    MS::WRL::ComPtr<ID3D11Resource> back_buffer;
    if (HResult res = swapchain_->GetBuffer(0, __uuidof(ID3D11Resource), &back_buffer); !res) {
        //throw Exception(res, __LINE__, __FILE__);
    }
    if (HResult res = device_->CreateRenderTargetView(back_buffer.Get(),
                                                             nullptr,  // _In_opt_ const D3D11_RENDER_TARGET_VIEW_DESC*
                                                             &target_view_);
        !res) {
        //throw Exception(res, __LINE__, __FILE__);
    }

    // create depth stensil state
    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = TRUE;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
    MS::WRL::ComPtr<ID3D11DepthStencilState> pDSState;
    device_->CreateDepthStencilState(&dsDesc, &pDSState);

    // bind depth state
    context_->OMSetDepthStencilState(pDSState.Get(), 1u);


    // create depth stensil texture
    MS::WRL::ComPtr<ID3D11Texture2D> depthStencil;
    D3D11_TEXTURE2D_DESC descDepth{};
    descDepth.Width = w;
    descDepth.Height = h;
    descDepth.MipLevels = 1u;
    descDepth.ArraySize = 1u;
    descDepth.Format = DXGI_FORMAT_D32_FLOAT;
    descDepth.SampleDesc.Count = 1u;
    descDepth.SampleDesc.Quality = 0u;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    if (const HResult res = device_->CreateTexture2D(&descDepth, nullptr, &depthStencil);
        !res) {
        //throw Exception(res, __LINE__, __FILE__);
    }

    // create view of depth stensil texture
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
    descDSV.Format = DXGI_FORMAT_D32_FLOAT;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0u;
    if (const HResult res = device_->CreateDepthStencilView(
          depthStencil.Get(),
          &descDSV,
          &depth_stencil_view_); !res) {
        //throw Exception(res, __LINE__, __FILE__);
    }


    // bind depth stensil view to OM (but we don't have it yet).
    context_->OMSetRenderTargets(1u,                           // UINT NumViews
                                 target_view_.GetAddressOf(),  //
                                 depth_stencil_view_.Get());   // ID3D11DepthStencilView * pDepthStencilView

    {
      auto [texture, texture_view] = LoadPNG("D:\\cppprjs\\animation\\assets\\DinoSprites - vita.png", device_);
      dino_.texture      = texture      ;
      dino_.texture_view = texture_view ;
    }
    {
      auto [texture, texture_view] = LoadPNG("D:\\cppprjs\\animation\\assets\\Sprites\\Background\\sky.png", device_);
      sky_.texture      = texture      ;
      sky_.texture_view = texture_view ;
    }

    //Creating Sampler
    {
      D3D11_SAMPLER_DESC desc = {};
      desc.Filter         = D3D11_FILTER_MIN_MAG_MIP_POINT;
      desc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
      desc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
      desc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;

      auto result = device_->CreateSamplerState(&desc, &dino_.sampler);
      //check that sampler state created correctly.
      assert(SUCCEEDED(result) && "problem creating sampler\n");

      sky_.sampler = dino_.sampler;
    }

    LoadShaders();
    SetVertices();


    // we are going to use this shit for all objects.
    context_->IASetInputLayout(input_layout_.Get());
    context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context_->VSSetShader(vertex_shader_.Get(), nullptr, 0u);
    context_->PSSetShader(pixel_shader_.Get(), nullptr, 0u);
}

void Graphics::LoadShaders()
{
    if (HResult res = D3DReadFileToBlob(LR"(animation\shaders\vsd.fxc)", &vertex_shader_blob_); !res)
    {
      std::cerr << __func__ << ":" << __LINE__ << std::endl;
      std::terminate();
    }
    if (HResult res = device_->CreateVertexShader(
          vertex_shader_blob_->GetBufferPointer(),
          vertex_shader_blob_->GetBufferSize(),
          nullptr,
          &vertex_shader_); !res)
    {
      std::cerr << __func__ << ":" << __LINE__ << " error: " << std::hex << static_cast<HRESULT>(res) << std::endl;
      std::terminate();
    }
     
    Microsoft::WRL::ComPtr<ID3DBlob> bytecode_blob_;
    if (HResult res = D3DReadFileToBlob(LR"(animation\shaders\psd.fxc)", &bytecode_blob_); !res)
    {
      std::cerr << __func__ << ":" << __LINE__ << std::endl;
      std::terminate();
    }
    if (HResult res = device_->CreatePixelShader(bytecode_blob_->GetBufferPointer(), bytecode_blob_->GetBufferSize(), nullptr, &pixel_shader_); !res)
    {
      std::cerr << __func__ << ":" << __LINE__ << std::endl;
      std::terminate();
    }

}

std::pair<Microsoft::WRL::ComPtr<ID3D11Texture2D>,
          Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>
Graphics::LoadPNG(const fs::path& path, const Microsoft::WRL::ComPtr<ID3D11Device>& device)
{
  const auto image = b8u::RawImage::Load(path);
  if (image) {
    std::cout << "Image: " << image->width << "x" << image->height << ", row size: " << image->row_size << "\n";
  } else {
    std::cerr << "loadPngImage failed\n";
    return {};
  }

  //Creating shader resource view
  D3D11_TEXTURE2D_DESC desc;
  ZeroMemory(&desc, sizeof(desc));
  desc.Width              = image->width; // in texels
  desc.Height             = image->height;
  desc.MipLevels          = 1;
  desc.ArraySize          = 1;
  desc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
  desc.SampleDesc.Count   = 1;
  desc.SampleDesc.Quality = 0;
  desc.Usage              = D3D11_USAGE_DEFAULT;
  desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;
  desc.CPUAccessFlags     = 0;
  desc.MiscFlags = 0;

  D3D11_SUBRESOURCE_DATA subResource;
  subResource.pSysMem = image->data.data();
  subResource.SysMemPitch = desc.Width * 4;
  subResource.SysMemSlicePitch = 0;

  Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
  auto result = device->CreateTexture2D(&desc, &subResource, &texture);
  //check to make sure that texture is created correctly
  assert(SUCCEEDED(result) && "issue creating texture\n");


  // Create texture view
  D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
  srvDesc.Format                    = desc.Format;
  srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MipLevels       = desc.MipLevels;
  srvDesc.Texture2D.MostDetailedMip = 0;

  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> texture_view;
  result = device->CreateShaderResourceView(texture.Get(), &srvDesc, &texture_view);
  //check to make sure that resource view is created correctly
  assert(SUCCEEDED(result) && "issue creating shaderResourceView \n");

  return { texture, texture_view };
}

void Graphics::SetVertices()
{

  /*
   *   1-----2
   *   |\    |
   *   | \   |
   *   |  \  |
   *   |   \ |
   *   |    \|
   *   4-----3
   *
   *   [-1; 1] - [1; -1]
   *
   *   1: (-0.9f,  0.9f)   2: (0.9f,  0.9f)
   *   4: (-0.9f, -0.9f)   3: (0.9f, -0.9f)
   */


  // vertices
  {
    const float ratio = g_settings->ratio;
    const Vertex vertices[] =
      //     x ,             y ,             u ,   v
    { { -0.12f ,  0.12f / ratio,           0.0f, 0.0f } // 1  
    , {  0.12f ,  0.12f / ratio, 24.0f / 576.0f, 0.0f } // 2
    , {  0.12f , -0.12f / ratio, 24.0f / 576.0f, 1.0f } // 3
    , { -0.12f , -0.12f / ratio,           0.0f, 1.0f } // 4
    };

    D3D11_BUFFER_DESC bd = {};
    bd.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
    bd.Usage               = D3D11_USAGE_DEFAULT;
    bd.CPUAccessFlags      = 0u;
    bd.MiscFlags           = 0u;
    bd.ByteWidth           = sizeof( vertices );
    bd.StructureByteStride = sizeof( Vertex );

    D3D11_SUBRESOURCE_DATA sd = {};
    sd.pSysMem = vertices;

    device_->CreateBuffer(&bd, &sd, &dino_.vertices);
  }

  // sky buffer
  {
    const Vertex sky_vertices[] =
    { { -1.0f,  1.0f, 0.0f, 0.0f } // 1
    , {  1.0f,  1.0f, 1.0f, 0.0f } // 2
    , {  1.0f, -1.0f, 1.0f, 1.0f } // 3
    , { -1.0f, -1.0f, 0.0f, 1.0f } // 3
    };

    D3D11_BUFFER_DESC bd = {};
    bd.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
    bd.Usage               = D3D11_USAGE_DEFAULT;
    bd.CPUAccessFlags      = 0u;
    bd.MiscFlags           = 0u;
    bd.ByteWidth           = sizeof sky_vertices;
    bd.StructureByteStride = sizeof Vertex;

    D3D11_SUBRESOURCE_DATA sd = {};
    sd.pSysMem = sky_vertices;

    device_->CreateBuffer(&bd, &sd, &sky_.vertices);

  }


  // indices
  {
    const unsigned short indices[] =
    { 0, 1, 2
    , 0, 2, 3
    };

    dino_.draw_list_size = std::size(indices);
    sky_.draw_list_size = std::size(indices);

    D3D11_BUFFER_DESC ibd = {};
    ibd.BindFlags           = D3D11_BIND_INDEX_BUFFER;
    ibd.Usage               = D3D11_USAGE_IMMUTABLE;
    ibd.CPUAccessFlags      = 0u;
    ibd.MiscFlags           = 0u;
    ibd.ByteWidth           = sizeof(indices);
    ibd.StructureByteStride = sizeof(unsigned short);

    D3D11_SUBRESOURCE_DATA isd = {};
    isd.pSysMem = indices;

    device_->CreateBuffer(&ibd, &isd, &dino_.indices);

    sky_.indices = dino_.indices;
  }

  // uv animation
  {
    Vertex default_offset{};

    D3D11_BUFFER_DESC uv_desc = {};
    uv_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    uv_desc.Usage = D3D11_USAGE_DYNAMIC;
    uv_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    uv_desc.ByteWidth = sizeof default_offset;
    uv_desc.StructureByteStride = sizeof Vertex;

    D3D11_SUBRESOURCE_DATA uv_sd = {};
    uv_sd.pSysMem = &default_offset;

    if (HResult res = device_->CreateBuffer(&uv_desc, &uv_sd, &dino_.cbuffer); !res)
    {
      std::cerr << __func__ << ":" << __LINE__ << " error: " << std::hex << static_cast<HRESULT>(res) << std::endl;
      std::terminate();
    }

    if (HResult res = device_->CreateBuffer(&uv_desc, &uv_sd, &sky_.cbuffer); !res)
    {
      std::cerr << __func__ << ":" << __LINE__ << " error: " << std::hex << static_cast<HRESULT>(res) << std::endl;
      std::terminate();
    }
  }

  // layout
  const D3D11_INPUT_ELEMENT_DESC ied[] =
  { { "Position", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
  , { "TexCoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(float) * 2, D3D11_INPUT_PER_VERTEX_DATA, 0 }
  };

  if (HResult res = device_->CreateInputLayout(
        ied,
        (UINT)std::size(ied),
        vertex_shader_blob_->GetBufferPointer(),
        vertex_shader_blob_->GetBufferSize(),
        &input_layout_); !res) {
    std::cerr << __func__ << ":" << __LINE__ << " error: " << std::hex << static_cast<HRESULT>(res) << std::endl;
    std::terminate();
  }
}

void Graphics::DrawAllThisShit()
{
  auto dt = std::chrono::steady_clock::now() - last_frame_;
  last_frame_ = std::chrono::steady_clock::now();

  std::chrono::milliseconds tmp = std::chrono::duration_cast<std::chrono::milliseconds>(dt);
  //std::cout << "Tick(" << tmp.count() << ")" << std::endl;
  if (animations_[1].Tick(tmp))
  {
    D3D11_MAPPED_SUBRESOURCE mapped;
    if (HResult res = context_->Map(dino_.cbuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped); !res)
    {
      std::cerr << __func__ << ":" << __LINE__ << " error: " << std::hex << static_cast<HRESULT>(res) << std::endl;
      std::terminate();
    }

    Vertex newUV{};

    newUV.u = animations_[1].frame().uv.u;
    newUV.v = animations_[1].frame().uv.v;

    if (g_settings->left ) { dino_position_.x -= 0.03f; }
    if (g_settings->right) { dino_position_.x += 0.03f; }

    newUV.x = dino_position_.x;
    newUV.y = dino_position_.y;
    memcpy(mapped.pData, &newUV, sizeof newUV);

    context_->Unmap(dino_.cbuffer.Get(), 0);
  }


  // configure viewport
  {
    D3D11_VIEWPORT vp;
    vp.Width    = g_settings->width();
    vp.Height   = g_settings->height();
    vp.MinDepth = 0;
    vp.MaxDepth = 1;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    context_->RSSetViewports(1u, &vp);
  }

  dino_.Draw(context_);
  sky_.Draw(context_);

}

void Graphics::EndFrame()
{
    if (HResult res = swapchain_->Present(1u, 0u); !res) {
        if (res == DXGI_ERROR_DEVICE_REMOVED) {
            // throw GFX_DEVICE_REMOVED_EXCEPT(pDevice->GetDeviceRemovedReason());
            std::terminate();
        } else {
//            throw Exception(res, __LINE__, __FILE__);
        }
    }
}

void Graphics::BeginFrame(float red, float green, float blue) noexcept
{
    const float color[] = {red, green, blue, 1.0f};
    context_->ClearRenderTargetView(target_view_.Get(), color);
    context_->ClearDepthStencilView(depth_stencil_view_.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);
}

DXGI_SWAP_CHAIN_DESC Graphics::CreateSwapChainDesc(UINT w, UINT h, HWND hwnd) noexcept
{
    return {
        // DXGI_MODE_DESC BufferDesc;
        {
            w,
            h,

            // DXGI_RATIONAL RefreshRate;
            {
                60,  // UINT Numerator;
                1    // UINT Denominator;
            },

            DXGI_FORMAT_R8G8B8A8_UNORM,            // DXGI_FORMAT Format;
            DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,  // DXGI_MODE_SCANLINE_ORDER ScanlineOrdering;
            DXGI_MODE_SCALING_UNSPECIFIED,         // DXGI_MODE_SCALING Scaling;
        },

        // DXGI_SAMPLE_DESC SampleDesc
        {
            1,  // UINT Count
            0   // UINT Quality
        },

        DXGI_USAGE_RENDER_TARGET_OUTPUT,  // DXGI_USAGE BufferUsage;
        1,                                // UINT BufferCount;
        hwnd,                             // HWND OutputWindow;
        TRUE,                             // BOOL Windowed;
        DXGI_SWAP_EFFECT_DISCARD,         // DXGI_SWAP_EFFECT SwapEffect;
        0,                                // UINT Flags;
    };
}
