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
        target_compile_definitions(${target} PRIVATE
            $<$<NOT:$<CONFIG:Debug>>:_FORTIFY_SOURCE=2>
        )
        target_compile_options(${target} PRIVATE
            -fstack-protector-strong
        )
    endif()
endfunction()
