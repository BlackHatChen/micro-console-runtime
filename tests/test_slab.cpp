#include <gtest/gtest.h>
#include "slab_allocator.h"
#include <cstdint>

// Original: 20 bytes. Align to 8-byte boundary: 24 bytes.
struct TestObj {
    int id;
    float value;
    float a, b, c;
};

// [Test 1] Basic Allocation
TEST(SlabAllocatorTest, BasicAllocation) {
    // We intentionally set pool_size to 72 bytes.
    // This simulates the memory pool holding exactly 3 blocks.
    mcr::SlabAllocator allocator(sizeof(TestObj), 72);

    void* ptr1 = allocator.Allocate();
    void* ptr2 = allocator.Allocate();
    void* ptr3 = allocator.Allocate();

    EXPECT_NE(ptr1, nullptr);
    EXPECT_NE(ptr2, nullptr);
    EXPECT_NE(ptr3, nullptr);

    // Ensure all allocated addresses are unique.
    EXPECT_NE(ptr1, ptr2);
    EXPECT_NE(ptr2, ptr3);
    EXPECT_NE(ptr1, ptr3);
}

// [Test 2] Capacity and Boundary
TEST(SlabAllocatorTest, CapacityAndBoundary) {
    // Expect the pool can only fit two 24-byte blocks (48 bytes total).
    // The remaining 12 bytes should be discarded.
    mcr::SlabAllocator allocator(sizeof(TestObj), 60);

    EXPECT_NE(allocator.Allocate(), nullptr);
    EXPECT_NE(allocator.Allocate(), nullptr);

    // Must fail to allocate 3rd block (OOM).
    EXPECT_EQ(allocator.Allocate(), nullptr);
}

// [Test 3] Free and Reuse (LIFO feature)
TEST(SlabAllocatorTest, FreeAndReuse) {
    mcr::SlabAllocator allocator(sizeof(TestObj), 72);

    void* ptr1 = allocator.Allocate();
    void* ptr2 = allocator.Allocate();
    void* ptr3 = allocator.Allocate();

    EXPECT_NE(ptr1, nullptr);
    EXPECT_NE(ptr2, nullptr);
    EXPECT_NE(ptr3, nullptr);

    allocator.Free(ptr2); 
    void* ptr_new = allocator.Allocate();

    // Due to LIFO feature, it should reuse ptr2.
    EXPECT_EQ(ptr_new, ptr2);
}

// [Test 4] Free nullptr (Edge Case)
TEST(SlabAllocatorTest, FreeNullptr) {
    mcr::SlabAllocator allocator(sizeof(TestObj), 72);

    // When free nullptr, it should do nothing.
    EXPECT_NO_FATAL_FAILURE(allocator.Free(nullptr)); 
}

// [Test 5] Memory Alignment (Word Alignment)
TEST(SlabAllocatorTest, MemoryAlignment) {
    mcr::SlabAllocator allocator(sizeof(TestObj), 72);

    void* ptr = allocator.Allocate();
    ASSERT_NE(ptr, nullptr);

    // Reinterpret address to integer for arithmetic (Division).
    uintptr_t address = reinterpret_cast<uintptr_t>(ptr);

    // Check if the address is a multiple of sizeof(void*).
    EXPECT_EQ(address % sizeof(void*), 0);
}

// [Test 6] Stress Test: Exhaust -> Free all -> Exhaust
TEST(SlabAllocatorTest, StressTest) {
    // Simulate a large pool (2400 bytes) for exactly 100 blocks.
    const int count = 100;
    const size_t pool_size = 24 * count;
    mcr::SlabAllocator allocator(sizeof(TestObj), pool_size);

    std::vector<void*> ptrs;

    // 1. Exhaust the memory pool.
    for (int i = 0; i < count; i++) {
        void* ptr = allocator.Allocate();
        EXPECT_NE(ptr, nullptr);
        ptrs.push_back(ptr);
    }
    // Ensure the 101st allocation fails.
    EXPECT_EQ(allocator.Allocate(), nullptr);

    // 2. Free all blocks to the pool.
    for (void* ptr : ptrs) {
        allocator.Free(ptr);
    }
    ptrs.clear();

    // 3. Exhaust again to ensure the free list is completely reset.
    for (int i = 0; i < count; i++) {
        void* ptr = allocator.Allocate();
        EXPECT_NE(ptr, nullptr);
    }
    // Ensure the 101st allocation fails again.
    EXPECT_EQ(allocator.Allocate(), nullptr);
}

// [Test 7] SIMD Alignment (AVX - 32 bytes boundary)
TEST(SlabAllocatorTest, SIMDAlignment32) {
    // Request 32-byte alignment.
    // Pool size: 100 bytes. Should fit exactly 3 blocks (96 bytes).
    mcr::SlabAllocator allocator(sizeof(TestObj), 100, 32);

    void* ptr1 = allocator.Allocate();
    void* ptr2 = allocator.Allocate();

    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);

    uintptr_t addr1 = reinterpret_cast<uintptr_t>(ptr1);
    uintptr_t addr2 = reinterpret_cast<uintptr_t>(ptr2);

    // Verify the starting addresses are multiples of 32.
    EXPECT_EQ(addr1 % 32, 0);
    EXPECT_EQ(addr2 % 32, 0);

    // Verify the distance between contiguous blocks is exactly 32 bytes.
    std::ptrdiff_t distance = std::abs(static_cast<std::ptrdiff_t>(addr2 - addr1));
    EXPECT_EQ(distance, 32);
}

// [Test 8] Cache Line Alignment (To prevent False Sharing)
TEST(SlabAllocatorTest, CacheLineAlignment64) {
    // Request Cache Line size (64-byte) alignment.
    // Pool size: 200 bytes. Should fit exactly 3 blocks (192 bytes).
    mcr::SlabAllocator allocator(sizeof(TestObj), 200, 64);

    void* ptr = allocator.Allocate();
    ASSERT_NE(ptr, nullptr);

    uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
    EXPECT_EQ(addr % 64, 0);
}

// [Test 9] Exception: Invalid Alignment (Not a power of 2)
TEST(SlabAllocatorTest, InvalidAlignmentException) {
    EXPECT_THROW({
        mcr::SlabAllocator allocator(sizeof(TestObj), 100, 17);
    }, std::invalid_argument);
}

// [Test 10] Exception: Insufficient Pool Size for Aligned Block
TEST(SlabAllocatorTest, InsufficientAlignedPoolSizeException) {
    EXPECT_THROW({
        mcr::SlabAllocator allocator(sizeof(TestObj), 60, 64);
    }, std::invalid_argument);
}