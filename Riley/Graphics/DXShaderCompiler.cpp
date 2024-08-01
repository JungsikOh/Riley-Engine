#include "DXShaderCompiler.h"
#include "../Core/Paths.h"
#include "../Utilities/FileUtil.h"
#include "../Utilities/HashUtil.h"
#include "../Utilities/StringUtil.h"
#include "DXShader.h"
#include "cereal/archives/binary.hpp"
#include <d3dcompiler.h>
#include <fstream>

namespace Riley
{

namespace DXShaderCompiler
{

bool CompileShader(DXShaderDesc const& input, DXShaderCompileOutput& output)
{
   output = DXShaderCompileOutput{};
   std::string default_entrypoint, model;
   switch (input.stage)
      {
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
         RI_CORE_ERROR("Unsuported Shader Stage!");
         assert(false && "Unsupported Shader Stage!");
      }
   default_entrypoint = input.entryPoint.empty() ? default_entrypoint : input.entryPoint;

   uint32 shader_compile_flags = 0;
   if (input.flags & DXShaderCompilerFlagBit_DisableOptimization)
      shader_compile_flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
   if (input.flags & DXShaderCompilerFlagBit_DEBUG)
      shader_compile_flags |= D3DCOMPILE_DEBUG;

   ID3DBlob* bytecode_blob = nullptr;
   ID3DBlob* error_blob = nullptr;

   HRESULT hr = D3DCompileFromFile(ToWideString(input.sourceFile).c_str(), 0, D3D_COMPILE_STANDARD_FILE_INCLUDE, default_entrypoint.c_str(),
                        model.c_str(), shader_compile_flags, 0, &bytecode_blob, &error_blob);

   if (FAILED(hr))
      {
         // no file
         if ((hr & D3D11_ERROR_FILE_NOT_FOUND) != 0)
            {
               std::cout << "File not found." << std::endl;
            }

         // if it get error, print error.
         if (error_blob)
            {
               std::cout << "Shader complie error\n" << (char*)error_blob->GetBufferPointer() << std::endl;
            }
         return false;
      }
   output.shader_bytecode.bytecode.resize(bytecode_blob->GetBufferSize());
   std::memcpy(output.shader_bytecode.GetPointer(), bytecode_blob->GetBufferPointer(), bytecode_blob->GetBufferSize());
   // output.includes = includes;
   output.includes.push_back(input.sourceFile);
   // output.hash = shader_hash;
   return true;
}

void FillInputLayoutDesc(DXShaderBytecode const& blob, DXInputLayoutDesc& input_desc)
{
   ID3D11ShaderReflection* vertex_shader_reflection = nullptr;
   HR(D3DReflect(blob.GetPointer(), blob.GetLength(), IID_ID3D11ShaderReflection, (void**)&vertex_shader_reflection));

   D3D11_SHADER_DESC shader_desc;
   vertex_shader_reflection->GetDesc(&shader_desc);

   input_desc.elements.clear();
   input_desc.elements.resize(shader_desc.InputParameters);
   for (uint32 i = 0; i < shader_desc.InputParameters; i++)
      {
         D3D11_SIGNATURE_PARAMETER_DESC param_desc;
         vertex_shader_reflection->GetInputParameterDesc(i, &param_desc);

         input_desc.elements[i].semantic_name = param_desc.SemanticName;
         input_desc.elements[i].semantic_index = param_desc.SemanticIndex;
         input_desc.elements[i].input_slot = 0;
         input_desc.elements[i].aligned_byte_offset = D3D11_APPEND_ALIGNED_ELEMENT;
         input_desc.elements[i].input_slot_class = DXInputClassification::PerVertexData;

         if (input_desc.elements[i].semantic_name.starts_with("INSTANCE"))
            {
               input_desc.elements[i].input_slot = 1;
               input_desc.elements[i].input_slot_class = DXInputClassification::PerInstanceData;
            }

         if (param_desc.Mask == 1)
            {
               if (param_desc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
                  input_desc.elements[i].format = DXFormat::R32_UINT;
               else if (param_desc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
                  input_desc.elements[i].format = DXFormat::R32_SINT;
               else if (param_desc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
                  input_desc.elements[i].format = DXFormat::R32_FLOAT;
            }
         else if (param_desc.Mask <= 3)
            {
               if (param_desc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
                  input_desc.elements[i].format = DXFormat::R32G32_UINT;
               else if (param_desc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
                  input_desc.elements[i].format = DXFormat::R32G32_SINT;
               else if (param_desc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
                  input_desc.elements[i].format = DXFormat::R32G32_FLOAT;
            }
         else if (param_desc.Mask <= 7)
            {
               if (param_desc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
                  input_desc.elements[i].format = DXFormat::R32G32B32_UINT;
               else if (param_desc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
                  input_desc.elements[i].format = DXFormat::R32G32B32_SINT;
               else if (param_desc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
                  input_desc.elements[i].format = DXFormat::R32G32B32_FLOAT;
            }
         else if (param_desc.Mask <= 15)
            {
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