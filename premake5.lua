workspace "pacman"
    architecture "x64"
    configurations { "Debug", "Release" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "pacman"
    location "src"
    kind "ConsoleApp"
    language "C++"

    targetdir ("bin/" .. outputdir)
    objdir ("bin-int/" .. outputdir)

    files { "src/**" }

    includedirs "ext/include"

    libdirs "ext/lib"

    filter "system:windows"
        links { "opengl32", "glfw3_mt", "glew32s" }
        defines "GLEW_STATIC"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "latest"

    filter "configurations:Debug"
        symbols "On"

    filter "configurations:Release"
        optimize "On"