project "*"
    sysincludedirs {
        "asmjit/src/"
    }

group "external"
project "asmjit"
    kind "StaticLib"
    language "C++"

    defines {
        "ASMJIT_EMBED",
        "ASMJIT_STATIC",
    }

    includedirs "asmjit/src"
    files {
        "asmjit/src/**.cpp"
    }

    defines {
        "ASMJIT_EMBED",
        "ASMJIT_STATIC",
    }

function require_asmjit()
    links "asmjit"

    sysincludedirs {
        -- "asmjit/src/" -- FIXME path inherits wrong
    }

    defines {
        "ASMJIT_EMBED",
        "ASMJIT_STATIC",
    }
end
