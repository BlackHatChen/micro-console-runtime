add_library(mcr_core 
    slab_allocator.cpp 
    slab_manager.cpp
)

# Set up include path.
target_include_directories(mcr_core 
    PUBLIC 
    ${CMAKE_SOURCE_DIR}/include
)

# Enable C++17 standards.
target_compile_features(mcr_core 
    PUBLIC 
    cxx_std_17
)