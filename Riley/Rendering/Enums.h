#pragma once

namespace Riley
{
enum class LightType : int32
{
   Directional,
   Point,
   Spot
};

enum class AmbientOcclusion : int32
{
	None,
	SSAO
};

enum class MaterialAlphaMode : int8
{
	Opaque,
	Blend,
	Mask
};

}