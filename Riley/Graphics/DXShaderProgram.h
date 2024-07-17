#pragma once
#include "../Core/CoreTypes.h"
#include "../Core/Rendering.h"
#include "DXShader.h"
#include "DXStates.h"
#include "DXInputLayout.h"
#include <memory>

namespace Riley {
class DXInputLayout;

class DXShader {
  public:
    DXShader() = default;
    DXShaderBytecode& GetBytecode() { return m_bytecode; }
    DXShaderStage GetStage() const { return m_stage; }

    virtual ~DXShader() = default;
    virtual void Recreate(DXShaderBytecode const& blob) = 0;

  protected:
    ID3D11Device* m_device;
    DXShaderBytecode m_bytecode;
    DXShaderStage m_stage;

  protected:
    DXShader(ID3D11Device* device, DXShaderBytecode const& bytecode,
             DXShaderStage stage)
        : m_device(device), m_bytecode(bytecode), m_stage(stage) {}
};

// fianl : 추가 상속을 차단하기 위한 키워드
// & 로 넘겨주는 이유는 device를 포인터로 넘겨주게되면 포인터 주소가 변경되는데, 이 주소를 DXShader의 device로 등록을 해버리게 된다.
// 그렇게 되면서 
// Vertex Shader
class DXVertexShader final : public DXShader {
  public:
    DXVertexShader() = default;
    DXVertexShader(ID3D11Device* device, DXShaderBytecode const& blob);
    virtual void Recreate(DXShaderBytecode const& blob);

    operator ID3D11VertexShader*() { return vs; }

  private:
    ID3D11VertexShader* vs = nullptr;
};

// Pixel Shader
class DXPixelShader final : public DXShader {
  public:
    DXPixelShader() = default;
    DXPixelShader(ID3D11Device* device, DXShaderBytecode const& blob);
    virtual void Recreate(DXShaderBytecode const& blob);

    operator ID3D11PixelShader*() { return ps; }

  private:
    ID3D11PixelShader* ps = nullptr;
};

// Domain Shader
class DXDomainShader final : public DXShader {
  public:
    DXDomainShader() = default;
    DXDomainShader(ID3D11Device* device, DXShaderBytecode const& blob);
    virtual void Recreate(DXShaderBytecode const& blob);

    operator ID3D11DomainShader*() const { return ds; }

  private:
    ID3D11DomainShader* ds = nullptr;
};

// Hull Shader
class DXHullShader final : public DXShader {
  public:
    DXHullShader() = default;
    DXHullShader(ID3D11Device* device, DXShaderBytecode const& blob);
    virtual void Recreate(DXShaderBytecode const& blob);

    operator ID3D11HullShader*() const { return hs; }

  private:
    ID3D11HullShader* hs = nullptr;
};

// Geometry Shader
class DXGeometryShader final : public DXShader {
  public:
    DXGeometryShader() = default;
    DXGeometryShader(ID3D11Device* device, DXShaderBytecode const& blob);
    virtual void Recreate(DXShaderBytecode const& blob);

    operator ID3D11GeometryShader*() const { return gs; }

  private:
    ID3D11GeometryShader* gs = nullptr;
};

// Compute Shader
class DXComputeShader final : public DXShader {
  public:
    DXComputeShader() = default;
    DXComputeShader(ID3D11Device* device, DXShaderBytecode const& blob);
    virtual void Recreate(DXShaderBytecode const& blob);

    operator ID3D11ComputeShader*() const { return cs; }

  private:
    ID3D11ComputeShader* cs = nullptr;
};

struct DXShaderProgram {
    virtual ~DXShaderProgram() = default;
    virtual void Bind(ID3D11DeviceContext* context) = 0;
    virtual void Unbind(ID3D11DeviceContext* context) = 0;
};

class DXGraphicsShaderProgram final : public DXShaderProgram {
  public:
    DXGraphicsShaderProgram() = default;
    virtual ~DXGraphicsShaderProgram() = default;
    DXGraphicsShaderProgram& SetVertexShader(DXVertexShader* _vs);
    DXGraphicsShaderProgram& SetPixelShader(DXPixelShader* _ps);
    DXGraphicsShaderProgram& SetDomainShader(DXDomainShader* _ds);
    DXGraphicsShaderProgram& SetHullShader(DXHullShader* _hs);
    DXGraphicsShaderProgram& SetGeometryShader(DXGeometryShader* _gs);
    DXGraphicsShaderProgram& SetInputLayout(DXInputLayout* _il);

    virtual void Bind(ID3D11DeviceContext* context);
    virtual void Unbind(ID3D11DeviceContext* context);

  private:
    DXVertexShader* vs = nullptr;
    DXPixelShader* ps = nullptr;
    DXHullShader* hs = nullptr;
    DXDomainShader* ds = nullptr;
    DXGeometryShader* gs = nullptr;
    DXInputLayout* inputLayout = nullptr;
};

class DXComputeShaderProgram final : public DXShaderProgram {
  public:
    DXComputeShaderProgram() = default;
    virtual ~DXComputeShaderProgram() = default;
    void SetComputeShader(DXComputeShader* cs);

    virtual void Bind(ID3D11DeviceContext* context);
    virtual void Unbind(ID3D11DeviceContext* context);

  private:
    DXComputeShader* shader = nullptr;
};

} // namespace Riley