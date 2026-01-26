# 設定 CMake 最低版本要求 (建議 3.15 以上以支援現代語法)
cmake_minimum_required(VERSION 3.15)

# 設定專案名稱 & 版本
# 注：這邊的專案名稱使用大駝峰式命名法 (PascalCase)，為了給 CMake 和 IDE 看
# CXX 表示專案使用 C++ 開發，直接檢查系統有無安裝 C++ 編譯器(g++)，不需要額外檢查 C 編譯器(gcc)
project(MicroConsoleRuntime VERSION 0.1.0 LANGUAGES CXX)

# 設定 C++ 標準為 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON) # 強制要求使用 C++17 標準
set(CMAKE_CXX_EXTENSIONS OFF) # 確保不使用 GCC 特有擴充，保證跨平台相容性

# 將警告視為錯誤，確保程式碼品質
if(MSVC)
    # /W4 開啟大部分警告，/WX 將警告視為錯誤
    add_compile_options(/W4 /WX)
else()
    # -Wall -Wextra 開啟大部分警告，-Wpedantic 嚴格遵守標準，-Werror 將警告視為錯誤
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# 設定輸出目錄為專案根目錄下的 bin 資料夾
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 引入 GoogleTest 套件 (用於單元測試)
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# 為了避免 GoogleTest 使用自己的 CMake 設定覆蓋專案設定
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# 加入以下子目錄
add_subdirectory(src)

# 加入測試目錄
enable_testing()
add_subdirectory(tests)

# add_subdirectory(external)