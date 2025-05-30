project "GLFW"
    kind "StaticLib"
    language "C"
    staticruntime "on"
    location "GLFW"
    
    targetdir("%{wks.location}/bin/" ..outputdir.. "/%{prj.name}")
    objdir("%{wks.location}/bin-int/" ..outputdir.. "/%{prj.name}")

    files{
        "%{prj.name}/include/GLFW/glfw3.h",
        "%{prj.name}/include/GLFW/glfw3native.h",
        "%{prj.name}/src/glfw_config.h",
        "%{prj.name}/src/context.c",
        "%{prj.name}/src/init.c",
        "%{prj.name}/src/input.c",
        "%{prj.name}/src/monitor.c",
        "%{prj.name}/src/vulkan.c",
        "%{prj.name}/src/window.c",
        "%{prj.name}/src/platform.c",
        "%{prj.name}/src/null_init.c",
        "%{prj.name}/src/null_monitor.c",
        "%{prj.name}/src/null_window.c",
        "%{prj.name}/src/null_joystick.c",
    }

    filter "system:windows"
        systemversion "latest"

        files{
            "%{prj.name}/src/win32_init.c",
            "%{prj.name}/src/win32_joystick.c",
            "%{prj.name}/src/win32_monitor.c",
            "%{prj.name}/src/win32_module.c",
            "%{prj.name}/src/win32_time.c",
            "%{prj.name}/src/win32_thread.c",
            "%{prj.name}/src/win32_window.c",
            "%{prj.name}/src/wgl_context.c",
            "%{prj.name}/src/egl_context.c",
            "%{prj.name}/src/osmesa_context.c",
        }

        defines{
            "_GLFW_WIN32",
            "_CRT_SECURE_NO_WARNINGS",
        }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"