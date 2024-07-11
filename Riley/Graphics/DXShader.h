#pragma once
#include <string>
#include <vector>

namespace Riley {
struct DXShaderMacro {
    std::string name;
    std::string value;
};

enum class DXShaderStage { VS, PS, HS, DS, GS, CS, StageCount };

struct DXShaderBytecode {
    std::vector<uint8> bytecode;

    void SetBytecode(void* data, uint32 dataSize) {
        bytecode.resize(dataSize);
        memcpy(bytecode.data(), data, dataSize);
    }

    void* GetPointer() const { return (void*)bytecode.data(); }
    uint32 GetLength() const { return (uint32)bytecode.size(); }
};

enum DXShaderCompilerFlagBit {
    DXShaderCompilerFlgBit_NONE = 0,
    DXShaderCompilerFlgBit_DEBUG = 1 << 0,
    DXShaderCompilerFlgBit_DisableOptimization = 1 << 1,
};

struct DXShaderDesc {
    DXShaderStage stage = DXShaderStage::StageCount;
    std::string sourceFile = "";
    std::string entryPoint = "";
    std::vector<DXShaderMacro> macros;
    uint64 flags = DXShaderCompilerFlagBit_NONE;
};
} // namespace Riley