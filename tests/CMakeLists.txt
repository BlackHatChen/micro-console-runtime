# ------------------------------------------------------------
# Google Test Settings
# ------------------------------------------------------------

add_executable(mcr_test 
    test_slab.cpp
    test_manager.cpp
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