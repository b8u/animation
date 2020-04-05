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



void DrawableObject::Draw(const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& context) const
{
  const UINT stride = sizeof Vertex;
  const UINT offset = 0u;
  context->IASetVertexBuffers(0u, 1u, vertices.GetAddressOf(), &stride, &offset);
  indices.Bind(context);
  cbuffer.Bind(context);
  context->PSSetShaderResources(0u, 1u, texture.texture_view().GetAddressOf());
  context->PSSetSamplers(0u, 1u, sampler.GetAddressOf());

  context->DrawIndexed(indices.size() , 0u, 0u);
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

    AnimationFrame frame{ { i / 576.0f, 0.0f }, 150ms };
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
    context_->OMSetRenderTargets(/* UINT NumViews */ 1u, target_view_.GetAddressOf(), depth_stencil_view_.Get());


  
  

    dino_.texture        = Texture2D{*b8u::RawImage::Load(R"(D:\cppprjs\animation\assets\DinoSprites - vita.png)"           ) , device_};
    sky_.texture         = Texture2D{*b8u::RawImage::Load(R"(D:\cppprjs\animation\assets\Sprites\Background\sky.png)"       ) , device_};
    clouds_.texture      = Texture2D{*b8u::RawImage::Load(R"(D:\cppprjs\animation\assets\Sprites\Background\cloud.png)"     ) , device_};
    mountains_.texture   = Texture2D{*b8u::RawImage::Load(R"(D:\cppprjs\animation\assets\Sprites\Background\mountain2.png)" ) , device_};
    trees_back_.texture  = Texture2D{*b8u::RawImage::Load(R"(D:\cppprjs\animation\assets\Sprites\Background\pine2.png)"     ) , device_};
    trees_front_.texture = Texture2D{*b8u::RawImage::Load(R"(D:\cppprjs\animation\assets\Sprites\Background\pine1.png)"     ) , device_};
    ground_.texture      = Texture2D{*b8u::RawImage::Load(R"(D:\cppprjs\animation\assets\Sprites\Tile\Ground\ground_11.png)") , device_};
    grass_.texture       = Texture2D{*b8u::RawImage::Load(R"(D:\cppprjs\animation\assets\Sprites\Tile\Ground\ground_2.png)" ) , device_};


    // Creating repeat sampler
    {
      D3D11_SAMPLER_DESC desc = {};
      desc.Filter         = D3D11_FILTER_MIN_MAG_MIP_POINT;
      desc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
      desc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
      desc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;

      auto result = device_->CreateSamplerState(&desc, &repeat_sampler_);
      //check that sampler state created correctly.
      assert(SUCCEEDED(result) && "problem creating sampler\n");

      grass_.sampler = ground_.sampler = clouds_.sampler = mountains_.sampler = trees_front_.sampler = trees_back_.sampler = sky_.sampler = repeat_sampler_;
    }

    {
      D3D11_SAMPLER_DESC desc = {};
      desc.Filter         = D3D11_FILTER_MIN_MAG_MIP_POINT;
      desc.AddressU       = D3D11_TEXTURE_ADDRESS_MIRROR;
      desc.AddressV       = D3D11_TEXTURE_ADDRESS_MIRROR;
      desc.AddressW       = D3D11_TEXTURE_ADDRESS_MIRROR;

      auto result = device_->CreateSamplerState(&desc, &mirror_sampler_);
      //check that sampler state created correctly.
      assert(SUCCEEDED(result) && "problem creating sampler\n");

      dino_.sampler = mirror_sampler_;
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


    auto createVertexBuffer = [](Microsoft::WRL::ComPtr<ID3D11Device>& device, const Box& tile) {
      const Vertex vertices[] =
      { { tile.left_top.x    ,  tile.left_top.y    , tile.left_top.u    ,  tile.left_top.v     } // 1
      , { tile.right_bottom.x,  tile.left_top.y    , tile.right_bottom.u,  tile.left_top.v     } // 2
      , { tile.right_bottom.x,  tile.right_bottom.y, tile.right_bottom.u,  tile.right_bottom.v } // 3
      , { tile.left_top.x    ,  tile.right_bottom.y, tile.left_top.u    ,  tile.right_bottom.v } // 3
      };

      D3D11_BUFFER_DESC bd = {};
      bd.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
      bd.Usage               = D3D11_USAGE_DEFAULT;
      bd.CPUAccessFlags      = 0u;
      bd.MiscFlags           = 0u;
      bd.ByteWidth           = sizeof vertices;
      bd.StructureByteStride = sizeof Vertex;

      D3D11_SUBRESOURCE_DATA sd = {};
      sd.pSysMem = vertices;
      Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
      device->CreateBuffer(&bd, &sd, &buffer);

      return buffer;
    };

  // vertices
  dino_.vertices = createVertexBuffer(device_,
      { { g_settings->pxToX(         40), g_settings->pxToY(391 - 24 * 2),           0.0f, 0.0f }
      , { g_settings->pxToX(40 + 24 * 2), g_settings->pxToY(         391), 24.0f / 576.0f, 1.0f }
      });

  sky_.vertices = createVertexBuffer(device_,
    { { g_settings->pxToX(                  0), g_settings->pxToY(  0), 0.0f, 0.0f } // 1
    , { g_settings->pxToX(g_settings->width()), g_settings->pxToY(211), 1.0f, 1.0f } // 3
    });

  clouds_.vertices = createVertexBuffer(device_,
    { { g_settings->pxToX(      27), g_settings->pxToY(      13), 0.0f, 0.0f } // 1
    , { g_settings->pxToX(27 + 634), g_settings->pxToY(13 + 136), 1.0f, 1.0f } // 3
    });

  mountains_.vertices = createVertexBuffer(device_,
    { { g_settings->pxToX(                  0), g_settings->pxToY(132), 0.0f, 0.0f } // 1
    , { g_settings->pxToX(g_settings->width()), g_settings->pxToY(259), 1.0f, 1.0f } // 3
    });

  trees_back_.vertices = createVertexBuffer(device_,
    { { g_settings->pxToX(                  0), g_settings->pxToY(192 - 20), 0.0f, 0.0f } // 1
    , { g_settings->pxToX(g_settings->width()), g_settings->pxToY(391 - 20), 1.0f, 1.0f } // 3
    });

  trees_front_.vertices = createVertexBuffer(device_,
    { { g_settings->pxToX(                  0), g_settings->pxToY(235), 0.0f, 0.0f } // 1
    , { g_settings->pxToX(g_settings->width()), g_settings->pxToY(383), 1.0f, 1.0f } // 3
    });

  ground_.vertices = createVertexBuffer(device_,
    { { g_settings->pxToX(                  0), g_settings->pxToY(383)                 , 0.0f, 0.0f } // 1
    , { g_settings->pxToX(g_settings->width()), g_settings->pxToY(g_settings->height()), float(g_settings->width()) / 16.0f, float(g_settings->height() - 383) / 16.0f } // 3
    });

  grass_.vertices = createVertexBuffer(device_,
    { { g_settings->pxToX(                  0), g_settings->pxToY(     383),                               0.0f, 0.0f } // 1
    , { g_settings->pxToX(g_settings->width()), g_settings->pxToY(383 + 16), float(g_settings->width()) / 16.0f, 1.0f } // 3
    });



  // indices
  {
    dino_.indices.Update(device_, { 0, 1, 2, 0, 2, 3 });
    grass_.indices.Update(device_, { 0, 1, 2, 0, 2, 3 });
    ground_.indices.Update(device_, { 0, 1, 2, 0, 2, 3 });
    clouds_.indices.Update(device_, { 0, 1, 2, 0, 2, 3 });
    mountains_.indices.Update(device_, { 0, 1, 2, 0, 2, 3 });
    trees_front_.indices.Update(device_, { 0, 1, 2, 0, 2, 3 });
    trees_back_.indices.Update(device_, { 0, 1, 2, 0, 2, 3 });
    sky_.indices.Update(device_, { 0, 1, 2, 0, 2, 3 });
  }

  // uv animation
  {
    dino_.cbuffer.Init(device_);
    mountains_.cbuffer.Init(device_);
    trees_back_.cbuffer.Init(device_);
    trees_front_.cbuffer.Init(device_);
    clouds_.cbuffer.Init(device_);
    ground_.cbuffer.Init(device_);
    grass_.cbuffer.Init(device_);
  }

  // layout
  const D3D11_INPUT_ELEMENT_DESC ied[] =
  { { "Position", 0, DXGI_FORMAT_R32G32_FLOAT, 0,                 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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


  const size_t animation_stage = static_cast<size_t>(dino::move);
  auto& animation = animations_[animation_stage];

  std::chrono::milliseconds tmp = std::chrono::duration_cast<std::chrono::milliseconds>(dt);
  //std::cout << "Tick(" << tmp.count() << ")" << std::endl;
  if (animation.Tick(tmp))
  {
    Vertex newUV{};

    if (g_settings->left ) {
      dino_position_.x -= 0.03f;
      dino_.direction = CharacterObject::Direction::left;
    } else if (g_settings->right) {
      dino_position_.x += 0.03f;
      dino_.direction = CharacterObject::Direction::right;
    }

    switch (dino_.direction)
    {
      case CharacterObject::Direction::left:
        newUV.u = -2.0f - animation.frame().uv.u;
        newUV.v = -2.0f - animation.frame().uv.v;
        break;

      case CharacterObject::Direction::right:
        newUV.u = animation.frame().uv.u;
        newUV.v = animation.frame().uv.v;
        break;
    }

    newUV.x = dino_position_.x;
    newUV.y = dino_position_.y;


    PerFrameBuffer pfb;
    pfb.x = newUV.x;
    pfb.y = newUV.y;
    pfb.u = newUV.u;
    pfb.v = newUV.v;

    dino_.cbuffer.Update(context_, pfb);
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
  grass_.Draw(context_);
  ground_.Draw(context_);
  trees_front_.Draw(context_);
  trees_back_.Draw(context_);
  mountains_.Draw(context_);
  clouds_.Draw(context_);
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
