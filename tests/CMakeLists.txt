# Google Test Settings
add_executable(mcr_test 
    test_slab.cpp
    test_manager.cpp
)

target_link_libraries(mcr_test 
    PRIVATE 
    mcr_core gtest_main
)

# Register the test to make CTest fetch it.
add_test(NAME AllMemoryCoreTests COMMAND $<TARGET_FILE:mcr_test>)

# ----------------------------------------------------------------

# Google Benchmark Settings
add_executable(mcr_benchmark 
    benchmark_slab.cpp
)

target_link_libraries(mcr_benchmark 
    PRIVATE 
    mcr_core benchmark::benchmark benchmark::benchmark_main
)