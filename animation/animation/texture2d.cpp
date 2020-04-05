#include "texture2d.hpp"

Texture2D::Texture2D(
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture,
    DevicePtr& device_)
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

Texture2D::Texture2D(const b8u::RawImage& image, DevicePtr& device_)
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

void Texture2D::Bind(const ContextPtr& context) const
{
  context->PSSetShaderResources(0u, 1u, texture_view().GetAddressOf());
}
