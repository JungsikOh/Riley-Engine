#pragma once
#include <DirectXCollision.h>
#include <concepts>

namespace Riley
{
namespace
{
using namespace DirectX;

template <typename Iterator>
BoundingBox AABBFromRange(Iterator begin, Iterator end)
{
   using V = typename std::iterator_traits<Iterator>::value_type;

   auto extremesX = std::minmax_element(begin, end, [](V const& lhs, V const& rhs) {
      return lhs.position.x < rhs.position.x;
   });

   auto extremesY = std::minmax_element(begin, end, [](V const& lhs, V const& rhs) {
      return lhs.position.y < rhs.position.y;
   });

   auto extremesZ = std::minmax_element(begin, end, [](V const& lhs, V const& rhs) {
      return lhs.position.z < rhs.position.z;
   });

   Vector3 lowerLeft(extremesX.first->position.x, extremesY.first->position.y, extremesZ.first->position.z);
   Vector3 upperRight(extremesX.second->position.x, extremesY.second->position.y, extremesZ.second->position.z);
   Vector3 center((lowerLeft.x + upperRight.x) * 0.5f, (lowerLeft.y + upperRight.y) * 0.5f, (lowerLeft.z + upperRight.z) * 0.5f);
   Vector3 extents(upperRight.x - center.x, upperRight.y - center.y, upperRight.z - center.z);

   return BoundingBox(center, extents);
}

template <typename V>
BoundingBox AABBFromVertices(std::vector<V> const& vertices)
{
   return AABBFromRange(vertices.begin(), vertices.end());
}

static BoundingBox AABBFromPositions(std::vector<Vector3> const& positions) {
   auto begin = positions.begin();
   auto end = positions.end();
   
   auto extremesX = std::minmax_element(begin, end, [](auto const& lhs, auto const& rhs) {
      return lhs.x < rhs.x;
   });

   auto extremesY = std::minmax_element(begin, end, [](auto const& lhs, auto const& rhs) {
      return lhs.y < rhs.y;
   });

   auto extremesZ = std::minmax_element(begin, end, [](auto const& lhs, auto const& rhs) {
      return lhs.z < rhs.z;
   });

   Vector3 lowerLeft(extremesX.first->x, extremesY.first->y, extremesZ.first->z);
   Vector3 upperRight(extremesX.second->x, extremesY.second->y, extremesZ.second->z);
   Vector3 center((lowerLeft.x + upperRight.x) * 0.5f, (lowerLeft.y + upperRight.y) * 0.5f, (lowerLeft.z + upperRight.z) * 0.5f);
   Vector3 extents(upperRight.x - center.x, upperRight.y - center.y, upperRight.z - center.z);

   return BoundingBox(center, extents);
}

} // namespace
} // namespace Riley