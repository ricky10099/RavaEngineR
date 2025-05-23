workspace "RavaEngineR"
	architecture "x64"
	startproject "RavaEngineR"

	configurations{
		"Debug",
		"Release",
	}

outputdir = "%{cfg.buildcfg}"
VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["GLFW"]		= "Externals/GLFW/include"
IncludeDir["spdlog"]	= "Externals/spdlog/include"
IncludeDir["glm"]		= "Externals/glm"
IncludeDir["ImGui"]		= "Externals/ImGui"
IncludeDir["ImGuizmo"]		= "Externals/ImGuizmo"

LibDir = {}

project "RavaEngineR"
	location "RavaEngineR"	
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++latest"
	staticruntime "off"

	debugdir "$(SolutionDir)"

	pchheader "rvpch.h"
	pchsource "%{prj.name}/src/rvpch.cpp"

	targetdir ("bin/" ..outputdir.. "/%{prj.name}")
	objdir ("bin-int/" ..outputdir.. "/%{prj.name}")

	files {
		"%{prj.name}/src/**.hpp",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.c",
		"%{prj.name}/RavaEngineR.rc",
		"%{prj.name}/resource.h",
	}
	
	defines {
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE"
	}

	includedirs {
		"%{prj.name}/src",
		"%{VULKAN_SDK}/Include",
		"Externals",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuizmo}",
	}
	
	libdirs {
		"%{VULKAN_SDK}/Lib",
	}

	links {
		"vulkan-1.lib",
		"GLFW",
		"ImGui",
		"ImGuizmo",
	}

	postbuildcommands {
		-- "call $(SolutionDir)CompileShaders.bat",
	}

	buildoptions {
		"/utf-8",
	}

	filter"system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "RV_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines {"RV_RELEASE", "NDEBUG"}
		runtime "Release"
		optimize "on"

group "Externals"
		include "Externals/ImGui.lua"
		include "Externals/ImGuizmo.lua"
		include "Externals/GLFW.lua"

