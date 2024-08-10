#pragma once
#include "Enums.h"

namespace Riley
{

static constexpr uint32 AO_NOISE_DIM = 8;
static constexpr uint32 SSAO_KERNEL_SIZE = 16;
static constexpr uint32 SHADOW_MAP_SIZE = 2048;
static constexpr uint32 SHADOW_CASCADE_SIZE = 2048;
static constexpr uint32 SHADOW_CUBE_SIZE = 512;
static constexpr uint32 CASCADE_COUNT = 4;

struct RenderSetting
{
   // ao
   AmbientOcclusion ao = AmbientOcclusion::SSAO;
   float ssaoPower = 2.0f;
   float ssaoRadius = 0.5f;
};
} // namespace Riley