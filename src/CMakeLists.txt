add_library(mcr_core 
    slab_allocator.cpp 
    slab_manager.cpp
)

target_include_directories(mcr_core PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(mcr_core PRIVATE mcr_project_warnings)