#include <gtest/gtest.h>
#include "slab_allocator.h"
#include <cstddef>
#include <vector>
#include <cstdint>
#include <stdexcept>

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

// ------------------------------------------------------------
// Core allocation and capacity behavior.
// ------------------------------------------------------------

TEST(SlabAllocatorTest, AllocateReturnsDistinctBlocks)
{
    // Compute the actual allocator pool size.
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

    // The returned block pointers should be distinct.
    EXPECT_NE(ptr1, ptr2);
    EXPECT_NE(ptr2, ptr3);
    EXPECT_NE(ptr1, ptr3);
}

TEST(SlabAllocatorTest, PoolRemainderDoesNotCreateExtraBlock)
{
    // Compute the actual allocator pool size.
    const std::size_t block_size = EffectiveBlockSize(sizeof(TestObj));
    const std::size_t pool_size = block_size * 2 + block_size / 2;

    mcr::SlabAllocator allocator(sizeof(TestObj), pool_size);

    ASSERT_NE(allocator.Allocate(), nullptr);
    ASSERT_NE(allocator.Allocate(), nullptr);

    EXPECT_EQ(allocator.Allocate(), nullptr); // The 3rd allocation should fail because the remainder does not form another whole block.
}

TEST(SlabAllocatorTest, FreeValidPtrRestoresOneAllocationAfterExhaustion)
{
    // Compute the actual allocator pool size.
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

    ASSERT_EQ(allocator.Allocate(), nullptr); // exhausted

    allocator.Free(ptr2);
    void *recovered = allocator.Allocate();
    ASSERT_NE(recovered, nullptr); // restored once

    EXPECT_EQ(allocator.Allocate(), nullptr); // exhausted again
}

TEST(SlabAllocatorTest, FreeingAllBlocksRestoresFullCapacity)
{
    // Compute the actual allocator pool size.
    const std::size_t block_size = EffectiveBlockSize(sizeof(TestObj));
    const int block_count = 100; // Simulate a large pool with exactly `block_count` effective blocks.
    const std::size_t pool_size = block_size * block_count;

    mcr::SlabAllocator allocator(sizeof(TestObj), pool_size);

    std::vector<void *> ptrs;

    // Exhaust the memory pool.
    for (int i = 0; i < block_count; i++)
    {
        void *ptr = allocator.Allocate();
        ASSERT_NE(ptr, nullptr);
        ptrs.push_back(ptr);
    }

    ASSERT_EQ(allocator.Allocate(), nullptr); // Ensure the 101st allocation fails.

    // Return all blocks to the pool.
    for (void *ptr : ptrs)
    {
        allocator.Free(ptr);
    }
    ptrs.clear();

    // Exhaust again to confirm full capacity is restored.
    for (int i = 0; i < block_count; i++)
    {
        void *ptr = allocator.Allocate();
        ASSERT_NE(ptr, nullptr);
    }

    EXPECT_EQ(allocator.Allocate(), nullptr); // Ensure the 101st allocation fails again.
}

// ------------------------------------------------------------
// `Free(nullptr)` special-case behavior.
// ------------------------------------------------------------

TEST(SlabAllocatorTest, FreeNullptrKeepsAllocatorExhausted)
{
    // Compute the actual allocator pool size.
    const std::size_t block_size = EffectiveBlockSize(sizeof(TestObj));
    const int block_count = 3;
    const std::size_t pool_size = block_size * block_count;

    mcr::SlabAllocator allocator(sizeof(TestObj), pool_size);

    for (int i = 0; i < block_count; i++)
    {
        void *ptr = allocator.Allocate();
        ASSERT_NE(ptr, nullptr);
    }

    ASSERT_EQ(allocator.Allocate(), nullptr); // exhausted

    allocator.Free(nullptr);

    EXPECT_EQ(allocator.Allocate(), nullptr); // keeps exhausted
}

TEST(SlabAllocatorTest, FreeNullptrDoesNotInterfereWithSubsequentRestore)
{
    // Compute the actual allocator pool size.
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

    ASSERT_EQ(allocator.Allocate(), nullptr); // exhausted

    // Create an observable two-node free-list prefix.
    allocator.Free(ptr2);
    allocator.Free(ptr1);

    // Free(nullptr) must not change the order.
    allocator.Free(nullptr);

    void *recovered1 = allocator.Allocate();
    void *recovered2 = allocator.Allocate();
    ASSERT_NE(recovered1, nullptr);
    ASSERT_NE(recovered2, nullptr);

    EXPECT_EQ(recovered1, ptr1);
    EXPECT_EQ(recovered2, ptr2);

    EXPECT_EQ(allocator.Allocate(), nullptr); // exhausted again
}

// ------------------------------------------------------------
// Implementation evidence for free-list reuse behavior.
// ------------------------------------------------------------

TEST(SlabAllocatorTest, FreeReusesMostRecentlyFreedBlock)
{
    // Compute the actual allocator pool size.
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

    allocator.Free(ptr2);
    void *ptr_new = allocator.Allocate();
    EXPECT_EQ(ptr_new, ptr2); // Due to LIFO feature, it should reuse ptr2.
}

// ------------------------------------------------------------
// Alignment and block-sizing behavior.
// ------------------------------------------------------------

TEST(SlabAllocatorTest, DefaultAllocationIsPointerAligned)
{
    const std::size_t block_size = EffectiveBlockSize(sizeof(TestObj));

    mcr::SlabAllocator allocator(sizeof(TestObj), block_size);

    void *ptr = allocator.Allocate();
    ASSERT_NE(ptr, nullptr);

    std::uintptr_t addr = reinterpret_cast<std::uintptr_t>(ptr); // Reinterpret address to integer for arithmetic (Division).
    EXPECT_EQ(addr % sizeof(void *), 0);               // Check if the address is a multiple of sizeof(void*).
}

TEST(SlabAllocatorTest, AlignmentFloorAtLeastPointerSize)
{
    // Try very small alignment, the effective alignment must be >= sizeof(void*).
    for (std::size_t alignment : {1, 2, 4})
    {
        const std::size_t pool_size = EffectiveBlockSize(sizeof(TestObj), alignment);

        mcr::SlabAllocator allocator(sizeof(TestObj), pool_size, alignment);

        void *ptr = allocator.Allocate();
        ASSERT_NE(ptr, nullptr);

        std::uintptr_t addr = reinterpret_cast<std::uintptr_t>(ptr);
        EXPECT_EQ(addr % sizeof(void *), 0); // Check if the alignment is floored to a pointer size.
    }
}

TEST(SlabAllocatorTest, RequestedAlignmentAboveWordSizeIsPreserved)
{
    for (std::size_t count : {2, 4, 8})
    {
        const std::size_t alignment = count * sizeof(void *);
        const std::size_t pool_size = EffectiveBlockSize(sizeof(TestObj), alignment);

        mcr::SlabAllocator allocator(sizeof(TestObj), pool_size, alignment);

        void *ptr = allocator.Allocate();
        ASSERT_NE(ptr, nullptr);

        std::uintptr_t addr = reinterpret_cast<std::uintptr_t>(ptr);
        EXPECT_EQ(addr % alignment, 0); // Check if the alignment is preserved.
    }
}

TEST(SlabAllocatorTest, BlockSizeRoundsUpToEffectiveAlignment)
{
    // Compute the essential allocator input values.
    const std::size_t alignment = 2 * sizeof(void *);
    const std::size_t raw_block_size = alignment - sizeof(void *);
    const std::size_t pool_size = alignment * 3;
    const int expected_block_count = 3;

    mcr::SlabAllocator allocator(raw_block_size, pool_size, alignment);

    // The requested object size is not a multiple of alignment,
    // so the allocator should round each block size up to alignment before cutting the pool.
    // With pool_size = alignment * 3, only 3 blocks should be allocatable before exhaustion.
    for (int i = 0; i < expected_block_count; i++)
    {
        void *ptr = allocator.Allocate();
        ASSERT_NE(ptr, nullptr);
    }

    // Without round-up, the raw size would allow more allocations than 3.
    // Exhaustion on the 4th call supports that the pool was cut using the rounded block size instead.
    EXPECT_EQ(allocator.Allocate(), nullptr);
}

// ------------------------------------------------------------
// Constructor failure paths.
// ------------------------------------------------------------

TEST(SlabAllocatorTest, NonPowerOfTwoAlignmentThrowsInvalidArgument)
{
    const std::size_t valid_pool_size = EffectiveBlockSize(sizeof(TestObj)) * 2; // sufficient pool size

    EXPECT_THROW({ mcr::SlabAllocator allocator(sizeof(TestObj), valid_pool_size, 17); }, std::invalid_argument);
}

TEST(SlabAllocatorTest, ZeroAlignmentThrowsInvalidArgument)
{
    const std::size_t valid_pool_size = EffectiveBlockSize(sizeof(TestObj)) * 2; // sufficient pool size

    EXPECT_THROW({ mcr::SlabAllocator allocator(sizeof(TestObj), valid_pool_size, 0); }, std::invalid_argument);
}

TEST(SlabAllocatorTest, InsufficientAlignedPoolSizeThrowsInvalidArgument)
{
    const std::size_t alignment = 64;
    const std::size_t effective_block = EffectiveBlockSize(sizeof(TestObj), alignment);
    const std::size_t insufficient_pool_size = effective_block - sizeof(void *);

    EXPECT_THROW({ mcr::SlabAllocator allocator(sizeof(TestObj), insufficient_pool_size, alignment); }, std::invalid_argument);
}