workspace "DirectX11Basics"
    architecture "x64"
    
    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Inlcude directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "libs/GLFW/include"

-- Include GLFW project --
include "libs/GLFW"

project "DirectX11Basics"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "On"
    systemversion "latest"

    files
    {
        "src/**.h",
        "src/**.hpp",
        "src/**.cpp"
    }

    includedirs
    {
        "%{IncludeDir.GLFW}",
    }

    links
    {
        "GLFW",
        "D3D11",
    }