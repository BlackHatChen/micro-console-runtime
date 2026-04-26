#include <gtest/gtest.h>
#include "slab_manager.h"
#include <cstddef>
#include <array>
#include <cstdint>
#include <stdexcept>

// ------------------------------------------------------------
// Allocation and routing success path.
// ------------------------------------------------------------

TEST(SlabManagerTest, OverAlignmentContract)
{
    mcr::SlabManager manager;

    // Request 16 bytes, but demand 64-byte alignment.
    // The request should be satisfied by a class that provides 64-byte alignment.
    void *ptr1 = manager.Allocate(16, 64);
    void *ptr2 = manager.Allocate(16, 64);
    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);

    // Verify the addresses are perfectly aligned to 64 bytes.
    uintptr_t addr1 = reinterpret_cast<uintptr_t>(ptr1);
    uintptr_t addr2 = reinterpret_cast<uintptr_t>(ptr2);
    EXPECT_EQ(addr1 % 64, 0);
    EXPECT_EQ(addr2 % 64, 0);
}

TEST(SlabManagerTest, PerClassAlignmentMatrix)
{
    constexpr std::array<std::size_t, 7> kClasses{16, 32, 64, 128, 256, 512, 1024};

    // Each class must align payloads to at least its class size.
    for (std::size_t cls : kClasses)
    {
        mcr::SlabManager manager;
        const std::size_t prev = cls / 2;
        const std::size_t requests[2] = {
            (cls == 16 ? 1 : (prev + 1)), // Left inner point in (prev, cls].
            cls                           // Exact class boundary.
        };

        for (std::size_t request : requests)
        {
            SCOPED_TRACE(testing::Message() << "class = " << cls << ", request size = " << request);

            void *ptr1 = manager.Allocate(request);
            void *ptr2 = manager.Allocate(request);
            ASSERT_NE(ptr1, nullptr);
            ASSERT_NE(ptr2, nullptr);

            // Returned block pointers must be aligned to at least the class minimum.
            uintptr_t addr1 = reinterpret_cast<uintptr_t>(ptr1);
            uintptr_t addr2 = reinterpret_cast<uintptr_t>(ptr2);
            EXPECT_EQ(addr1 % cls, 0);
            EXPECT_EQ(addr2 % cls, 0);
        }
    }
}

// ------------------------------------------------------------
// Deallocation and reuse behavior.
// ------------------------------------------------------------

TEST(SlabManagerTest, OverAlignedFreeRoutingIsSymmetric)
{
    mcr::SlabManager manager;

    // Allocate 2 blocks that must route to the 64-byte class.
    void *ptr1 = manager.Allocate(16, 64);
    void *ptr2 = manager.Allocate(16, 64);
    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);

    // Both returned pointers should satisfy the requested alignment.
    uintptr_t addr1 = reinterpret_cast<uintptr_t>(ptr1);
    uintptr_t addr2 = reinterpret_cast<uintptr_t>(ptr2);
    ASSERT_EQ(addr1 % 64, 0);
    ASSERT_EQ(addr2 % 64, 0);

    // Free the most recent block with the same contract (size, alignment).
    manager.Free(ptr2, 16, 64);

    // Reallocate with the same request.
    // Allocate/Free routing is symmetric and the free list is LIFO,
    // it should return the same address.
    void *ptr3 = manager.Allocate(16, 64);
    ASSERT_NE(ptr3, nullptr);

    EXPECT_EQ(ptr3, ptr2);

    uintptr_t addr3 = reinterpret_cast<uintptr_t>(ptr3);
    EXPECT_EQ(addr3 % 64, 0);
}

TEST(SlabManagerTest, DefaultAlignedDeallocationReuseProxy)
{
    mcr::SlabManager manager;

    // Request 40 bytes, it should route to 64-byte class under the current size-class mapping.
    void *ptr1 = manager.Allocate(40);
    ASSERT_NE(ptr1, nullptr);

    // Free with the same default-aligned contract and allocate again.
    manager.Free(ptr1, 40, sizeof(void *));
    void *ptr2 = manager.Allocate(40);
    ASSERT_NE(ptr2, nullptr);

    // Proxy assertion:
    // Under LIFO free-list behavior, reuse the same address immediately
    // indicates that deallocation routes back to the same size class allocator.
    EXPECT_EQ(ptr1, ptr2);
}

// ------------------------------------------------------------
// Allocation failure and invalid input.
// ------------------------------------------------------------

TEST(SlabManagerTest, AllocationExceedingMaxClassSizeReturnsNullptr)
{
    mcr::SlabManager manager;

    // If the request exceeds the maximum managed class size (1024 bytes), it should return nullptr.
    EXPECT_EQ(manager.Allocate(2048), nullptr);
}

TEST(SlabManagerTest, AllocationWhoseAlignmentExceedsMaxClassSizeReturnsNullptr)
{
    mcr::SlabManager manager;

    // If `max(size, alignment)` exceeds the maximum managed class size (1024 bytes), it should return nullptr.
    EXPECT_EQ(manager.Allocate(16, 2048), nullptr);
}

TEST(SlabManagerTest, ZeroAlignmentThrowsInvalidArgument)
{
    mcr::SlabManager manager;

    EXPECT_THROW({ manager.Allocate(16, 0); }, std::invalid_argument);
}

TEST(SlabManagerTest, NonPowerOfTwoAlignmentThrowsInvalidArgument)
{
    mcr::SlabManager manager;

    EXPECT_THROW({ manager.Allocate(16, 24); }, std::invalid_argument);
}

// ------------------------------------------------------------
// `Free(nullptr)` special-case behavior.
// ------------------------------------------------------------

TEST(SlabManagerTest, OverAlignedFreeNullptrIsNoOp)
{
    mcr::SlabManager manager;

    void *ptr1 = manager.Allocate(16, 64);
    void *ptr2 = manager.Allocate(16, 64);
    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);

    // Create an observable two-node free-list prefix in the routed size class.
    manager.Free(ptr2, 16, 64);
    manager.Free(ptr1, 16, 64);

    // Free(nullptr) must not change the order.
    manager.Free(nullptr, 16, 64);

    void *ptr3 = manager.Allocate(16, 64);
    void *ptr4 = manager.Allocate(16, 64);
    ASSERT_NE(ptr3, nullptr);
    ASSERT_NE(ptr4, nullptr);

    EXPECT_EQ(ptr3, ptr1);
    EXPECT_EQ(ptr4, ptr2);
}

TEST(SlabManagerTest, DefaultAlignedFreeNullptrIsNoOp)
{
    mcr::SlabManager manager;

    void *ptr1 = manager.Allocate(40);
    void *ptr2 = manager.Allocate(40);
    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);

    // Create an observable two-node free-list prefix in the routed size class.
    manager.Free(ptr2, 40, sizeof(void *));
    manager.Free(ptr1, 40, sizeof(void *));

    // Free(nullptr) must not change the order.
    manager.Free(nullptr, 40, sizeof(void *));

    void *ptr3 = manager.Allocate(40);
    void *ptr4 = manager.Allocate(40);
    ASSERT_NE(ptr3, nullptr);
    ASSERT_NE(ptr4, nullptr);

    EXPECT_EQ(ptr3, ptr1);
    EXPECT_EQ(ptr4, ptr2);
}
