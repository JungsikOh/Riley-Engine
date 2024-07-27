#pragma once
//std headers + win32/d3d12
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <array>
#include <queue>
#include <mutex>
#include <thread>
#include <optional>
#include <algorithm>
#include <functional>
#include <span>
#include <unordered_set>
#include <type_traits>
#include <unordered_map>
#include <map>
#include <d3d11_3.h>
#include <wrl.h>
#include <dxgi1_3.h>
#include <windows.h>

//external utility
#include <DirectXMath.h>
#include "entt.hpp"

#pragma warning(push, 0)
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#pragma warning(pop)

#define IMGUI_DEFINE_MATH_OPERATORS
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_internal.h"

//project utility
#include "Core/CoreTypes.h"
#include "Core/Log.h"
#include "Core/Input.h"
#include "Math/MathTypes.h"
