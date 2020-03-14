#include "graphics.hpp"
#include "settings.hpp"
#include "raw_image.hpp"


#include <sstream>
#include <memory>
#include <cassert>
#include <optional>
#include <filesystem>
#include <iostream>

#include <d3dcompiler.h>


bool Animation::Tick(std::chrono::milliseconds elapsed)
{
  //std::cout << "Tick: " << elapsed.count() << ", deadline: " << deadline.count() << std::endl;

  if (deadline > elapsed) {
    deadline -= elapsed;
    return false;
  } 

  elapsed -= deadline;

  ShiftFrame();
  Tick(elapsed);
  return true;
}


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

Graphics::Graphics(UINT w, UINT h, HWND hwnd)
{
  int count = 0;
  for (int i = 0; i < 576; i += 24)
  {
    using namespace std::chrono_literals;
    if (count >= 4) break; ++count;
    AnimationFrame frame{ { i / 576.0f, 1.0f }, 150ms };
    dino_animation_.frames.push_back(frame);
  }

  dino_animation_.ResetFrame();

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

    LoadPNG();
    LoadShaders();
    SetVertices();


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

void Graphics::LoadPNG()
{
  // 576 x 24
  const fs::path doux = "D:\\cppprjs\\animation\\assets\\DinoSprites - doux.png";
  const auto image = b8u::RawImage::Load(doux);
  if (image) {
    std::cout << "Image: " << image->width << "x" << image->height << ", row size: " << image->row_size << "\n";
    sprite_ = std::move(*image);
  }
  else {
    std::cerr << "loadPngImage failed\n";
    return;
  }


  {
    //Creating shader resource view
    {
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
      auto result = device_->CreateTexture2D(&desc, &subResource, &texture_);
      //check to make sure that texture is created correctly
      assert(SUCCEEDED(result) && "issue creating texture\n");


      // Create texture view
      D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
      srvDesc.Format                    = desc.Format;
      srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
      srvDesc.Texture2D.MipLevels       = desc.MipLevels;
      srvDesc.Texture2D.MostDetailedMip = 0;
      result = device_->CreateShaderResourceView(texture_.Get(), &srvDesc, &texture_view_);
      //check to make sure that resource view is created correctly
      assert(SUCCEEDED(result) && "issue creating shaderResourceView \n");

    }
    //Creating Sampler
    {
      D3D11_SAMPLER_DESC desc = {};
      desc.Filter         = D3D11_FILTER_MIN_MAG_MIP_POINT;
      desc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
      desc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
      desc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;

      auto result = device_->CreateSamplerState(&desc, &sampler_);
      //check that sampler state created correctly.
      assert(SUCCEEDED(result) && "problem creating sampler\n");
    }
    //return true;
  }

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

    const float ratio = g_settings->ratio;
    const Vertex vertices[] =
      //     x ,    y ,          u ,   v
      { { -0.9f * ratio,  0.9f,           0.0f, 0.0f } // 1  
      , {  0.9f * ratio,  0.9f, 24.0f / 576.0f, 0.0f } // 2
      , {  0.9f * ratio, -0.9f, 24.0f / 576.0f, 1.0f } // 3
      , { -0.9f * ratio, -0.9f,           0.0f, 1.0f } // 4
      };

    const unsigned short indices[] =
      { 0, 1, 2
      , 0, 2, 3
      };

    draw_size_ = std::size(indices);

    // vertices
    D3D11_BUFFER_DESC bd = {};
    bd.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
    bd.Usage               = D3D11_USAGE_DEFAULT;
    bd.CPUAccessFlags      = 0u;
    bd.MiscFlags           = 0u;
    bd.ByteWidth           = sizeof( vertices );
    bd.StructureByteStride = sizeof( Vertex );

    D3D11_SUBRESOURCE_DATA sd = {};
    sd.pSysMem = vertices;

    device_->CreateBuffer(&bd, &sd, &buffer_);

    // indices
    D3D11_BUFFER_DESC ibd = {};
    ibd.BindFlags           = D3D11_BIND_INDEX_BUFFER;
    ibd.Usage               = D3D11_USAGE_DEFAULT;
    ibd.CPUAccessFlags      = 0u;
    ibd.MiscFlags           = 0u;
    ibd.ByteWidth           = sizeof(indices);
    ibd.StructureByteStride = sizeof(unsigned short);

    D3D11_SUBRESOURCE_DATA isd = {};
    isd.pSysMem = indices;

    device_->CreateBuffer(&ibd, &isd, &index_buffer_);

    // uv animation
    {
      UVAligned default_offset{};

      D3D11_BUFFER_DESC uv_desc = {};
      uv_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
      uv_desc.Usage = D3D11_USAGE_DYNAMIC;
      uv_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
      uv_desc.ByteWidth = sizeof default_offset;
      uv_desc.StructureByteStride = sizeof UVAligned;

      D3D11_SUBRESOURCE_DATA uv_sd = {};
      uv_sd.pSysMem = &default_offset;

      if (HResult res = device_->CreateBuffer(&uv_desc, &uv_sd, &uv_buffer_); !res)
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
  assert(buffer_ && "vertex buffer is empty");


  auto dt = std::chrono::steady_clock::now() - last_frame_;
  last_frame_ = std::chrono::steady_clock::now();


  // TODO: do it in draw call
  const UINT stride = sizeof(Vertex);
  const UINT offset = 0u;


  // TODO: handle animation
  std::chrono::milliseconds tmp = std::chrono::duration_cast<std::chrono::milliseconds>(dt);
  //std::cout << "Tick(" << tmp.count() << ")" << std::endl;
  if (dino_animation_.Tick(tmp))
  {
    D3D11_MAPPED_SUBRESOURCE mapped;
    if (HResult res = context_->Map(uv_buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped); !res)
    {
      std::cerr << __func__ << ":" << __LINE__ << " error: " << std::hex << static_cast<HRESULT>(res) << std::endl;
      std::terminate();
    }

    static UVAligned newUV{ {0.0f, 0.0f} };
    newUV.uv = dino_animation_.frame().uv;
    memcpy(mapped.pData, &newUV, sizeof newUV);

    context_->Unmap(uv_buffer_.Get(), 0);
  }

  context_->IASetVertexBuffers(0u, 1u, buffer_.GetAddressOf(), &stride, &offset);
  context_->IASetIndexBuffer(index_buffer_.Get(), DXGI_FORMAT_R16_UINT, 0u);
  context_->IASetInputLayout(input_layout_.Get());
  context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  context_->VSSetShader(vertex_shader_.Get(), nullptr, 0u);
  context_->VSSetConstantBuffers(0u, 1u, uv_buffer_.GetAddressOf());
  context_->PSSetShader(pixel_shader_.Get(), nullptr, 0u);
  context_->PSSetShaderResources(0u, 1u, texture_view_.GetAddressOf());
  context_->PSSetSamplers(0u, 1u, sampler_.GetAddressOf());

  // configure viewport
  {
    D3D11_VIEWPORT vp;
    vp.Width    = g_settings->width();
    vp.Height   = g_settings->height();
    vp.MinDepth = 0;
    vp.MaxDepth = 1;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    context_->RSSetViewports( 1u,&vp );
  }

  context_->DrawIndexed((UINT)draw_size_, 0u, 0u);
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
