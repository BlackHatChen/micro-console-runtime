cmake_minimum_required(VERSION 3.15)

# 專案基本資訊
project(micro-console-runtime 
    VERSION 0.1.0
    DESCRIPTION "High-performance C++ runtime simulation for embedded systems."
    LANGUAGES CXX
)

# 設定 C++ 標準
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # 停用編譯器特定的擴充功能 (保證跨平台移植性)

# 輸出 compile_commands.json 以利於 C/C++ 插件正確抓到 Include 路徑
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 設定編譯選項：開啟所有警告，並將警告都視為錯誤 (確保代碼品質)
if(MSVC)
    add_compile_options(/W4 /WX) # /W4 開啟大部分警告，/WX 將警告視為錯誤
else()
    add_compile_options(-Wall -Wextra -Werror -Wpedantic) # -Wall -Wextra 開啟大部分警告，-Wpedantic 嚴格遵守標準，-Werror 將警告視為錯誤
endif()

# 設定輸出目錄
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 添加子目錄
add_subdirectory(src)
# add_subdirectory(tests)
# add_subdirectory(external)