#include "ShaderManager.h"
#include "../Core/Rendering.h"
#include "../Graphics/DXShaderCompiler.h"
#include "../Graphics/DXShaderProgram.h"
#include "spdlog\spdlog.h"
#include <execution>
#include <numeric>
#include <unordered_map>

// ShaderManager를 통해서 각 렌더링들의 설정을 관리
// 해쉬맵을 이용하여, 해당 번호의 설정들을 호출하는 방식으로 작동.
// 총 두 종류의 해쉬맵을 생성,
// 1. 각 스테이지(vs,ps, ..)를 담은 해쉬맵
// 2. 렌더 패스의 설정들을 담은 해쉬맵(vs=PBR, ps=PBR)
// 1번을 생성한 후. 2번 해쉬맵에다가 enum ShaderProgram을 미리 정의해둔 것을
// 통해 해당 enum을 key로 설정하여 해당 해쉬맵에 설정들을 저장
// std::unoreder_map<ShaderProgram, DXShaderProgram> 선언.

// enum과 enum class의 차이?
// enum의 경우, 정의 시 해당 scope 안에 enum 값들이 정의 되어, 다른 enum에
// 동일하게 정의되어 있는 경우에 컴파일 에러가 발생한다.
// enum class의 경우,
// 새로운 scope 안에 각각의 값들을 넣는 느낌이다. 따라서 enum class를 사용시에는
// '클래스이름::정의한것' 과 같은 형식으로 사용해야 한다.

// constexpr이란? 컴파일 시간 상수를 만드는 키워드
// https://blockdmask.tistory.com/482
// 상수란 2종류가 있다.
// 컴파일 시간에 알수 있는 상수와 없는 상수(실행시간에 알 수 있는 실행시간
// 상수)이다. const의 경우 둘 다 사용 가능하고 constexpr의 경우에는 전자에
// 해당한다.

namespace Riley {

// Shader 설정을 도울 std::unordered_map 정의
namespace {

ID3D11Device* device;

std::unordered_map<ShaderId, std::unique_ptr<DXVertexShader>> vsShaderMap;
std::unordered_map<ShaderId, std::unique_ptr<DXPixelShader>> psShaderMap;
std::unordered_map<ShaderId, std::unique_ptr<DXInputLayout>> inputLayoutMap;

// 만든 셰이더들을 사용하고 싶은 설정에 맞게 담을 map
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

// 만든 세이더들을 원하는 설정들을 모아 커스텀하는 함수
void CreateAllPrograms() {
    // enum class는 암시적 형변환을 강력하게 막고 있기 때문에 이 문제를 극복하기
    // 위해 std::underlying_tye_t를 사용한다.
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
    // std::iota는 컨테이너에 연속적 숫자를 할당한다. [0,ShaderIdCount]
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