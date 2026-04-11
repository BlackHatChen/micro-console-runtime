#include <gtest/gtest.h>
#include "slab_allocator.h"
#include <cstddef>
#include <vector>
#include <cstdlib>

// Test object used for block-size and alignment-related allocator tests.
struct TestObj
{
    int id;
    float value;
    float a, b, c;
};

namespace
{
    constexpr std::size_t MaxSize(std::size_t a, std::size_t b)
    {
        return (a > b) ? a : b;
    }

    constexpr std::size_t AlignUp(std::size_t value, std::size_t alignment)
    {
        return (value + alignment - 1) & ~(alignment - 1);
    }

    constexpr std::size_t EffectiveAlignment(std::size_t requested_alignment = sizeof(void *))
    {
        return MaxSize(requested_alignment, sizeof(void *));
    }

    constexpr std::size_t EffectiveBlockSize(std::size_t raw_block_size, std::size_t requested_alignment = sizeof(void *))
    {
        const std::size_t effective_alignment = EffectiveAlignment(requested_alignment);
        const std::size_t floored_block_size = MaxSize(raw_block_size, sizeof(void *));
        return AlignUp(floored_block_size, effective_alignment);
    }
}

// [Test 01] Basic Allocation
TEST(SlabAllocatorTest, BasicAllocation)
{
    const std::size_t block_size = EffectiveBlockSize(sizeof(TestObj));
    const std::size_t pool_size = block_size * 3;
    mcr::SlabAllocator allocator(sizeof(TestObj), pool_size);

    void *ptr1 = allocator.Allocate();
    void *ptr2 = allocator.Allocate();
    void *ptr3 = allocator.Allocate();

    EXPECT_NE(ptr1, nullptr);
    EXPECT_NE(ptr2, nullptr);
    EXPECT_NE(ptr3, nullptr);

    // Ensure all allocated addresses are unique.
    EXPECT_NE(ptr1, ptr2);
    EXPECT_NE(ptr2, ptr3);
    EXPECT_NE(ptr1, ptr3);
}

// [Test 02] Capacity and Boundary
TEST(SlabAllocatorTest, CapacityAndBoundary)
{
    const std::size_t block_size = EffectiveBlockSize(sizeof(TestObj));
    const std::size_t pool_size = block_size * 2 + block_size / 2;
    mcr::SlabAllocator allocator(sizeof(TestObj), pool_size);

    EXPECT_NE(allocator.Allocate(), nullptr);
    EXPECT_NE(allocator.Allocate(), nullptr);

    // Must fail to allocate 3rd block (OOM).
    EXPECT_EQ(allocator.Allocate(), nullptr);
}

// [Test 03] Free Reuses Most Recently Freed Block (LIFO feature)
TEST(SlabAllocatorTest, FreeReusesMostRecentlyFreedBlock)
{
    const std::size_t block_size = EffectiveBlockSize(sizeof(TestObj));
    const std::size_t pool_size = block_size * 3;
    mcr::SlabAllocator allocator(sizeof(TestObj), pool_size);

    void *ptr1 = allocator.Allocate();
    void *ptr2 = allocator.Allocate();
    void *ptr3 = allocator.Allocate();

    EXPECT_NE(ptr1, nullptr);
    EXPECT_NE(ptr2, nullptr);
    EXPECT_NE(ptr3, nullptr);

    allocator.Free(ptr2);
    void *ptr_new = allocator.Allocate();

    // Due to LIFO feature, it should reuse ptr2.
    EXPECT_EQ(ptr_new, ptr2);
}

TEST(SlabAllocatorTest, FreeValidPtrRestoresOneAllocationAfterExhaustion)
{
    const std::size_t block_size = EffectiveBlockSize(sizeof(TestObj));
    const int block_count = 3;
    const std::size_t pool_size = block_size * block_count;
    mcr::SlabAllocator allocator(sizeof(TestObj), pool_size);

    void *ptr1 = allocator.Allocate();
    void *ptr2 = allocator.Allocate();
    void *ptr3 = allocator.Allocate();

    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);
    ASSERT_NE(ptr3, nullptr);

    ASSERT_EQ(allocator.Allocate(), nullptr);   // exhausted

    allocator.Free(ptr2);

    void *recovered = allocator.Allocate();
    ASSERT_NE(recovered, nullptr);              // restored once

    EXPECT_EQ(allocator.Allocate(), nullptr);   // exhausted again
}

// [Test 04] Free nullptr (Edge Case)
TEST(SlabAllocatorTest, FreeNullptr)
{
    const std::size_t block_size = EffectiveBlockSize(sizeof(TestObj));
    mcr::SlabAllocator allocator(sizeof(TestObj), block_size);

    // When free nullptr, it should do nothing.
    EXPECT_NO_FATAL_FAILURE(allocator.Free(nullptr));
}

TEST(SlabAllocatorTest, DefaultAllocationIsPointerAligned)
{
    const std::size_t block_size = EffectiveBlockSize(sizeof(TestObj));
    mcr::SlabAllocator allocator(sizeof(TestObj), block_size);

    void *ptr = allocator.Allocate();
    ASSERT_NE(ptr, nullptr);

    // Reinterpret address to integer for arithmetic (Division).
    uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);

    // Check if the address is a multiple of sizeof(void*).
    EXPECT_EQ(addr % sizeof(void *), 0);
}

TEST(SlabAllocatorTest, AlignmentOverWordSizeRetained)
{
    for (size_t count : {1, 2, 4})
    {
        mcr::SlabAllocator allocator(sizeof(TestObj), 72, count * sizeof(void *));
        void *ptr = allocator.Allocate();
        ASSERT_NE(ptr, nullptr);
        uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
        EXPECT_EQ(addr % (count * sizeof(void *)), 0);
    }
}

// [Test 06] Stress Test: Exhaust -> Free all -> Exhaust
TEST(SlabAllocatorTest, StressTest)
{
    // Simulate a large pool for exactly `count` effective blocks.
    const int count = 100;
    const std::size_t block_size = EffectiveBlockSize(sizeof(TestObj));
    const std::size_t pool_size = block_size * count;
    mcr::SlabAllocator allocator(sizeof(TestObj), pool_size);

    std::vector<void *> ptrs;

    // 1. Exhaust the memory pool.
    for (int i = 0; i < count; i++)
    {
        void *ptr = allocator.Allocate();
        EXPECT_NE(ptr, nullptr);
        ptrs.push_back(ptr);
    }
    // Ensure the 101st allocation fails.
    EXPECT_EQ(allocator.Allocate(), nullptr);

    // 2. Free all blocks to the pool.
    for (void *ptr : ptrs)
    {
        allocator.Free(ptr);
    }
    ptrs.clear();

    // 3. Exhaust again to ensure the free list is completely reset.
    for (int i = 0; i < count; i++)
    {
        void *ptr = allocator.Allocate();
        EXPECT_NE(ptr, nullptr);
    }
    // Ensure the 101st allocation fails again.
    EXPECT_EQ(allocator.Allocate(), nullptr);
}

// [Test 07] SIMD Alignment (AVX - 32 bytes boundary)
TEST(SlabAllocatorTest, SIMDAlignment32)
{
    // Request 32-byte alignment.
    // Pool size: 100 bytes. Should fit exactly 3 blocks (96 bytes).
    mcr::SlabAllocator allocator(sizeof(TestObj), 100, 32);

    void *ptr1 = allocator.Allocate();
    void *ptr2 = allocator.Allocate();

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

// [Test 08] Cache Line Alignment (To prevent False Sharing)
TEST(SlabAllocatorTest, CacheLineAlignment64)
{
    // Request Cache Line size (64-byte) alignment.
    // Pool size: 200 bytes. Should fit exactly 3 blocks (192 bytes).
    mcr::SlabAllocator allocator(sizeof(TestObj), 200, 64);

    void *ptr = allocator.Allocate();
    ASSERT_NE(ptr, nullptr);

    uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
    EXPECT_EQ(addr % 64, 0);
}

// [Test 09] Exception: Invalid Alignment (Not a power of 2)
TEST(SlabAllocatorTest, InvalidAlignmentException)
{
    EXPECT_THROW({ mcr::SlabAllocator allocator(sizeof(TestObj), 100, 17); }, std::invalid_argument);
}

// [Test 10] Exception: Insufficient Pool Size for Aligned Block
TEST(SlabAllocatorTest, InsufficientAlignedPoolSizeException)
{
    EXPECT_THROW({ mcr::SlabAllocator allocator(sizeof(TestObj), 60, 64); }, std::invalid_argument);
}

// [Test 11] Alignment Floor (alignment < sizeof(void*) is floored to pointer-size)
TEST(SlabAllocatorTest, AlignmentFloorAtLeastPointerSize)
{
    // Try very small alignment, the effective alignment must be >= sizeof(void*).
    for (size_t alignment : {1, 2, 4})
    {
        mcr::SlabAllocator allocator(sizeof(TestObj), 72, alignment);
        void *ptr = allocator.Allocate();
        ASSERT_NE(ptr, nullptr);
        uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
        EXPECT_EQ(addr % sizeof(void *), 0);
    }
}

// [Test 12] Exception: Zero Alignment
TEST(SlabAllocatorTest, ZeroAlignmentException)
{
    EXPECT_THROW({ mcr::SlabAllocator allocator(sizeof(TestObj), 100, 0); }, std::invalid_argument);
}

TEST(SlabAllocatorTest, FreeNullptrKeepsAllocatorExhausted)
{
    const std::size_t block_size = EffectiveBlockSize(sizeof(TestObj));
    const int block_count = 3;
    const std::size_t pool_size = block_size * block_count;
    mcr::SlabAllocator allocator(sizeof(TestObj), pool_size);

    for (int i = 0; i < block_count; i++)
    {
        void *ptr = allocator.Allocate();
        EXPECT_NE(ptr, nullptr);
    }

    EXPECT_EQ(allocator.Allocate(), nullptr);

    allocator.Free(nullptr);

    EXPECT_EQ(allocator.Allocate(), nullptr);
}