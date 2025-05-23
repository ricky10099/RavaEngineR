#pragma once

#ifdef _WIN32
#	ifndef NOMINMAX
#		define NOMINMAX
#	endif
#	include <Windows.h>
#endif

////////////////////////////////////////////////////////////////////////
// std
////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>
#include <chrono>
using namespace std::literals::chrono_literals;
#include <string>
#include <sstream>
#include <array>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <filesystem>
#include <shobjidl.h>

////////////////////////////////////////////////////////////////////////
// spdlog
////////////////////////////////////////////////////////////////////////
#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#pragma warning(pop)
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

////////////////////////////////////////////////////////////////////////
// GLFW
////////////////////////////////////////////////////////////////////////
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

////////////////////////////////////////////////////////////////////////
// stb_image
////////////////////////////////////////////////////////////////////////
#define STB_IMAGE_IMPLEMENTATION

////////////////////////////////////////////////////////////////////////
// glm
////////////////////////////////////////////////////////////////////////
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#ifndef GLM_ENABLE_EXPERIMENTAL
#	define GLM_ENABLE_EXPERIMENTAL
#endif
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

////////////////////////////////////////////////////////////////////////
// ImGui
////////////////////////////////////////////////////////////////////////
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_vulkan.h>

////////////////////////////////////////////////////////////////////////
// ImGuizmo
////////////////////////////////////////////////////////////////////////
#include <ImGuizmo/ImGuizmo.h>

////////////////////////////////////////////////////////////////////////
// EnTT
////////////////////////////////////////////////////////////////////////
//#include <EnTT/entt.hpp>

////////////////////////////////////////////////////////////////////////
// PhyX
////////////////////////////////////////////////////////////////////////
//#include <PxPhysicsAPI.h>

////////////////////////////////////////////////////////////////////////
// Log
////////////////////////////////////////////////////////////////////////
#include "Engine/System/Log.h"

constexpr auto ASSETS_DIR = "Assets/";

////////////////////////////////////////////////////////////////////////
// Macros
////////////////////////////////////////////////////////////////////////
#define EXPAND_MACRO(x)    x
#define STRINGIFY_MACRO(x) #x

#define BIT(x) (1 << x)

#define BIND_FN(fn) std::bind(&fn, this, std::placeholders::_1)

#define NO_COPY(type)                      \
	type(const type&)            = delete; \
	type& operator=(const type&) = delete;
#define NO_MOVE(type)                 \
	type(type&&)            = delete; \
	type& operator=(type&&) = delete;
#define CAN_MOVE(type)                 \
	type(type&&)            = default; \
	type& operator=(type&&) = default;

////////////////////////////////////////////////////////////////////////
// Typedef
////////////////////////////////////////////////////////////////////////
using i8  = char;
using i16 = short;
using i32 = int;
using i64 = long long;

using u8  = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;

using f32 = float;
using f64 = double;

template <typename T>
using Unique = std::unique_ptr<T>;

template <typename T>
using Shared = std::shared_ptr<T>;

template <typename T>
using Weak = std::weak_ptr<T>;

////////////////////////////////////////////////////////////////////////
// Assert
////////////////////////////////////////////////////////////////////////
#ifdef RV_DEBUG
#	define RV_ENABLE_ASSERTS
#	define RV_DEBUGBREAK() __debugbreak()
#endif

#ifdef RV_ENABLE_ASSERTS
#	define RV_INTERNAL_ASSERT_IMPL(type, check, msg, ...) \
		{                                                    \
			if (!(check)) {                                  \
				##type##ERROR(msg, __VA_ARGS__);             \
				RV_DEBUGBREAK();                           \
			}                                                \
		}
#	define RV_INTERNAL_ASSERT_WITH_MSG(type, check, ...) \
		RV_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
#	define RV_INTERNAL_ASSERT_NO_MSG(type, check)             \
		RV_INTERNAL_ASSERT_IMPL(                               \
			type,                                                \
			check,                                               \
			"Assertion '{0}' failed at {1}:{2}",                 \
			STRINGIFY_MACRO(check),                              \
			std::filesystem::path(__FILE__).filename().string(), \
			__LINE__                                             \
		)

#	define RV_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
#	define RV_INTERNAL_ASSERT_GET_MACRO(...) \
		EXPAND_MACRO(RV_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, RV_INTERNAL_ASSERT_WITH_MSG, RV_INTERNAL_ASSERT_NO_MSG))

#	define ASSERT(...)        EXPAND_MACRO(RV_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(LOG_, __VA_ARGS__))
#	define ENGINE_ASSERT(...) EXPAND_MACRO(RV_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(ENGINE_, __VA_ARGS__))
#else
#	define ASSERT(...)
#	define ENGINE_ASSERT(...)
#endif
