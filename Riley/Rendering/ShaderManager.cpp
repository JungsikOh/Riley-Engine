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
// ���ο� scope �ȿ� ������ ������ �ִ� �����̴�. ���� enum class�� ���ÿ���
// 'Ŭ�����̸�::�����Ѱ�' �� ���� �������� ����ؾ� �Ѵ�.

// constexpr�̶�? ������ �ð� ����� ����� Ű����
// https://blockdmask.tistory.com/482
// ����� 2������ �ִ�.
// ������ �ð��� �˼� �ִ� ����� ���� ���(����ð��� �� �� �ִ� ����ð�
// ���)�̴�. const�� ��� �� �� ��� �����ϰ� constexpr�� ��쿡�� ���ڿ�
// �ش��Ѵ�.

namespace Riley {

// Shader ������ ���� std::unordered_map ����
namespace {

ID3D11Device* device;

std::unordered_map<ShaderId, std::unique_ptr<DXVertexShader>> vsShaderMap;
std::unordered_map<ShaderId, std::unique_ptr<DXPixelShader>> psShaderMap;
std::unordered_map<ShaderId, std::unique_ptr<DXInputLayout>> inputLayoutMap;

// ���� ���̴����� ����ϰ� ���� ������ �°� ���� map
std::unordered_map<ShaderProgram, DXGraphicsShaderProgram> DXShaderProgramMap;

constexpr DXShaderStage GetStage(ShaderId shader) {
    switch (shader) {
    case VS_Solid:
        return DXShaderStage::VS;
    case PS_Solid:
        return DXShaderStage::PS;
    }
}

constexpr std::string GetShaderSource(ShaderId shader) {
    switch (shader) {
    case VS_Solid:
    case PS_Solid:
        return "Resources/Shaders/Solid.hlsl";
    }
}

constexpr std::string GetEntryPoint(ShaderId shader) {
    switch (shader) {
    case VS_Solid:
        return "SolidVS";
    case PS_Solid:
        return "SolidPS";
    default:
        return "main";
    }
}

void CompileShader(ShaderId shader, bool firstCompile = false) {
    DXShaderDesc input{.entryPoint = GetEntryPoint(shader)};

#if _DEBUG
    input.flags = DXShaderCompilerFlagBit_DEBUG |
                  DXShaderCompilerFlagBit_DisableOptimization;
#else
    input.flags = DXShaderCompilerFlagBit_NONE;
#endif
    input.sourceFile = GetShaderSource(shader);
    input.stage = GetStage(shader);

    DXShaderCompileOutput output{};
    bool result = DXShaderCompiler::CompileShader(input, output);
    if (!result)
        return;

    switch (input.stage) {
    case DXShaderStage::VS:
        if (firstCompile)
            vsShaderMap[shader] = std::make_unique<DXVertexShader>(
                device, output.shader_bytecode);
        else
            vsShaderMap[shader]->Recreate(output.shader_bytecode);
        break;
    case DXShaderStage::PS:
        if (firstCompile)
            psShaderMap[shader] = std::make_unique<DXPixelShader>(
                device, output.shader_bytecode);
        else
            psShaderMap[shader]->Recreate(output.shader_bytecode);
        break;
    default:
        assert(false);
    }
}

// ���� ���̴����� ���ϴ� �������� ��� Ŀ�����ϴ� �Լ�
void CreateAllPrograms() {
    // enum class�� �Ͻ��� ����ȯ�� �����ϰ� ���� �ֱ� ������ �� ������ �غ��ϱ�
    // ���� std::underlying_tye_t�� ����Ѵ�.
    using UnderlyingType = std::underlying_type_t<ShaderId>;
    for (UnderlyingType s = 0; s < ShaderIdCount; ++s) {
        ShaderId shader = (ShaderId)s; // It's OK.
        if (GetStage(shader) != DXShaderStage::VS)
            continue;

        inputLayoutMap[shader] = std::make_unique<DXInputLayout>(
            device, vsShaderMap[shader]->GetBytecode());
    }

    DXShaderProgramMap[ShaderProgram::Solid]
        .SetVertexShader(vsShaderMap[VS_Solid].get())
        .SetPixelShader(psShaderMap[PS_Solid].get())
        .SetInputLayout(inputLayoutMap[VS_Solid].get());
}

void CompileAllShaders() {
    Timer t;
    spdlog::info("Compiling All Shaders...");
    using UnderlyingType = std::underlying_type_t<ShaderId>;

    std::vector<UnderlyingType> shaders(ShaderIdCount);
    // std::iota�� �����̳ʿ� ������ ���ڸ� �Ҵ��Ѵ�. [0,ShaderIdCount]
    std::iota(std::begin(shaders), std::end(shaders), 0);
    std::for_each(std::execution::seq, std::begin(shaders), std::end(shaders),
                  [](UnderlyingType s) { CompileShader((ShaderId)s, true); });
    CreateAllPrograms();
    spdlog::info("Copilation done in {:f} seconds!", t.ElapsedInSeconds());
}
} // namespace

void ShaderManager::Initialize(ID3D11Device* _device) {
    device = _device;
    CompileAllShaders();
}

void ShaderManager::Destroy() {
    device = nullptr;
    auto FreeContainer = []<typename T>(T& container) {
        container.clear();
        T empty;
        std::swap(container, empty);
    };
    FreeContainer(DXShaderProgramMap);
    FreeContainer(vsShaderMap);
    FreeContainer(psShaderMap);
}

DXShaderProgram* ShaderManager::GetShaderProgram(ShaderProgram shaderProgram) {
    bool isDXProgram = DXShaderProgramMap.contains(shaderProgram);
    if (isDXProgram)
        return &DXShaderProgramMap[shaderProgram];
    return &DXShaderProgramMap[shaderProgram];
}

} // namespace Riley