#pragma once

#include <d3d11.h>
#include <wrl/client.h>

//#include <span>
#include <vector>

class IndexBuffer
{
  public:
    using ContextPtr = Microsoft::WRL::ComPtr<ID3D11DeviceContext>;
    using DevicePtr = Microsoft::WRL::ComPtr<ID3D11Device>;

  public:
    IndexBuffer() = default;
    IndexBuffer(const IndexBuffer&) = default;
    IndexBuffer(IndexBuffer&&) = default;
    IndexBuffer(const DevicePtr& device, const std::vector<uint16_t>& indices);

    IndexBuffer& operator=(const IndexBuffer&) = default;

    // Don't have span yet
    //IndexBuffer(const DevicePtr& device, std::span<uint16_t> indices);

    void Update(const DevicePtr& device, const std::vector<uint16_t>& indices);
    void Bind(const ContextPtr& context) const;
    size_t size() const;

  private:
    size_t size_ = 0;
    Microsoft::WRL::ComPtr<ID3D11Buffer> buffer_;
};
