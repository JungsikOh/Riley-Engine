#pragma once
#include "DXShaderProgram.h"

namespace Riley {
struct DXShaderCompileOutput {
    DXShaderBytecode shader_bytecode;
    std::vector<std::string> includes;
    uint64 hash;
};

struct DXInputLayoutDesc;
namespace DXShaderCompiler {
bool CompileShader(DXShaderDesc const& input, DXShaderCompileOutput& output);
void FillInputLayoutDesc(DXShaderBytecode const& blob,
                         DXInputLayoutDesc& input_desc);
} // namespace DXShaderCompiler
} // namespace Riley