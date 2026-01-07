# 定義執行檔名稱
set(EXECUTABLE_NAME "micro-runtime")

# 搜尋所有的 .cpp 檔案 (GLOB：命令 CMake 去硬碟搜尋檔案) (搜尋到的結果存入到 SOURCES 變數中)
file(GLOB SOURCES "*.cpp")

# 建立執行檔 Target
add_executable(${EXECUTABLE_NAME} ${SOURCES})

# 設定 include 目錄 (針對 ${EXECUTABLE_NAME} 設定路徑) (當程式碼有 #include "header.h" 時，編譯器會去 include 目錄下尋找 header.h)
target_include_directories(${EXECUTABLE_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)

# 連結執行緒庫 (若找不到 Threads 會直接報錯) (PRIVATE 代表這個 Threads::Threads 只有自己知道)
find_package(Threads REQUIRED)
target_link_libraries(${EXECUTABLE_NAME} PRIVATE Threads::Threads)