#pragma once
#include <DirectXMath.h>
#include "pch.h"

namespace Riley
{
static Vector3 ExtractScaleFromMatrix(Matrix const& m)
{
   Vector3 scale;
   /*scale.x = std::sqrt(m._11 * m._11 + m._21 * m._21 + m._31 * m._31);
   scale.y = std::sqrt(m._12 * m._12 + m._22 * m._22 + m._32 * m._32);
   scale.z = std::sqrt(m._13 * m._13 + m._23 * m._23 + m._33 * m._33);*/

   scale.x = std::sqrt(m._11 * m._11 + m._12 * m._12 + m._13 * m._13);
   scale.y = std::sqrt(m._21 * m._21 + m._22 * m._22 + m._23 * m._23);
   scale.z = std::sqrt(m._31 * m._31 + m._32 * m._32 + m._33 * m._33);

   return scale;
}

static Matrix ExtractRoationMatrix(Matrix const& m)
{
   Vector3 scale = ExtractScaleFromMatrix(m);
   Matrix r;

   r._11 = m._11 / scale.x;
   r._12 = m._12 / scale.x;
   r._13 = m._13 / scale.x;
   r._21 = m._21 / scale.y;
   r._22 = m._22 / scale.y;
   r._23 = m._23 / scale.y;
   r._31 = m._31 / scale.z;
   r._32 = m._32 / scale.z;
   r._33 = m._33 / scale.z;

   return r;
}
} // namespace Riley