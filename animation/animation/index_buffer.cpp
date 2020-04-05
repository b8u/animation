#include "index_buffer.hpp"

#include <cassert>
#include <iostream>

//IndexBuffer::IndexBuffer(const DevicePtr& device, std::span<uint16_t> indices)
IndexBuffer::IndexBuffer(const DevicePtr& device, const std::vector<uint16_t>& indices)
{
  Update(device, indices);
}

void IndexBuffer::Update(const DevicePtr& device, const std::vector<uint16_t>& indices)
{
  assert(device);

  size_ = indices.size();

  D3D11_BUFFER_DESC desc = {};
  desc.BindFlags           = D3D11_BIND_INDEX_BUFFER;
  desc.Usage               = D3D11_USAGE_IMMUTABLE;
  desc.CPUAccessFlags      = 0u;
  desc.MiscFlags           = 0u;
  desc.ByteWidth           = indices.size() * sizeof(uint16_t);
  desc.StructureByteStride = sizeof(uint16_t);

  D3D11_SUBRESOURCE_DATA data = {};
  data.pSysMem = indices.data();

  device->CreateBuffer(&desc, &data, &buffer_);
}

void IndexBuffer::Bind(const ContextPtr& context) const
{
  assert(context);
  assert(buffer_);

  context->IASetIndexBuffer(buffer_.Get(), DXGI_FORMAT_R16_UINT, 0u);
}

size_t IndexBuffer::size() const
{
  return size_;
}
