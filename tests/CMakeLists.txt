# 定義測試執行檔名稱
set(TEST_EXECUTABLE "micro-test")

# 測試用程式碼
add_executable(${TEST_EXECUTABLE} SlabAllocatorTest.cpp)

# 連結 GoogleTest 函式庫
target_link_libraries(${TEST_EXECUTABLE} PRIVATE GTest::gtest_main)

# 連結專案程式碼
# 由於尚未將 src 定義為函式庫，因此這邊直接連結 src 下的原始碼檔案

# 暫時解法：直接指定 Include 路徑
target_include_directories(${TEST_EXECUTABLE} PRIVATE ${CMAKE_SOURCE_DIR}/include)

# 暫時解法：把 SlabAllocator.cpp 直接加入測試執行檔
target_sources(${TEST_EXECUTABLE} PRIVATE ${CMAKE_SOURCE_DIR}/src/SlabAllocator.cpp)

# 註冊測試
include(GoogleTest)
gtest_discover_tests(${TEST_EXECUTABLE})