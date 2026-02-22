cmake_minimum_required(VERSION 3.15)

# Basic project information.
project(micro-console-runtime 
    VERSION 0.2.0
    DESCRIPTION "High-performance C++ runtime simulation for embedded systems."
    LANGUAGES CXX
)

# Set up the C++ standards.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Disable compiler-specific extensions (Ensure cross-platform portability).

# Output compile_commands.json to make C/C++ extension fetch include path correctly.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable and regard all warning as error (To ensure the code quality).
if(MSVC) # Windows enviironment.
    # /W4 enables most of the warning. 
    # /WX regards warning as error.
    add_compile_options(/W4 /WX)
else()
    # -Wall -Wextra enable most of the warning. 
    # -Wpedantic follows the standard strictly.
    # -Werror regards warning as error.
    add_compile_options(-Wall -Wextra -Werror -Wpedantic)
endif()

# Set up the output directories.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)

# Introduce Google Test
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# Prevent overriding the project's compiler/linker settings.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Introduce Google Benchmark
FetchContent_Declare(
    googlebenchmark
    URL https://github.com/google/benchmark/archive/refs/tags/v1.8.3.zip
)
# Disable Benchmark built-in tests (To save CI compile time).
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
# Disable the installation rules for the Google Benchmark library.
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googlebenchmark)

# Enable CTest system.
enable_testing()

# Add the subdirectories.
add_subdirectory(src)
add_subdirectory(tests)
# add_subdirectory(external)