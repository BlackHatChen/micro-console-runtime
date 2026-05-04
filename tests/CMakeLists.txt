# ------------------------------------------------------------
# Google Test Settings
# ------------------------------------------------------------

add_executable(mcr_test 
    slab_allocator_test.cpp
    slab_manager_test.cpp
)

target_link_libraries(mcr_test 
    PRIVATE 
    mcr_core 
    mcr_project_warnings
    gtest_main
)

include(GoogleTest)
gtest_discover_tests(mcr_test)

# ------------------------------------------------------------
# Google Benchmark Settings
# ------------------------------------------------------------

add_executable(mcr_benchmark 
    benchmark_slab.cpp
)

target_link_libraries(mcr_benchmark 
    PRIVATE 
    mcr_core 
    mcr_project_warnings
    benchmark::benchmark 
    benchmark::benchmark_main
)