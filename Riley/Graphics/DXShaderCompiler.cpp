#include "DXShaderCompiler.h"
#include "../Core/Paths.h"
#include "../Utilities/FileUtil.h"
#include "../Utilities/HashUtil.h"
#include "../Utilities/StringUtil.h"
#include "DXShader.h"
#include "cereal/archives/binary.hpp"
#include "spdlog\spdlog.h"
#include <d3dcompiler.h>
#include <fstream>
#include <memory>

namespace Riley {
//namespace {
//
//inline static std::string shaders_cache_directory = paths::ShaderCacheDir();
//inline static std::string shaders_common_directory = paths::ShaderDir();
//
//class CShaderInclude : public ID3DInclude {
//  public:
//    explicit CShaderInclude(char const* shader_dir) : shader_dir(shader_dir) {}
//
//    HRESULT __stdcall Open(D3D_INCLUDE_TYPE include_type, char const* filename,
//                           void const* p_parent_data, void const** pp_data,
//                           uint32* bytes) {
//        fs::path final_path;
//        switch (include_type) {
//        case D3D_INCLUDE_LOCAL:
//            final_path = fs::path(shader_dir) / fs::path(filename);
//            break;
//        case D3D_INCLUDE_SYSTEM:
//            final_path =
//                fs::path(shaders_common_directory) / fs::path(filename);
//            break;
//        default:
//            return E_FAIL;
//        }
//        includes.push_back(final_path.string());
//        std::ifstream file_stream(final_path.string());
//        if (file_stream) {
//            std::string contents;
//            contents.assign(std::istreambuf_iterator<char>(file_stream),
//                            std::istreambuf_iterator<char>());
//
//            char* buf = new char[contents.size()];
//            contents.copy(buf, contents.size());
//            *pp_data = buf;
//            *bytes = (uint32)contents.size();
//        } else {
//            *pp_data = nullptr;
//            *bytes = 0;
//        }
//        return S_OK;
//    }
//
//    HRESULT __stdcall Close(void const* data) {
//        char* buf = (char*)data;
//        delete[] buf;
//        return S_OK;
//    }
//
//    std::vector<std::string> const& GetIncludes() const { return includes; }
//
//  private:
//    std::string shader_dir;
//    std::vector<std::string> includes;
//};
//
//bool CheckCache(char const* cache_path, DXShaderDesc const& input,
//                DXShaderCompileOutput& output) {
//    if (!FileExists(cache_path))
//        return false;
//    if (GetFileLastWriteTime(cache_path) <
//        GetFileLastWriteTime(input.sourceFile))
//        return false;
//
//    std::ifstream is(cache_path, std::ios::binary);
//    cereal::BinaryInputArchive archive(is);
//
//    archive(output.hash);
//    archive(output.includes);
//    uint32 binary_size = 0;
//    archive(binary_size);
//    std::unique_ptr<char[]> binary_data(new char[binary_size]);
//    archive.loadBinary(binary_data.get(), binary_size);
//    output.shader_bytecode.SetBytecode(binary_data.get(), binary_size);
//    return true;
//}
//bool SaveToCache(char const* cache_path, DXShaderCompileOutput const& output) {
//    std::ofstream os(cache_path, std::ios::binary);
//    cereal::BinaryOutputArchive archive(os);
//    archive(output.hash);
//    archive(output.includes);
//    archive(output.shader_bytecode.GetLength());
//    archive.saveBinary(output.shader_bytecode.GetPointer(),
//                       output.shader_bytecode.GetLength());
//    return true;
//}
//} // namespace
//
namespace DXShaderCompiler {
//
//void GetBytecodeFromCompiledShader(char const* filename,
//                                   DXShaderBytecode& blob) {
//    ID3DBlob* bytecode_blob;
//
//    std::wstring wide_filename = ToWideString(std::string(filename));
//    HRESULT hr = D3DReadFileToBlob(wide_filename.c_str(), &bytecode_blob);
//    HR(hr);
//
//    blob.bytecode.resize(bytecode_blob->GetBufferSize());
//    std::memcpy(blob.GetPointer(), bytecode_blob->GetBufferPointer(),
//                blob.GetLength());
//}
//
//bool CompileShader(DXShaderDesc const& input, DXShaderCompileOutput& output) {
//    output = DXShaderCompileOutput{};
//    std::string default_entrypoint, model;
//    switch (input.stage) {
//    case DXShaderStage::VS:
//        default_entrypoint = "vs_main";
//        model = "vs_5_0";
//        break;
//    case DXShaderStage::PS:
//        default_entrypoint = "ps_main";
//        model = "ps_5_0";
//        break;
//    case DXShaderStage::HS:
//        default_entrypoint = "hs_main";
//        model = "hs_5_0";
//        break;
//    case DXShaderStage::DS:
//        default_entrypoint = "ds_main";
//        model = "ds_5_0";
//        break;
//    case DXShaderStage::GS:
//        default_entrypoint = "gs_main";
//        model = "gs_5_0";
//        break;
//    case DXShaderStage::CS:
//        default_entrypoint = "cs_main";
//        model = "cs_5_0";
//        break;
//    default:
//        assert(false && "Unsupported Shader Stage!");
//    }
//    default_entrypoint =
//        input.entryPoint.empty() ? default_entrypoint : input.entryPoint;
//
//    std::string macro_key;
//    for (DXShaderMacro const& macro : input.macros) {
//        macro_key += macro.name;
//        macro_key += macro.value;
//    }
//    uint64 macro_hash = crc64(macro_key.c_str(), macro_key.size());
//
//    std::string build_string =
//        input.flags & DXShaderCompilerFlagBit_DEBUG ? "debug" : "release";
//    char cache_path[256];
//    sprintf_s(cache_path, "%s%s_%s_%llx_%s.bin",
//              shaders_cache_directory.c_str(),
//              GetFilenameWithoutExtension(input.sourceFile).c_str(),
//              default_entrypoint.c_str(), macro_hash, build_string.c_str());
//
//    if (CheckCache(cache_path, input, output))
//        return true;
//     //spdlog::info("Shader {%s} not found in cache. Compiling...",
//     //             input.sourceFile.c_str());
//     // default_entrypoint.c_str()
//
//compile:
//    uint32 shader_compile_flags = D3DCOMPILE_ENABLE_STRICTNESS;
//    if (input.flags & DXShaderCompilerFlagBit_DisableOptimization)
//        shader_compile_flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
//    if (input.flags & DXShaderCompilerFlagBit_DEBUG)
//        shader_compile_flags |= D3DCOMPILE_DEBUG;
//    std::vector<D3D_SHADER_MACRO> defines{};
//    defines.resize(input.macros.size());
//
//    for (uint32 i = 0; i < input.macros.size(); ++i) {
//        defines[i].Name = (char*)malloc(sizeof(input.macros[i].name));
//        defines[i].Definition = (char*)malloc(sizeof(input.macros[i].value));
//
//        strcpy(const_cast<char*>(defines[i].Name),
//               input.macros[i].name.c_str());
//        strcpy(const_cast<char*>(defines[i].Definition),
//               input.macros[i].value.c_str());
//    }
//    defines.push_back({NULL, NULL});
//
//    ID3DBlob* bytecode_blob = nullptr;
//    ID3DBlob* error_blob = nullptr;
//
//    CShaderInclude includer(GetParentPath(input.sourceFile).c_str());
//    HRESULT hr = D3DCompileFromFile(
//        ToWideString(input.sourceFile).c_str(), defines.data(), &includer,
//        default_entrypoint.c_str(), model.c_str(), shader_compile_flags, 0,
//        &bytecode_blob, &error_blob);
//
//    auto const& includes = includer.GetIncludes();
//    if (FAILED(hr)) {
//        if (error_blob) {
//            char const* err_msg =
//                reinterpret_cast<char const*>(error_blob->GetBufferPointer());
//            spdlog::error("%s", err_msg);
//            std::string msg =
//                "Click OK after you have fixed the following errors: \n";
//            msg += err_msg;
//            int32 result = MessageBoxA(NULL, msg.c_str(), NULL, MB_OKCANCEL);
//            if (result == IDOK)
//                goto compile;
//            else if (result == IDCANCEL)
//                return false;
//        }
//        return false;
//    }
//    for (auto& define : defines) {
//        if (define.Name)
//            free((void*)define.Name);
//        if (define.Definition)
//            free((void*)define
//                     .Definition); // change malloc and free to new and delete
//    }
//
//    uint64 shader_hash = crc64((char*)bytecode_blob->GetBufferPointer(),
//                               bytecode_blob->GetBufferSize());
//
//    output.shader_bytecode.bytecode.resize(bytecode_blob->GetBufferSize());
//    std::memcpy(output.shader_bytecode.GetPointer(),
//                bytecode_blob->GetBufferPointer(),
//                bytecode_blob->GetBufferSize());
//    output.includes = includes;
//    output.includes.push_back(input.sourceFile);
//    output.hash = shader_hash;
//    SaveToCache(cache_path, output);
//    return true;
//}

bool CompileShader(DXShaderDesc const& input, DXShaderCompileOutput& output) {
    output = DXShaderCompileOutput{};
    std::string default_entrypoint, model;
    switch (input.stage) {
    case DXShaderStage::VS:
        default_entrypoint = "vs_main";
        model = "vs_5_0";
        break;
    case DXShaderStage::PS:
        default_entrypoint = "ps_main";
        model = "ps_5_0";
        break;
    case DXShaderStage::HS:
        default_entrypoint = "hs_main";
        model = "hs_5_0";
        break;
    case DXShaderStage::DS:
        default_entrypoint = "ds_main";
        model = "ds_5_0";
        break;
    case DXShaderStage::GS:
        default_entrypoint = "gs_main";
        model = "gs_5_0";
        break;
    case DXShaderStage::CS:
        default_entrypoint = "cs_main";
        model = "cs_5_0";
        break;
    default:
        assert(false && "Unsupported Shader Stage!");
    }
    default_entrypoint =
        input.entryPoint.empty() ? default_entrypoint : input.entryPoint;
compile:
    uint32 shader_compile_flags = 0;
    if (input.flags & DXShaderCompilerFlagBit_DisableOptimization)
        shader_compile_flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
    if (input.flags & DXShaderCompilerFlagBit_DEBUG)
        shader_compile_flags |= D3DCOMPILE_DEBUG;

    ID3DBlob* bytecode_blob = nullptr;
    ID3DBlob* error_blob = nullptr;

    HRESULT hr = D3DCompileFromFile(
        ToWideString(input.sourceFile).c_str(), 0,
                                    D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                    default_entrypoint.c_str(),
        model.c_str(), shader_compile_flags, 0, &bytecode_blob, &error_blob);

    if (FAILED(hr)) {
        // no file
        if ((hr & D3D11_ERROR_FILE_NOT_FOUND) != 0) {
            std::cout << "File not found." << std::endl;
        }

        // if it get error, print error.
        if (error_blob) {
            std::cout << "Shader complie error\n"
                      << (char*)error_blob->GetBufferPointer() << std::endl;
        }
        return false;
    }
    output.shader_bytecode.bytecode.resize(bytecode_blob->GetBufferSize());
    std::memcpy(output.shader_bytecode.GetPointer(),
                bytecode_blob->GetBufferPointer(),
                bytecode_blob->GetBufferSize());
    //output.includes = includes;
    //output.includes.push_back(input.sourceFile);
    //output.hash = shader_hash;
    return true;
}

void FillInputLayoutDesc(DXShaderBytecode const& blob,
                         DXInputLayoutDesc& input_desc) {
    ID3D11ShaderReflection* vertex_shader_reflection = nullptr;
    HR(D3DReflect(blob.GetPointer(), blob.GetLength(),
                  IID_ID3D11ShaderReflection,
                  (void**)&vertex_shader_reflection));

    D3D11_SHADER_DESC shader_desc;
    vertex_shader_reflection->GetDesc(&shader_desc);

    input_desc.elements.clear();
    input_desc.elements.resize(shader_desc.InputParameters);
    for (uint32 i = 0; i < shader_desc.InputParameters; i++) {
        D3D11_SIGNATURE_PARAMETER_DESC param_desc;
        vertex_shader_reflection->GetInputParameterDesc(i, &param_desc);

        input_desc.elements[i].semantic_name = param_desc.SemanticName;
        input_desc.elements[i].semantic_index = param_desc.SemanticIndex;
        input_desc.elements[i].input_slot = 0;
        input_desc.elements[i].aligned_byte_offset =
            D3D11_APPEND_ALIGNED_ELEMENT;
        input_desc.elements[i].input_slot_class =
            DXInputClassification::PerVertexData;

        if (input_desc.elements[i].semantic_name.starts_with("INSTANCE")) {
            input_desc.elements[i].input_slot = 1;
            input_desc.elements[i].input_slot_class =
                DXInputClassification::PerInstanceData;
        }

        if (param_desc.Mask == 1) {
            if (param_desc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
                input_desc.elements[i].format = DXFormat::R32_UINT;
            else if (param_desc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
                input_desc.elements[i].format = DXFormat::R32_SINT;
            else if (param_desc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
                input_desc.elements[i].format = DXFormat::R32_FLOAT;
        } else if (param_desc.Mask <= 3) {
            if (param_desc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
                input_desc.elements[i].format = DXFormat::R32G32_UINT;
            else if (param_desc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
                input_desc.elements[i].format = DXFormat::R32G32_SINT;
            else if (param_desc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
                input_desc.elements[i].format = DXFormat::R32G32_FLOAT;
        } else if (param_desc.Mask <= 7) {
            if (param_desc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
                input_desc.elements[i].format = DXFormat::R32G32B32_UINT;
            else if (param_desc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
                input_desc.elements[i].format = DXFormat::R32G32B32_SINT;
            else if (param_desc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
                input_desc.elements[i].format = DXFormat::R32G32B32_FLOAT;
        } else if (param_desc.Mask <= 15) {
            if (param_desc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
                input_desc.elements[i].format = DXFormat::R32G32B32A32_UINT;
            else if (param_desc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
                input_desc.elements[i].format = DXFormat::R32G32B32A32_SINT;
            else if (param_desc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
                input_desc.elements[i].format = DXFormat::R32G32B32A32_FLOAT;
        }
    }
}
} // namespace DXShaderCompiler
} // namespace Riley