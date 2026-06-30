include "Dependencies.lua"

-- premake5.lua
workspace "OpenGLEngine"
    architecture "x86_64"
    startproject "OpenGLEngine"

    configurations {
        "Debug",
        "Release",
        "Dist"
    }

    multiprocessorcompile ("On")

    -- Workspace-wide build options for MSVC
    filter "system:windows"
        buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus", "/utf-8" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
	include "Vendor/glfw"
	include "Vendor/glad"
	include "Vendor/imgui"
    include "Vendor/ImGuizmo"
    include "Vendor/yaml-cpp"
    include "Vendor/portaudio"
group ""

project "OpenGLEngine"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++latest"
    targetdir "bin/%{cfg.buildcfg}"
    staticruntime "on"
   
	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "Source/**.h",
        "Source/**.cpp",
        "Vendor/stb/stb_image.h",
        "Vendor/stb/stb_image_write.h",
		"Vendor/glm/glm/**.hpp",
		"Vendor/glm/glm/**.inl",
        "Vendor/ImGuizmo/*.h",
        "Vendor/portaudio/include/*.h",
        "Vendor/dr_libs/*.h"
    }

    defines {
        "IMGUI_DEFINE_MATH_OPERATORS",
        "_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE",
        "YAML_CPP_STATIC_DEFINE",
        "GLM_ENABLE_EXPERIMENTAL",
        "DR_WAV_IMPLEMENTATION"
    }

    includedirs {
        "Source",
		"%{IncludeDir.stb}",
		"%{IncludeDir.glfw}",
		"%{IncludeDir.glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.yamlcpp}",
        "%{IncludeDir.ImGuizmo}",
        "%{IncludeDir.portaudio}",
        "%{IncludeDir.dr_libs}"
    }

    links {
		"GLFW",
		"Glad",
		"ImGui",
        "ImGuizmo",
        "yamlcpp",
        "portaudio"
    }

    postbuildcommands {
        "{COPYDIR} Assets %{cfg.targetdir}/Assets",
        "{COPYDIR} Internal %{cfg.targetdir}/Internal",
        "{COPYFILE} imgui.ini %{cfg.targetdir}/imgui.ini",
        "{COPYFILE} Settings.yaml %{cfg.targetdir}/Settings.yaml"
    }

    filter "system:windows"
        systemversion "latest"
        defines { "OGE_PLATFORM_WINDOWS" }
        files { "Internal/Icons/AppIcon/appicon.rc" }

    filter "configurations:Debug"
        defines {
            "OGE_DEBUG",
            "OGE_PROFILE"
        }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "OGE_RELEASE" }
        runtime "Release"
        optimize "On"
        symbols "On"

    filter "configurations:Dist"
        kind "WindowedApp"
        defines { "OGE_DIST" }
        runtime "Release"
        optimize "On"
        symbols "Off"