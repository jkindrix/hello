# Centralized warning flags. Apply with hello_apply_warnings(<target>).
#
# We are aggressive by design: in a fresh codebase, every warning is a bug
# we should fix now, not defer.

function(hello_apply_warnings target)
    if(MSVC)
        set(_warnings
            /W4
            /permissive-
            /w14242 /w14254 /w14263 /w14265 /w14287 /we4289
            /w14296 /w14311 /w14545 /w14546 /w14547 /w14549
            /w14555 /w14619 /w14640 /w14826 /w14905 /w14906
            /w14928
        )
        if(HELLO_WARNINGS_AS_ERRORS)
            list(APPEND _warnings /WX)
        endif()
    else()
        set(_warnings
            -Wall
            -Wextra
            -Wpedantic
            -Wshadow
            -Wconversion
            -Wsign-conversion
            -Wcast-align
            -Wcast-qual
            -Wwrite-strings
            -Wstrict-prototypes
            -Wmissing-prototypes
            -Wmissing-declarations
            -Wredundant-decls
            -Wnull-dereference
            -Wdouble-promotion
            -Wformat=2
            -Wformat-security
            -Wundef
            -Wvla
            -Wpointer-arith
            -Wswitch-default
            -Wswitch-enum
            -Winit-self
        )
        if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
            list(APPEND _warnings
                -Wduplicated-cond
                -Wduplicated-branches
                -Wlogical-op
                -Wjump-misses-init
                -Wtrampolines
            )
        endif()
        if(HELLO_WARNINGS_AS_ERRORS)
            list(APPEND _warnings -Werror)
        endif()
    endif()

    target_compile_options(${target} PRIVATE ${_warnings})

    # Hardening flags useful on GCC / Clang in release builds.
    if(NOT MSVC)
        # Pick the strongest _FORTIFY_SOURCE the toolchain accepts. =3 needs
        # glibc >= 2.34 and GCC >= 12 / Clang >= 9.
        if(NOT DEFINED HELLO_FORTIFY_LEVEL_CACHED)
            include(CheckCSourceCompiles)
            set(CMAKE_REQUIRED_FLAGS "-O1 -Werror -D_FORTIFY_SOURCE=3")
            check_c_source_compiles("#include <string.h>
int main(void){char b[8]; strcpy(b,\"hi\"); return b[0];}" HELLO_HAVE_FORTIFY3)
            unset(CMAKE_REQUIRED_FLAGS)
            if(HELLO_HAVE_FORTIFY3)
                set(HELLO_FORTIFY_LEVEL_CACHED 3 CACHE INTERNAL "")
            else()
                set(HELLO_FORTIFY_LEVEL_CACHED 2 CACHE INTERNAL "")
            endif()
        endif()
        target_compile_definitions(${target} PRIVATE
            $<$<NOT:$<CONFIG:Debug>>:_FORTIFY_SOURCE=${HELLO_FORTIFY_LEVEL_CACHED}>
        )
        target_compile_options(${target} PRIVATE
            -fstack-protector-strong
        )
    endif()

    # Link-time hardening: full RELRO + BIND_NOW + PIE for executables on ELF.
    # Skipped on macOS (Mach-O) and Windows. Shared libs already get PIC via
    # POSITION_INDEPENDENT_CODE; PIE is only meaningful for executables.
    get_target_property(_target_type ${target} TYPE)
    if(_target_type STREQUAL "EXECUTABLE"
       AND CMAKE_SYSTEM_NAME STREQUAL "Linux"
       AND NOT MSVC)
        set_property(TARGET ${target} PROPERTY POSITION_INDEPENDENT_CODE ON)
        target_link_options(${target} PRIVATE
            -pie
            -Wl,-z,relro
            -Wl,-z,now
            -Wl,-z,noexecstack
        )
    endif()
endfunction()
