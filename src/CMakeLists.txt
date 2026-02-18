# 定義執行檔名稱、編譯與連結相關資源
add_executable(micro-runtime slab_allocator.cpp main.cpp)

# 設定 Include 路徑
target_include_directories(micro-runtime PRIVATE ${CMAKE_SOURCE_DIR}/include)

# 開啟 C++17 標準
target_compile_features(micro-runtime PRIVATE cxx_std_17)