cmake_minimum_required(VERSION 3.20)

# --------------------------------------------------------------------------- #
# Project metadata
# --------------------------------------------------------------------------- #
project(hello
    VERSION 1.0.0
    DESCRIPTION "A small, modern C 'Hello, World!' library and CLI."
    LANGUAGES C
)

# --------------------------------------------------------------------------- #
# Standards & global settings
# --------------------------------------------------------------------------- #
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Symbols are hidden by default; only HELLO_API-tagged symbols are exported.
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

# Generate compile_commands.json for clangd / clang-tidy / IDEs.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default to Release if the user didn't pick a configuration.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        Debug Release RelWithDebInfo MinSizeRel)
endif()

# Treat this as a top-level vs subproject build (lets consumers `add_subdirectory`).
set(HELLO_IS_TOP_LEVEL OFF)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(HELLO_IS_TOP_LEVEL ON)
endif()

# --------------------------------------------------------------------------- #
# Options
# --------------------------------------------------------------------------- #
option(HELLO_BUILD_TESTS    "Build unit tests"           ${HELLO_IS_TOP_LEVEL})
option(HELLO_BUILD_EXAMPLES "Build example programs"     ${HELLO_IS_TOP_LEVEL})
option(HELLO_WARNINGS_AS_ERRORS "Treat warnings as errors" ${HELLO_IS_TOP_LEVEL})
option(HELLO_ENABLE_ASAN    "Enable AddressSanitizer"    OFF)
option(HELLO_ENABLE_UBSAN   "Enable UndefinedBehaviorSanitizer" OFF)
option(HELLO_ENABLE_MSAN    "Enable MemorySanitizer (clang only)" OFF)
option(HELLO_ENABLE_TSAN    "Enable ThreadSanitizer"     OFF)
option(HELLO_ENABLE_COVERAGE "Enable code-coverage instrumentation" OFF)
option(HELLO_BUILD_FUZZERS  "Build libFuzzer-based fuzz targets (Clang only)" OFF)
option(BUILD_SHARED_LIBS    "Build libhello as a shared library" OFF)

# --------------------------------------------------------------------------- #
# Helper modules
# --------------------------------------------------------------------------- #
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompilerWarnings)
include(Sanitizers)

# --------------------------------------------------------------------------- #
# Generated version header
# --------------------------------------------------------------------------- #
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/hello/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/include/hello/version.h"
    @ONLY
)

# --------------------------------------------------------------------------- #
# Library target
# --------------------------------------------------------------------------- #
add_library(hello src/hello.c)
add_library(hello::hello ALIAS hello)

target_include_directories(hello
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/generated/include>
        $<INSTALL_INTERFACE:include>
)

set_target_properties(hello PROPERTIES
    VERSION   ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME hello
    POSITION_INDEPENDENT_CODE ON
)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(hello PRIVATE HELLO_BUILD_SHARED)
    target_compile_definitions(hello INTERFACE HELLO_USE_SHARED)
endif()

hello_apply_warnings(hello)
hello_apply_sanitizers(hello)

# --------------------------------------------------------------------------- #
# CLI executable
# --------------------------------------------------------------------------- #
add_executable(hello_cli src/main.c)
set_target_properties(hello_cli PROPERTIES OUTPUT_NAME hello)
target_link_libraries(hello_cli PRIVATE hello::hello)
hello_apply_warnings(hello_cli)
hello_apply_sanitizers(hello_cli)

# --------------------------------------------------------------------------- #
# Examples
# --------------------------------------------------------------------------- #
if(HELLO_BUILD_EXAMPLES)
    add_executable(greet_example examples/greet_example.c)
    target_link_libraries(greet_example PRIVATE hello::hello)
    hello_apply_warnings(greet_example)
    hello_apply_sanitizers(greet_example)
endif()

# --------------------------------------------------------------------------- #
# Tests
# --------------------------------------------------------------------------- #
if(HELLO_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(HELLO_BUILD_FUZZERS)
    if(CMAKE_C_COMPILER_ID MATCHES "Clang")
        add_subdirectory(tests/fuzz)
    else()
        message(WARNING
            "HELLO_BUILD_FUZZERS=ON ignored: libFuzzer requires Clang "
            "(got ${CMAKE_C_COMPILER_ID}). Re-run with CC=clang to enable.")
        set(HELLO_BUILD_FUZZERS OFF CACHE BOOL "" FORCE)
    endif()
endif()

# --------------------------------------------------------------------------- #
# Install / packaging
# --------------------------------------------------------------------------- #
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS hello hello_cli
    EXPORT helloTargets
    RUNTIME       DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY       DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE       DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/hello
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/generated/include/hello/version.h"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hello
)

install(EXPORT helloTargets
    FILE helloTargets.cmake
    NAMESPACE hello::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/hello
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/helloConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/helloConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/helloConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/hello
)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/helloConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/helloConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/hello
)

# pkg-config integration for non-CMake consumers. The .pc file is made
# relocatable by deriving prefix from ${pcfiledir} at runtime; the relative
# distance from <libdir>/pkgconfig back to <prefix> is computed at configure
# time so this works for lib/, lib64/, and Debian multiarch layouts alike.
file(RELATIVE_PATH HELLO_PC_RELATIVE_PREFIX
    "${CMAKE_INSTALL_FULL_LIBDIR}/pkgconfig"
    "${CMAKE_INSTALL_PREFIX}"
)
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/hello.pc.in"
    "${CMAKE_CURRENT_BINARY_DIR}/hello.pc"
    @ONLY
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/hello.pc"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

# --------------------------------------------------------------------------- #
# Summary
# --------------------------------------------------------------------------- #
message(STATUS "hello ${PROJECT_VERSION}")
message(STATUS "  C standard:        C${CMAKE_C_STANDARD}")
message(STATUS "  Build type:        ${CMAKE_BUILD_TYPE}")
message(STATUS "  Shared library:    ${BUILD_SHARED_LIBS}")
message(STATUS "  Tests:             ${HELLO_BUILD_TESTS}")
message(STATUS "  Examples:          ${HELLO_BUILD_EXAMPLES}")
message(STATUS "  ASan/UBSan/MSan/TSan: ${HELLO_ENABLE_ASAN}/${HELLO_ENABLE_UBSAN}/${HELLO_ENABLE_MSAN}/${HELLO_ENABLE_TSAN}")
message(STATUS "  Coverage:          ${HELLO_ENABLE_COVERAGE}")
