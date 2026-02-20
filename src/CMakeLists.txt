add_library(mcr_core slab_allocator.cpp)
target_include_directories(mcr_core PUBLIC ${CMAKE_SOURCE_DIR}/include) # Set up include path.
target_compile_features(mcr_core PUBLIC cxx_std_17) # Enable C++17 standards.