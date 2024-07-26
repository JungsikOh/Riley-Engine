#include "ShaderManager.h"
#include "../Core/Rendering.h"
#include "../Graphics/DXShaderCompiler.h"
#include "../Graphics/DXShaderProgram.h"
#include "spdlog\spdlog.h"
#include <execution>
#include <numeric>
#include <unordered_map>

// ShaderManager�� ���ؼ� �� ���������� ������ ����
// �ؽ����� �̿��Ͽ�, �ش� ��ȣ�� �������� ȣ���ϴ� ������� �۵�.
// �� �� ������ �ؽ����� ����,
// 1. �� ��������(vs,ps, ..)�� ���� �ؽ���
// 2. ���� �н��� �������� ���� �ؽ���(vs=PBR, ps=PBR)
// 1���� ������ ��. 2�� �ؽ��ʿ��ٰ� enum ShaderProgram�� �̸� �����ص� ����
// ���� �ش� enum�� key�� �����Ͽ� �ش� �ؽ��ʿ� �������� ����
// std::unoreder_map<ShaderProgram, DXShaderProgram> ����.

// enum�� enum class�� ����?
// enum�� ���, ���� �� �ش� scope �ȿ� enum ������ ���� �Ǿ�, �ٸ� enum��
// �����ϰ� ���ǵǾ� �ִ� ��쿡 ������ ������ �߻��Ѵ�.
// enum class�� ���,
// ���ο� scope �ȿ� ������ ������ �ִ� �����̴�. ���� enum class��
// ���ÿ��� 'Ŭ�����̸�::�����Ѱ�' �� ���� �������� ����ؾ� �Ѵ�.

// constexpr�̶�? ������ �ð� ����� ����� Ű����
// https://blockdmask.tistory.com/482
// ����� 2������ �ִ�.
// ������ �ð��� �˼� �ִ� ����� ���� ���(����ð��� �� �� �ִ� ����ð�
// ���)�̴�. const�� ��� �� �� ��� �����ϰ� constexpr�� ��쿡�� ���ڿ�
// �ش��Ѵ�.

namespace Riley
{

  // Shader ������ ���� std::unordered_map ����
  namespace
  {

    ID3D11Device* device;

    std::unordered_map<ShaderId, std::unique_ptr<DXVertexShader> >      vsShaderMap;
    std::unordered_map<ShaderId, std::unique_ptr<DXGeometryShader> >    gsShaderMap;
    std::unordered_map<ShaderId, std::unique_ptr<DXPixelShader> >       psShaderMap;
    std::unordered_map<ShaderId, std::unique_ptr<DXInputLayout> >       inputLayoutMap;

    // ���� ���̴����� ����ϰ� ���� ������ �°� ���� map
    std::unordered_map<ShaderProgram, DXGraphicsShaderProgram>          DXShaderProgramMap;

    constexpr DXShaderStage GetStage(ShaderId shader)
    {
      switch(shader)
        {
        case VS_Solid:
        case VS_Phong:
        case VS_Shadow:
        case VS_ShadowCube:
          return DXShaderStage::VS;
        case PS_Solid:
        case PS_Phong:
        case PS_Shadow:
        case PS_ShadowCube:
          return DXShaderStage::PS;
        case GS_ShadowCube:
          return DXShaderStage::GS;
        default:
          assert("Not supported DXShaderStage.");
        }
    }

    constexpr std::string GetShaderSource(ShaderId shader)
    {
      switch(shader)
        {
        case VS_Solid:
        case PS_Solid:
          return "Resources/Shaders/Solid.hlsl";
        case VS_Phong:
        case PS_Phong:
          return "Resources/Shaders/ForwardPhong.hlsl";
        case VS_Shadow:
        case PS_Shadow:
          return "Resources/Shaders/Shadow.hlsl";
        case VS_ShadowCube:
        case GS_ShadowCube:
        case PS_ShadowCube:
          return "Resources/Shaders/ShadowCube.hlsl";
        default:
          assert("Don't found Shader Resource Path");
        }
    }

    constexpr std::string GetEntryPoint(ShaderId shader)
    {
      switch(shader)
        {
        case VS_Solid:
          return "SolidVS";
        case PS_Solid:
          return "SolidPS";
        case VS_Phong:
          return "PhongVS";
        case PS_Phong:
          return "PhongPS";
        case VS_Shadow:
          return "ShadowVS";
        case PS_Shadow:
          return "ShadowPS";
        case VS_ShadowCube:
          return "ShadowCubeVS";
        case GS_ShadowCube:
          return "ShadowCubeGS";
        case PS_ShadowCube:
          return "ShadowCubePS";
        default:
          return "main";
        }
    }

    void CompileShader(ShaderId shader, bool firstCompile = false)
    {
      DXShaderDesc input{.entryPoint = GetEntryPoint(shader)};

#if _DEBUG
      input.flags = DXShaderCompilerFlagBit_DEBUG
                    | DXShaderCompilerFlagBit_DisableOptimization;
#else
      input.flags = DXShaderCompilerFlagBit_NONE;
#endif
      input.sourceFile = GetShaderSource(shader);
      input.stage = GetStage(shader);

      DXShaderCompileOutput output{};
      bool result = DXShaderCompiler::CompileShader(input, output);
      if(!result) return;

      switch(input.stage)
        {
        case DXShaderStage::VS:
          if(firstCompile)
            vsShaderMap[shader] = std::make_unique<DXVertexShader>(
              device, output.shader_bytecode);
          else
            vsShaderMap[shader]->Recreate(output.shader_bytecode);
          break;
        case DXShaderStage::PS:
          if(firstCompile)
            psShaderMap[shader] = std::make_unique<DXPixelShader>(
              device, output.shader_bytecode);
          else
            psShaderMap[shader]->Recreate(output.shader_bytecode);
          break;
        case DXShaderStage::GS:
          if(firstCompile)
            gsShaderMap[shader] = std::make_unique<DXGeometryShader>(
              device, output.shader_bytecode);
          else
            gsShaderMap[shader]->Recreate(output.shader_bytecode);
          break;
        default:
          assert("Unsupported Shader Stage!");
        }
    }

    // ���� ���̴����� ���ϴ� �������� ��� Ŀ�����ϴ� �Լ�
    void CreateAllPrograms()
    {
      // enum class�� �Ͻ��� ����ȯ�� �����ϰ� ���� �ֱ� ������ �� ������
      // �غ��ϱ� ���� std::underlying_tye_t�� ����Ѵ�.
      using UnderlyingType = std::underlying_type_t<ShaderId>;
      for(UnderlyingType s = 0; s < ShaderIdCount; ++s)
        {
          ShaderId shader = (ShaderId)s; // It's OK.
          if(GetStage(shader) != DXShaderStage::VS) continue;

          inputLayoutMap[shader] = std::make_unique<DXInputLayout>(
            device, vsShaderMap[shader]->GetBytecode());
        }

      DXShaderProgramMap[ShaderProgram::Solid]
        .SetVertexShader(vsShaderMap[VS_Solid].get())
        .SetPixelShader(psShaderMap[PS_Solid].get())
        .SetInputLayout(inputLayoutMap[VS_Solid].get());
      DXShaderProgramMap[ShaderProgram::ForwardPhong]
        .SetVertexShader(vsShaderMap[VS_Phong].get())
        .SetPixelShader(psShaderMap[PS_Phong].get())
        .SetInputLayout(inputLayoutMap[VS_Phong].get());
      DXShaderProgramMap[ShaderProgram::ShadowDepthMap]
        .SetVertexShader(vsShaderMap[VS_Shadow].get())
        .SetPixelShader(psShaderMap[PS_Shadow].get())
        .SetInputLayout(inputLayoutMap[VS_Shadow].get());
      DXShaderProgramMap[ShaderProgram::ShadowDepthCubeMap]
        .SetVertexShader(vsShaderMap[VS_ShadowCube].get())
        .SetGeometryShader(gsShaderMap[GS_ShadowCube].get())
        .SetPixelShader(psShaderMap[PS_ShadowCube].get())
        .SetInputLayout(inputLayoutMap[VS_ShadowCube].get());
    }

    void CompileAllShaders()
    {
      Timer t;
      spdlog::info("Compiling All Shaders...");
      using UnderlyingType = std::underlying_type_t<ShaderId>;

      std::vector<UnderlyingType> shaders(ShaderIdCount);
      // std::iota�� �����̳ʿ� ������ ���ڸ� �Ҵ��Ѵ�. [0,ShaderIdCount]
      std::iota(std::begin(shaders), std::end(shaders), 0);
      std::for_each(
        std::execution::seq, std::begin(shaders), std::end(shaders),
        [](UnderlyingType s) { CompileShader((ShaderId)s, true); });
      CreateAllPrograms();
      spdlog::info("Copilation done in {:f} seconds!", t.ElapsedInSeconds());
    }
  } // namespace

  void ShaderManager::Initialize(ID3D11Device* _device)
  {
    device = _device;
    CompileAllShaders();
  }

  void ShaderManager::Destroy()
  {
    device = nullptr;
    auto FreeContainer = []<typename T>(T& container) {
      container.clear();
      T empty;
      std::swap(container, empty);
    };
    FreeContainer(DXShaderProgramMap);
    FreeContainer(vsShaderMap);
    FreeContainer(psShaderMap);
    FreeContainer(gsShaderMap);
  }

  DXShaderProgram* ShaderManager::GetShaderProgram(ShaderProgram shaderProgram)
  {
    bool isDXProgram = DXShaderProgramMap.contains(shaderProgram);
    if(isDXProgram) return &DXShaderProgramMap[shaderProgram];
    return &DXShaderProgramMap[shaderProgram];
  }

} // namespace Riley