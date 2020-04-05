#include "constant_buffer.hpp"
#include <iostream>


PerFrameConstantBuffer::PerFrameConstantBuffer(const DevicePtr& device)
{
  Init(device);
}

void PerFrameConstantBuffer::Init(const DevicePtr& device)
{
  D3D11_BUFFER_DESC desc = {};

  desc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
  desc.Usage               = D3D11_USAGE_DYNAMIC;
  desc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
  desc.ByteWidth           = sizeof buffer_;
  desc.StructureByteStride = sizeof PerFrameBuffer;

  D3D11_SUBRESOURCE_DATA data = {};
  data.pSysMem = &buffer_;

  if (HResult res = device->CreateBuffer(&desc, &data, &cbuffer_); !res)
  {
    std::cerr << __func__ << ":" << __LINE__ << " error: " << std::hex << static_cast<HRESULT>(res) << std::endl;
    std::terminate();
  }
}
