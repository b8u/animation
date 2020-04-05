#pragma once
#include <animation/hresult.hpp>

#include <tuple>

#include <d3d11.h>
#include <wrl/client.h>

template <typename T>
class ConstantBuffer
{
  public:
    using DevicePtr = Microsoft::WRL::ComPtr<ID3D11Device>;
    using ContextPtr = Microsoft::WRL::ComPtr<ID3D11DeviceContext>;

    using value_type = T;

  public:
    ConstantBuffer() = default;

    void Bind(const ContextPtr& context) const
    {
      context->VSSetConstantBuffers(0u, 1u, cbuffer_.GetAddressOf());
    }

    void Update(const ContextPtr& context, const value_type& value)
    {
      if (value != buffer_)
      {
        buffer_ = value;
        D3D11_MAPPED_SUBRESOURCE mapped;
        if (HResult res = context->Map(cbuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped); !res)
        {
          std::cerr << __func__ << ":" << __LINE__ << " error: " << std::hex << static_cast<HRESULT>(res) << std::endl;
          std::terminate();
        }
        memcpy(mapped.pData, &buffer_, sizeof buffer_);

        context->Unmap(cbuffer_.Get(), 0);

      }
    }



  protected:
    value_type buffer_{};
    Microsoft::WRL::ComPtr<ID3D11Buffer> cbuffer_;
    bool is_dirty = false;
};


struct PerFrameBuffer
{
  float x = 0.0f;
  float y = 0.0f;
  float u = 0.0f;
  float v = 0.0f;

  PerFrameBuffer& operator=(const PerFrameBuffer& other) noexcept
  {
    std::tie(x, y, u, v) = std::tie(other.x, other.y, other.u, other.v);
    return *this;
  }
  bool operator!=(const PerFrameBuffer& other) const noexcept
  {
    return std::tie(x, y, u, v) != std::tie(other.x, other.y, other.u, other.v);
  }
};

class PerFrameConstantBuffer : public ConstantBuffer<PerFrameBuffer>
{
  public:
    PerFrameConstantBuffer () = default;
    PerFrameConstantBuffer(const DevicePtr& device);
    void Init(const DevicePtr& device);

  private:
};
