#include "ShaderManager.h"

#include <execution>
#include <numeric>

#include "../Core/Rendering.h"
#include "../Graphics/DXShaderCompiler.h"
#include "../Graphics/DXShaderProgram.h"

// ShaderManager¸¦ ÅëÇØ¼­ °¢ ·»´õ¸µµéÀÇ ¼³Á¤À» °ü¸®
// ÇØ½¬¸ÊÀ» ÀÌ¿ëÇÏ¿©, ÇØ´ç ¹øÈ£ÀÇ ¼³Á¤µéÀ» È£ÃâÇÏ´Â ¹æ½ÄÀ¸·Î ÀÛµ¿.
// ÃÑ µÎ Á¾·ùÀÇ ÇØ½¬¸ÊÀ» »ý¼º,
// 1. °¢ ½ºÅ×ÀÌÁö(vs,ps, ..)¸¦ ´ãÀº ÇØ½¬¸Ê
// 2. ·»´õ ÆÐ½ºÀÇ ¼³Á¤µéÀ» ´ãÀº ÇØ½¬¸Ê(vs=PBR, ps=PBR)
// 1¹øÀ» »ý¼ºÇÑ ÈÄ. 2¹ø ÇØ½¬¸Ê¿¡´Ù°¡ enum ShaderProgramÀ» ¹Ì¸® Á¤ÀÇÇØµÐ °ÍÀ»
// ÅëÇØ ÇØ´ç enumÀ» key·Î ¼³Á¤ÇÏ¿© ÇØ´ç ÇØ½¬¸Ê¿¡ ¼³Á¤µéÀ» ÀúÀå
// std::unoreder_map<ShaderProgram, DXShaderProgram> ¼±¾ð.

// enum°ú enum classÀÇ Â÷ÀÌ?
// enumÀÇ °æ¿ì, Á¤ÀÇ ½Ã ÇØ´ç scope ¾È¿¡ enum °ªµéÀÌ Á¤ÀÇ µÇ¾î, ´Ù¸¥ enum¿¡
// µ¿ÀÏÇÏ°Ô Á¤ÀÇµÇ¾î ÀÖ´Â °æ¿ì¿¡ ÄÄÆÄÀÏ ¿¡·¯°¡ ¹ß»ýÇÑ´Ù.
// enum classÀÇ °æ¿ì,
// »õ·Î¿î scope ¾È¿¡ °¢°¢ÀÇ °ªµéÀ» ³Ö´Â ´À³¦ÀÌ´Ù. µû¶ó¼­ enum class¸¦
// »ç¿ë½Ã¿¡´Â 'Å¬·¡½ºÀÌ¸§::Á¤ÀÇÇÑ°Í' °ú °°Àº Çü½ÄÀ¸·Î »ç¿ëÇØ¾ß ÇÑ´Ù.

// constexprÀÌ¶õ? ÄÄÆÄÀÏ ½Ã°£ »ó¼ö¸¦ ¸¸µå´Â Å°¿öµå
// https://blockdmask.tistory.com/482
// »ó¼ö¶õ 2Á¾·ù°¡ ÀÖ´Ù.
// ÄÄÆÄÀÏ ½Ã°£¿¡ ¾Ë¼ö ÀÖ´Â »ó¼ö¿Í ¾ø´Â »ó¼ö(½ÇÇà½Ã°£¿¡ ¾Ë ¼ö ÀÖ´Â ½ÇÇà½Ã°£
// »ó¼ö)ÀÌ´Ù. constÀÇ °æ¿ì µÑ ´Ù »ç¿ë °¡´ÉÇÏ°í constexprÀÇ °æ¿ì¿¡´Â ÀüÀÚ¿¡
// ÇØ´çÇÑ´Ù.

namespace Riley
{

// Shader ¼³Á¤À» µµ¿ï std::unordered_map Á¤ÀÇ
namespace
{

ID3D11Device* device;

std::unordered_map<ShaderId, std::unique_ptr<DXVertexShader>> vsShaderMap;
std::unordered_map<ShaderId, std::unique_ptr<DXGeometryShader>> gsShaderMap;
std::unordered_map<ShaderId, std::unique_ptr<DXPixelShader>> psShaderMap;
std::unordered_map<ShaderId, std::unique_ptr<DXInputLayout>> inputLayoutMap;

// ¸¸µç ¼ÎÀÌ´õµéÀ» »ç¿ëÇÏ°í ½ÍÀº ¼³Á¤¿¡ ¸Â°Ô ´ãÀ» map
std::unordered_map<ShaderProgram, DXGraphicsShaderProgram> DXShaderProgramMap;

constexpr DXShaderStage GetStage(ShaderId shader)
{
    switch (shader)
    {
    case VS_ScreenQuad:
    case VS_Solid:
    case VS_Phong:
    case VS_GBuffer:
    case VS_Shadow:
    case VS_ShadowCube:
    case VS_Picking:
        return DXShaderStage::VS;
    case PS_Solid:
    case PS_Phong:
    case PS_GBuffer:
    case PS_Ambient:
    case PS_DeferredLighting:
    case PS_SSAO:
    case PS_SSAOBlur:
    case PS_Shadow:
    case PS_ShadowCube:
    case PS_Picking:
        return DXShaderStage::PS;
    case GS_ShadowCube:
        return DXShaderStage::GS;
    default:
        assert("Not supported DXShaderStage.");
    }
}

constexpr std::string GetShaderSource(ShaderId shader)
{
    switch (shader)
    {
    case VS_ScreenQuad:
        return "Resources/Shaders/ScreenQuad.hlsl";
    case VS_Solid:
    case PS_Solid:
        return "Resources/Shaders/Solid.hlsl";
    case VS_Phong:
    case PS_Phong:
        return "Resources/Shaders/ForwardPhong.hlsl";
    case VS_GBuffer:
    case PS_GBuffer:
        return "Resources/Shaders/GBuffer.hlsl";
    case PS_Ambient:
        return "Resources/Shaders/Ambient.hlsl";
    case PS_DeferredLighting:
        return "Resources/Shaders/DeferredLighting.hlsl";
    case PS_SSAO:
        return "Resources/Shaders/SSAO.hlsl";
    case PS_SSAOBlur:
        return "Resources/Shaders/SSAOBlur.hlsl";
    case VS_Shadow:
    case PS_Shadow:
        return "Resources/Shaders/Shadow.hlsl";
    case VS_ShadowCube:
    case GS_ShadowCube:
    case PS_ShadowCube:
        return "Resources/Shaders/ShadowCube.hlsl";
    case VS_Picking:
    case PS_Picking:
        return "Resources/Shaders/Picking.hlsl";
    default:
        assert("Don't found Shader Resource Path");
    }
}

constexpr std::string GetEntryPoint(ShaderId shader)
{
    switch (shader)
    {
    case VS_ScreenQuad:
        return "ScreenQuad";
    case VS_Solid:
        return "SolidVS";
    case PS_Solid:
        return "SolidPS";
    case VS_Phong:
        return "PhongVS";
    case PS_Phong:
        return "PhongPS";
    case VS_GBuffer:
        return "GBufferVS";
    case PS_GBuffer:
        return "GBUfferPS";
    case PS_Ambient:
        return "AmbientPS";
    case PS_DeferredLighting:
        return "DeferredLightingPS";
    case PS_SSAO:
        return "SSAO";
    case PS_SSAOBlur:
        return "SSAOBlur";
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
    case VS_Picking:
        return "PickingVS";
    case PS_Picking:
        return "PickingPS";
    default:
        return "main";
    }
}

void CompileShader(ShaderId shader, bool firstCompile = false)
{
    DXShaderDesc input{.entryPoint = GetEntryPoint(shader)};

#if _DEBUG
    input.flags = DXShaderCompilerFlagBit_DEBUG | DXShaderCompilerFlagBit_DisableOptimization;
#else
    input.flags = DXShaderCompilerFlagBit_NONE;
#endif
    input.sourceFile = GetShaderSource(shader);
    input.stage = GetStage(shader);

    DXShaderCompileOutput output{};
    bool result = DXShaderCompiler::CompileShader(input, output);
    if (!result)
        return;

    switch (input.stage)
    {
    case DXShaderStage::VS:
        if (firstCompile)
            vsShaderMap[shader] = std::make_unique<DXVertexShader>(device, output.shader_bytecode);
        else
            vsShaderMap[shader]->Recreate(output.shader_bytecode);
        break;
    case DXShaderStage::PS:
        if (firstCompile)
            psShaderMap[shader] = std::make_unique<DXPixelShader>(device, output.shader_bytecode);
        else
            psShaderMap[shader]->Recreate(output.shader_bytecode);
        break;
    case DXShaderStage::GS:
        if (firstCompile)
            gsShaderMap[shader] = std::make_unique<DXGeometryShader>(device, output.shader_bytecode);
        else
            gsShaderMap[shader]->Recreate(output.shader_bytecode);
        break;
    default:
        assert("Unsupported Shader Stage!");
    }
}

void CreateAllPrograms()
{
    using UnderlyingType = std::underlying_type_t<ShaderId>;
    for (UnderlyingType s = 0; s < ShaderIdCount; ++s)
    {
        ShaderId shader = (ShaderId)s; // It's OK.
        if (GetStage(shader) != DXShaderStage::VS)
            continue;

        inputLayoutMap[shader] = std::make_unique<DXInputLayout>(device, vsShaderMap[shader]->GetBytecode());
    }

    DXShaderProgramMap[ShaderProgram::Solid]
        .SetVertexShader(vsShaderMap[VS_Solid].get())
        .SetPixelShader(psShaderMap[PS_Solid].get())
        .SetInputLayout(inputLayoutMap[VS_Solid].get());
    DXShaderProgramMap[ShaderProgram::ForwardPhong]
        .SetVertexShader(vsShaderMap[VS_Phong].get())
        .SetPixelShader(psShaderMap[PS_Phong].get())
        .SetInputLayout(inputLayoutMap[VS_Phong].get());
    DXShaderProgramMap[ShaderProgram::GBuffer]
        .SetVertexShader(vsShaderMap[VS_GBuffer].get())
        .SetPixelShader(psShaderMap[PS_GBuffer].get())
        .SetInputLayout(inputLayoutMap[VS_GBuffer].get());
    DXShaderProgramMap[ShaderProgram::Ambient]
        .SetVertexShader(vsShaderMap[VS_ScreenQuad].get())
        .SetPixelShader(psShaderMap[PS_Ambient].get())
        .SetInputLayout(inputLayoutMap[VS_ScreenQuad].get());
    DXShaderProgramMap[ShaderProgram::DeferredLighting]
        .SetVertexShader(vsShaderMap[VS_ScreenQuad].get())
        .SetPixelShader(psShaderMap[PS_DeferredLighting].get())
        .SetInputLayout(inputLayoutMap[VS_ScreenQuad].get());
    DXShaderProgramMap[ShaderProgram::SSAO]
        .SetVertexShader(vsShaderMap[VS_ScreenQuad].get())
        .SetPixelShader(psShaderMap[PS_SSAO].get())
        .SetInputLayout(inputLayoutMap[VS_ScreenQuad].get());
    DXShaderProgramMap[ShaderProgram::SSAOBlur]
        .SetVertexShader(vsShaderMap[VS_ScreenQuad].get())
        .SetPixelShader(psShaderMap[PS_SSAOBlur].get())
        .SetInputLayout(inputLayoutMap[VS_ScreenQuad].get());
    DXShaderProgramMap[ShaderProgram::ShadowDepthMap]
        .SetVertexShader(vsShaderMap[VS_Shadow].get())
        .SetPixelShader(psShaderMap[PS_Shadow].get())
        .SetInputLayout(inputLayoutMap[VS_Shadow].get());
    DXShaderProgramMap[ShaderProgram::ShadowDepthCubeMap]
        .SetVertexShader(vsShaderMap[VS_ShadowCube].get())
        .SetGeometryShader(gsShaderMap[GS_ShadowCube].get())
        .SetPixelShader(psShaderMap[PS_ShadowCube].get())
        .SetInputLayout(inputLayoutMap[VS_ShadowCube].get());
    DXShaderProgramMap[ShaderProgram::Picking]
        .SetVertexShader(vsShaderMap[VS_Picking].get())
        .SetPixelShader(psShaderMap[PS_Picking].get())
        .SetInputLayout(inputLayoutMap[VS_Picking].get());
}

void CompileAllShaders()
{
    RI_CORE_INFO("Shaders Compiling...");
    using UnderlyingType = std::underlying_type_t<ShaderId>;

    std::vector<UnderlyingType> shaders(ShaderIdCount);
    // std::iota´Â ÄÁÅ×ÀÌ³Ê¿¡ ¿¬¼ÓÀû ¼ýÀÚ¸¦ ÇÒ´çÇÑ´Ù. [0,ShaderIdCount]
    std::iota(std::begin(shaders), std::end(shaders), 0);
    std::for_each(std::execution::seq, std::begin(shaders), std::end(shaders),
                  [](UnderlyingType s) { CompileShader((ShaderId)s, true); });
    CreateAllPrograms();
    RI_CORE_INFO("Shaders Compilation done in {:f}s", timer.MarkInSeconds());
    timer.Mark();
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
    FreeContainer(inputLayoutMap);
    FreeContainer(vsShaderMap);
    FreeContainer(psShaderMap);
    FreeContainer(gsShaderMap);
}

DXShaderProgram* ShaderManager::GetShaderProgram(ShaderProgram shaderProgram)
{
    bool isDXProgram = DXShaderProgramMap.contains(shaderProgram);
    if (isDXProgram)
        return &DXShaderProgramMap[shaderProgram];
    return &DXShaderProgramMap[shaderProgram];
}

} // namespace Riley