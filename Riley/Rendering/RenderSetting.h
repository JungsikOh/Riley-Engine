#pragma once
#include "Enums.h"

namespace Riley
{
struct RenderSetting
{
   // ao
   AmbientOcclusion ao = AmbientOcclusion::SSAO;
   float ssaoPower = 4.0f;
   float ssaoRadius = 1.0f;
};
} // namespace Riley