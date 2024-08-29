# Riley-Engine

## Install
```
$vcpkg install directxtk:x64-windows
$vcpkg install directxmath:x64-windows
$vcpkg install assimp:x64-windows
$vcpkg install spdlog:X64-windows
```

## Engine Features
- Sparse-Set ECS (entt ECS)
- ImGui GUI

## Graphics Features
- View Frustum Culling
- Shadow Mapping
- Point Shadow Mapping
- Cascade Shadow Mapping
- Unreal Sphere Light
- Deferred Lighting
- Tiled Deferred Lighting ([ref](https://www.intel.com/content/www/us/en/developer/articles/technical/deferred-rendering-for-current-and-future-rendering-pipelines.html))
- Halo Light Effect
- Gods Ray(Light Shaft) ([ref](https://developer.nvidia.com/gpugems/gpugems3/part-ii-light-and-shadows/chapter-13-volumetric-light-scattering-post-process))
- SSAO
- SSR (using 3D Ray Marching in view space)
- AABB
