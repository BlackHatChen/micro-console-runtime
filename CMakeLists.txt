cmake_minimum_required(VERSION 3.22)

project(micro-console-runtime VERSION 0.3.2 LANGUAGES CXX)

# ------------------------------------------------------------
# Language & tooling & setting
# ------------------------------------------------------------

set(CMAKE_CXX_STANDARD 17)              # Require C++17 features.
set(CMAKE_CXX_STANDARD_REQUIRED ON)     # Force compiler to do C++17.
set(CMAKE_CXX_EXTENSIONS OFF)           # Ensure cross-platform portability.

# Generate compile_commands.json for IDEs and tooling.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build artifacts can be linked safely into shared libs.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Output directories (lib/bin) under build/.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ------------------------------------------------------------
# Project-local warning policy
# ------------------------------------------------------------

option(MCR_WERROR "Treat warnings as errors" ON)    # Define MCR_WERROR flag.

add_library(mcr_project_warnings INTERFACE)         # Create the interface.

target_compile_options(mcr_project_warnings         # Add warning flags to the interface.
    INTERFACE
        # MSVC: /W4 + optional /WX
        $<$<CXX_COMPILER_ID:MSVC>:
            /W4 
            $<$<BOOL:${MCR_WERROR}>:/WX>
        >
        # GCC/Clang: -Wall -Wextra -Wpedantic + optional -Werror (for C++)
        $<$<AND:$<NOT:$<CXX_COMPILER_ID:MSVC>>,$<COMPILE_LANGUAGE:CXX>>:
            -Wall -Wextra -Wpedantic
            $<$<BOOL:${MCR_WERROR}>:-Werror>
        >
        # GCC/Clang: -Wall -Wextra -Wpedantic + optional -Werror (for C)
        $<$<AND:$<NOT:$<CXX_COMPILER_ID:MSVC>>,$<COMPILE_LANGUAGE:C>>:
            -Wall -Wextra -Wpedantic
            $<$<BOOL:${MCR_WERROR}>:-Werror>
        >
)

# ------------------------------------------------------------
# Testing gate (CTest)
# ------------------------------------------------------------

# Provides BUILD_TESTING option and the testing infrastructures.
include(CTest)

# ------------------------------------------------------------
# Third-party dependencies (GoogleTest and Google Benchmark)
# ------------------------------------------------------------

if(BUILD_TESTING)
    include(FetchContent)   # Allow fetching third-party projects.

    FetchContent_Declare(   # Declare where to fetch GoogleTest.
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        URL_HASH SHA256=1f357c27ca988c3f7c6b4bf68a9395005ac6761f034046e9dde0896e3aba00e4
    )
    if(MSVC)
        # For Windows: Prevent overriding the parent project's compiler/linker settings.
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    endif()

    FetchContent_Declare(   # Declare where to fetch Google Benchmark.
        googlebenchmark
        URL https://github.com/google/benchmark/archive/refs/tags/v1.8.3.zip
        URL_HASH SHA256=abfc22e33e3594d0edf8eaddaf4d84a2ffc491ad74b6a7edc6e7a608f690e691
    )
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)   # Disable built-in tests.
    set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)   # Just use the tool, not install it.

    FetchContent_MakeAvailable(googletest googlebenchmark)
endif()

# ------------------------------------------------------------
# Subdirectories
# ------------------------------------------------------------

add_subdirectory(src)
if(BUILD_TESTING)
    add_subdirectory(tests)
endif()