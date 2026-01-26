#include <gtest/gtest.h>
#include "SlabAllocator.hpp"

// 定義測試案例 (Test Case)
TEST(SlabAllocatorTest, BasicAllocation) {
    SlabAllocator pool(32, 10); // 準備：建立一個 Pool

    void* ptr1 = pool.allocate(); // 行動：分配記憶體區塊
    EXPECT_NE(ptr1, nullptr); // 驗證：檢查分配是否成功

    void* ptr2 = pool.allocate(); // 行動：再次分配記憶體區塊
    EXPECT_NE(ptr2, nullptr);

    EXPECT_NE(ptr1, ptr2); // 驗證：確保兩次分配的區塊不同
}

TEST(SlabAllocatorTest, AlignmentCheck) {
    // 測試對齊：要求 64 位元對齊
    size_t alignment = 64;
    SlabAllocator pool(16, 5, alignment);

    void* ptr = pool.allocate();

    uintptr_t addr = reinterpret_cast<uintptr_t>(ptr); // 取得指標的位址
    EXPECT_EQ(addr % alignment, 0); // 驗證：檢查是否整除 (addr % 64 == 0)
}