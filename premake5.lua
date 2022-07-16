workspace "disaster"
    configurations { "Debug", "Release"}
    targetdir "bin/%{cfg.buildcfg}"
    objdir "obj/%{cfg.buildcfg}"
    startproject "disaster"
    debugdir "."

    filter "system:windows"
        location "."
        platforms { "x64" }

    filter { "system:windows", "language:not C#" }
        defines {
            "_WIN32",
            "_WIN32_WINNT=0x0601",
            "_CRT_SECURE_NO_WARNINGS",
        }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        symbols "On"

    include "external"
    include "disaster"
