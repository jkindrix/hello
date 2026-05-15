# Sanitizer support. Apply with hello_apply_sanitizers(<target>).
#
# Sanitizers are mutually exclusive in some combinations; this module enforces
# the supported overlaps (ASan+UBSan are commonly combined; TSan/MSan are not).

function(hello_apply_sanitizers target)
    set(_flags "")

    if(HELLO_ENABLE_ASAN AND HELLO_ENABLE_TSAN)
        message(FATAL_ERROR "ASan and TSan cannot be enabled simultaneously")
    endif()
    if(HELLO_ENABLE_MSAN AND (HELLO_ENABLE_ASAN OR HELLO_ENABLE_TSAN))
        message(FATAL_ERROR "MSan is incompatible with ASan/TSan")
    endif()

    if(NOT MSVC)
        if(HELLO_ENABLE_ASAN)
            list(APPEND _flags -fsanitize=address -fno-omit-frame-pointer)
        endif()
        if(HELLO_ENABLE_UBSAN)
            list(APPEND _flags -fsanitize=undefined -fno-sanitize-recover=undefined)
        endif()
        if(HELLO_ENABLE_TSAN)
            list(APPEND _flags -fsanitize=thread)
        endif()
        if(HELLO_ENABLE_MSAN)
            if(NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
                message(FATAL_ERROR "MSan is only supported by Clang")
            endif()
            list(APPEND _flags -fsanitize=memory -fsanitize-memory-track-origins=2
                               -fno-omit-frame-pointer)
        endif()
        if(HELLO_ENABLE_COVERAGE)
            list(APPEND _flags --coverage)
        endif()
    elseif(HELLO_ENABLE_ASAN)
        list(APPEND _flags /fsanitize=address)
    endif()

    if(_flags)
        target_compile_options(${target} PRIVATE ${_flags})
        target_link_options(${target} PRIVATE ${_flags})
    endif()
endfunction()
