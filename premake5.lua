workspace "Direct3D"
    architecture "x86_64"
    startproject "Sandbox"

    configurations
    {
        "Debug",
        "Release"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["entt"]      = "D3DCore/vendor/entt/include"
IncludeDir["stb_image"] = "D3DCore/vendor/stb_image/"
IncludeDir["ImGui"]     = "D3DCore/vendor/imgui"
IncludeDir["assimp"]    = "D3DCore/vendor/assimp/include"
IncludeDir["spdlog"]    = "D3DCore/vendor/spdlog/include"

include "D3DCore/vendor/imgui"
include "D3DCore/vendor/assimp"

project "D3DCore"
    location "D3DCore"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "d3dcpch.h"
    pchsource "D3DCore/src/d3dcpch.cpp"

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
        "%{prj.name}/src/**.inl",
    }

    includedirs
    {
        "%{prj.name}/src",
        "%{IncludeDir.entt}",
        "%{IncludeDir.stb_image}",
        "%{prj.name}/vendor",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.assimp}",
        "%{IncludeDir.spdlog}",
    }

    links
    {
        "ImGui",
        "assimp"
    }

    --filter "files:%{prj.name}/src/Renderer/ErrorHandling/dxerr.cpp"
        --flags {"NoPCH"}

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        defines "D3DC_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "D3DC_RELEASE"
        runtime "Release"
        optimize "on"

project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
    }

    includedirs
    {
        "%{prj.name}/src",
        "D3DCore/src/",
        "%{IncludeDir.entt}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.assimp}",
        "%{IncludeDir.spdlog}",
    }

    links
    {
        "D3DCore"
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        defines "D3DC_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "D3DC_RELEASE"
        runtime "Release"
        optimize "on"

--[[ project "Raycaster"
    location "Raycaster"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
    }

    includedirs
    {
        "%{prj.name}/src",
        "D3DCore/src/",
        "%{IncludeDir.entt}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.assimp}",
        "%{IncludeDir.spdlog}",
    }

    links
    {
        "D3DCore"
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        defines "D3DC_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "D3DC_RELEASE"
        runtime "Release"
        optimize "on" ]]