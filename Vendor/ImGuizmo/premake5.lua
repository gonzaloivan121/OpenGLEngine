project "ImGuizmo"
    kind "StaticLib"
    language "C++"
    cppdialect "C++latest"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    includedirs { 
        ".",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.glm}"
    }

    files {
        "ImGuizmo.h",
        "ImGuizmo.cpp"
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        runtime "Release"
        optimize "on"