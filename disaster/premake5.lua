project "disaster"
    kind "ConsoleApp"
    language "C++"

    files {
        "**.cpp",
        "**.h",
    }

    require_asmjit()
