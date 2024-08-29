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

enum class LightingType : int32
{
	Deferred,
	TiledDeferred,
    TiledDeferred_DEBUG,
};

enum class MaterialAlphaMode : int8
{
	Opaque,
	Blend,
	Mask
};

}