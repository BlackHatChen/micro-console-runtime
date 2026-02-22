# Google Test Settings
add_executable(mcr_test test_slab_basic.cpp)
target_link_libraries(mcr_test PRIVATE mcr_core gtest_main)
add_test(NAME SlabAllocatorTest COMMAND mcr_test) # Register the test to make CTest fetch it.

# Google Benchmark Settings
add_executable(mcr_benchmark benchmark_slab.cpp)
target_link_libraries(mcr_benchmark PRIVATE mcr_core benchmark::benchmark benchmark::benchmark_main)