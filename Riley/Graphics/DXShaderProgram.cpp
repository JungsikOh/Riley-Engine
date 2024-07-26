#include "DXShaderProgram.h"
#include <memory>

namespace Riley
{
  DXVertexShader::DXVertexShader(ID3D11Device *device,
                                 DXShaderBytecode const &blob)
      : DXShader(device, blob, DXShaderStage::VS)
  {
    DXShaderBytecode const &vsBlob = GetBytecode();
    HR(m_device->CreateVertexShader(vsBlob.GetPointer(), vsBlob.GetLength(),
                                    nullptr, &vs));
  }

  void DXVertexShader::Recreate(DXShaderBytecode const &blob)
  {
    GetBytecode() = blob;
    HR(m_device->CreateVertexShader(blob.GetPointer(), blob.GetLength(),
                                    nullptr, &vs));
  }

  DXPixelShader::DXPixelShader(ID3D11Device *device,
                               DXShaderBytecode const &blob)
      : DXShader(device, blob, DXShaderStage::PS)
  {
    DXShaderBytecode const &psBlob = GetBytecode();
    HR(m_device->CreatePixelShader(psBlob.GetPointer(), psBlob.GetLength(),
                                   nullptr, &ps));
  }

  void DXPixelShader::Recreate(DXShaderBytecode const &blob)
  {
    GetBytecode() = blob;
    HR(m_device->CreatePixelShader(blob.GetPointer(), blob.GetLength(),
                                   nullptr, &ps));
  }
  
  DXDomainShader::DXDomainShader(ID3D11Device* device,
                                 DXShaderBytecode const& blob)
      : DXShader(device, blob, DXShaderStage::DS) {
      DXShaderBytecode const& dsBlob = GetBytecode();
      HR(m_device->CreateDomainShader(dsBlob.GetPointer(), dsBlob.GetLength(),
                                      nullptr, &ds));
  }

  void DXDomainShader::Recreate(DXShaderBytecode const& blob) {
      GetBytecode() = blob;
      HR(m_device->CreateDomainShader(blob.GetPointer(), blob.GetLength(),
                                      nullptr, &ds));
  }

  DXHullShader::DXHullShader(ID3D11Device* device,
                             DXShaderBytecode const&
  blob) : DXShader(device, blob, DXShaderStage::HS) { DXShaderBytecode const&
  hsBlob = GetBytecode(); HR(m_device->CreateHullShader(hsBlob.GetPointer(),
  hsBlob.GetLength(), nullptr, &hs));
  }

  void DXHullShader::Recreate(DXShaderBytecode const& blob) {
      GetBytecode() = blob;
      HR(m_device->CreateHullShader(blob.GetPointer(), blob.GetLength(),
  nullptr, &hs));
  }

  DXGeometryShader::DXGeometryShader(ID3D11Device* device,
                                     DXShaderBytecode const& blob)
      : DXShader(device, blob, DXShaderStage::GS) {
      DXShaderBytecode const& gsBlob = GetBytecode();
      HR(m_device->CreateGeometryShader(gsBlob.GetPointer(),
  gsBlob.GetLength(), nullptr, &gs));
  }

  void DXGeometryShader::Recreate(DXShaderBytecode const& blob) {
      GetBytecode() = blob;
      HR(m_device->CreateGeometryShader(blob.GetPointer(), blob.GetLength(),
                                        nullptr, &gs));
  }

  DXComputeShader::DXComputeShader(ID3D11Device* device,
                                   DXShaderBytecode const& blob)
      : DXShader(device, blob, DXShaderStage::CS) {
      DXShaderBytecode const& csBlob = GetBytecode();
      HR(m_device->CreateComputeShader(csBlob.GetPointer(), csBlob.GetLength(),
                                       nullptr, &cs));
  }

  void DXComputeShader::Recreate(DXShaderBytecode const& blob) {
      GetBytecode() = blob;
      HR(m_device->CreateComputeShader(blob.GetPointer(), blob.GetLength(),
                                       nullptr, &cs));
  }
  

  DXGraphicsShaderProgram &
  DXGraphicsShaderProgram::SetVertexShader(DXVertexShader *_vs)
  {
    vs = _vs;
    return *this;
  }

  DXGraphicsShaderProgram &
  DXGraphicsShaderProgram::SetPixelShader(DXPixelShader *_ps)
  {
    ps = _ps;
    return *this;
  }

  DXGraphicsShaderProgram &
  DXGraphicsShaderProgram::SetDomainShader(DXDomainShader *_ds)
  {
    ds = _ds;
    return *this;
  }

  DXGraphicsShaderProgram &
  DXGraphicsShaderProgram::SetHullShader(DXHullShader *_hs)
  {
    hs = _hs;
    return *this;
  }

  DXGraphicsShaderProgram &
  DXGraphicsShaderProgram::SetGeometryShader(DXGeometryShader *_gs)
  {
    gs = _gs;
    return *this;
  }

  DXGraphicsShaderProgram &
  DXGraphicsShaderProgram::SetInputLayout(DXInputLayout *_il)
  {
    inputLayout = _il;
    return *this;
  }

  void DXGraphicsShaderProgram::Bind(ID3D11DeviceContext *context)
  {
    if(inputLayout)
      context->IASetInputLayout(*inputLayout);
    if(vs)
      context->VSSetShader(*vs, nullptr, 0);
    if(gs)
      context->GSSetShader(*gs, nullptr, 0);
    if(hs)
      context->HSSetShader(*hs, nullptr, 0);
    if(ds)
      context->DSSetShader(*ds, nullptr, 0);
    if(ps)
      context->PSSetShader(*ps, nullptr, 0);
  }

  void DXGraphicsShaderProgram::Unbind(ID3D11DeviceContext *context)
  {
    if(inputLayout)
      context->IASetInputLayout(nullptr);
    if(vs)
      context->VSSetShader(nullptr, nullptr, 0);
    if(gs)
      context->GSSetShader(nullptr, nullptr, 0);
    if(hs)
      context->HSSetShader(nullptr, nullptr, 0);
    if(ds)
      context->DSSetShader(nullptr, nullptr, 0);
    if(ps)
      context->PSSetShader(nullptr, nullptr, 0);
  }

  void DXComputeShaderProgram::SetComputeShader(DXComputeShader *cs)
  {
    shader = cs;
  }
  void DXComputeShaderProgram::Bind(ID3D11DeviceContext *context)
  {
    if(shader)
      context->CSSetShader(*shader, nullptr, 0);
  }
  void DXComputeShaderProgram::Unbind(ID3D11DeviceContext *context)
  {
    if(shader)
      context->CSSetShader(nullptr, nullptr, 0);
  }

} // namespace Riley