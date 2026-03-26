#include <gtest/gtest.h>
#include "slab_manager.h"
#include <cstddef>
#include <array>
#include <cstdlib>

// [Test 01] O(1) Size Class Routing & Distance Verification
TEST(SlabManagerTest, SizeClassRouting)
{
    mcr::SlabManager manager;

    // Request 15 bytes, should route to 16-byte pool.
    void *ptr1_16 = manager.Allocate(15);
    void *ptr2_16 = manager.Allocate(15);
    ASSERT_NE(ptr1_16, nullptr);
    ASSERT_NE(ptr2_16, nullptr);

    // Calculate the distance between 2 consecutive allocations.
    uintptr_t addr1 = reinterpret_cast<uintptr_t>(ptr1_16);
    uintptr_t addr2 = reinterpret_cast<uintptr_t>(ptr2_16);
    std::ptrdiff_t distance = std::abs(static_cast<std::ptrdiff_t>(addr1 - addr2));
    EXPECT_EQ(distance, 16); // Proves it came from 16-byte pool.

    // Test 64-byte pool
    void *ptr1_64 = manager.Allocate(33);
    void *ptr2_64 = manager.Allocate(33);
    ASSERT_NE(ptr1_64, nullptr);
    ASSERT_NE(ptr2_64, nullptr);

    addr1 = reinterpret_cast<uintptr_t>(ptr1_64);
    addr2 = reinterpret_cast<uintptr_t>(ptr2_64);
    distance = std::abs(static_cast<std::ptrdiff_t>(addr1 - addr2));
    EXPECT_EQ(distance, 64);
}

// [Test 02] Over-Alignment Routing
TEST(SlabManagerTest, OverAlignmentRouting)
{
    mcr::SlabManager manager;

    // Request 16 bytes, but demand 64-byte Cache Line alignment.
    // The manager should std::max(16, 64) and route the request to 64-byte pool.
    void *ptr1 = manager.Allocate(16, 64);
    void *ptr2 = manager.Allocate(16, 64);

    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);

    // Verify the addresses are perfectly aligned to 64 bytes.
    uintptr_t addr1 = reinterpret_cast<uintptr_t>(ptr1);
    uintptr_t addr2 = reinterpret_cast<uintptr_t>(ptr2);
    EXPECT_EQ(addr1 % 64, 0);
    EXPECT_EQ(addr2 % 64, 0);

    // Calculate the distance between 2 consecutive allocations to prove p1 came from 64-byte pool.
    std::ptrdiff_t distance = std::abs(static_cast<std::ptrdiff_t>(addr1 - addr2));
    EXPECT_EQ(distance, 64);
}

// [Test 02b] Over-Alignment Deallocation Symmetry Regression
TEST(SlabManagerTest, OverAlignedFreeRoutingIsSymmetric)
{
    mcr::SlabManager manager;

    // Allocate 2 blocks that must route to the 64-byte class.
    void *ptr1 = manager.Allocate(16, 64);
    void *ptr2 = manager.Allocate(16, 64);

    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);

    // Both returned pointers should statisfy the requested alignment.
    uintptr_t addr1 = reinterpret_cast<uintptr_t>(ptr1);
    uintptr_t addr2 = reinterpret_cast<uintptr_t>(ptr2);
    EXPECT_EQ(addr1 % 64, 0);
    EXPECT_EQ(addr2 % 64, 0);

    // Free the most recent block with the same contract (size, alignment).
    manager.Free(ptr2, 16, 64);

    // Reallocate with the same request. Allocate/Free routing is symmetric and the free list is LIFO, it should return the same address.
    void *ptr3 = manager.Allocate(16, 64);
    ASSERT_NE(ptr3, nullptr);

    EXPECT_EQ(ptr3, ptr2);

    uintptr_t addr3 = reinterpret_cast<uintptr_t>(ptr3);
    EXPECT_EQ(addr3 % 64, 0);
}

// [Test 03] Size Deallocation
TEST(SlabManagerTest, SizeDeallocation)
{
    mcr::SlabManager manager;

    // It will be routed to 64-byte pool.
    void *ptr1 = manager.Allocate(40);

    // Free it and request another same size allocation (Route to 64-byte pool again).
    manager.Free(ptr1, 40, sizeof(void *));
    void *ptr2 = manager.Allocate(40);

    // Because of LIFO free list feature, they will use the same address.
    EXPECT_EQ(ptr1, ptr2);
}

// [Test 04] Exceeding Maximum Class Size
TEST(SlabManagerTest, ExceedMaxSize)
{
    mcr::SlabManager manager;

    void *ptr = manager.Allocate(2048);

    // The max size of class is 1024 bytes.
    // If the request exceeds the max size, it should return nullptr.
    EXPECT_EQ(ptr, nullptr);
}

// [Test 05] Per-Class Alignment
TEST(SlabManagerTest, PerClassAlignmentMatrix)
{
    constexpr std::array<size_t, 7> kClasses{16, 32, 64, 128, 256, 512, 1024};

    // Each class must align payloads to at least its class size.
    for (size_t cls : kClasses)
    {
        mcr::SlabManager manager;
        const size_t prev = cls / 2;
        const size_t requests[2] = {
            (cls == 16 ? 1 : (prev + 1)), // Left inner point in (prev, cls].
            cls                           // Exact class boundary.
        };

        for (size_t request : requests)
        {
            SCOPED_TRACE(testing::Message() << "class = " << cls << ", request size = " << request);

            void *p1 = manager.Allocate(request);
            void *p2 = manager.Allocate(request);
            ASSERT_NE(p1, nullptr);
            ASSERT_NE(p2, nullptr);

            uintptr_t addr1 = reinterpret_cast<uintptr_t>(p1);
            uintptr_t addr2 = reinterpret_cast<uintptr_t>(p2);
            EXPECT_EQ(addr1 % cls, 0); // Payload must be aligned to at least the class minimum.

            std::ptrdiff_t dist = std::abs(static_cast<std::ptrdiff_t>(addr2 - addr1));
            EXPECT_EQ(dist, static_cast<std::ptrdiff_t>(cls)); // Inter-allocation stride equals to the class size.
        }
    }
}

// [Test 06] Exception: Zero Alignment
TEST(SlabManagerTest, ZeroAlignmentException)
{
    mcr::SlabManager manager;

    EXPECT_THROW({ manager.Allocate(16, 0); }, std::invalid_argument);
}

// [Test 07] Exception: Non-Power-Of-Two Alignment
TEST(SlabManagerTest, NonPowerOfTwoAlignmentException)
{
    mcr::SlabManager manager;

    EXPECT_THROW({ manager.Allocate(16, 24); }, std::invalid_argument);
}