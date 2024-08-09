#pragma once
#include "Enums.h"

namespace Riley
{
struct RenderSetting
{
   // ao
   AmbientOcclusion ao = AmbientOcclusion::SSAO;
   float ssaoPower = 2.0f;
   float ssaoRadius = 0.5f;
};
} // namespace Riley