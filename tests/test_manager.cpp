#include <gtest/gtest.h>
#include "slab_manager.h"
#include <cstdint>

// [Test 1] O(1) Size Class Routing & Distance Verification
TEST(SlabManagerTest, SizeClassRouting) {
    mcr::SlabManager manager;

    // Request 15 bytes, should route to 16-byte pool.
    void* p1_16 = manager.Allocate(15);
    void* p2_16 = manager.Allocate(15);
    ASSERT_NE(p1_16, nullptr);
    ASSERT_NE(p2_16, nullptr);

    // Calculate the distance between 2 consecutive allocations.
    uintptr_t addr1 = reinterpret_cast<uintptr_t>(p1_16);
    uintptr_t addr2 = reinterpret_cast<uintptr_t>(p2_16);
    std::ptrdiff_t distance = std::abs(static_cast<std::ptrdiff_t>(addr1 - addr2));
    EXPECT_EQ(distance, 16); // Proves it came from 16-byte pool.

    // Test 64-byte pool
    void* p1_64 = manager.Allocate(33);
    void* p2_64 = manager.Allocate(33);
    ASSERT_NE(p1_64, nullptr);
    ASSERT_NE(p2_64, nullptr);

    addr1 = reinterpret_cast<uintptr_t>(p1_64);
    addr2 = reinterpret_cast<uintptr_t>(p2_64);
    distance = std::abs(static_cast<std::ptrdiff_t>(addr1 - addr2));
    EXPECT_EQ(distance, 64);
}

// [Test 2] Over-Alignment Routing
TEST(SlabManagerTest, OverAlignmentRouting) {
    mcr::SlabManager manager;

    // Request 16 bytes, but demand 64-byte Cache Line alignment.
    // The manager should std::max(16, 64) and route the request to 64-byte pool.
    void* p1 = manager.Allocate(16, 64);
    void* p2 = manager.Allocate(16, 64);

    ASSERT_NE(p1, nullptr);
    ASSERT_NE(p2, nullptr);

    // Verify the addresses are perfectly aligned to 64 bytes.
    uintptr_t addr1 = reinterpret_cast<uintptr_t>(p1);
    uintptr_t addr2 = reinterpret_cast<uintptr_t>(p2);
    EXPECT_EQ(addr1 % 64, 0);
    EXPECT_EQ(addr2 % 64, 0);

    // Calculate the distance between 2 consecutive allocations to prove p1 came from 64-byte pool.
    std::ptrdiff_t distance = std::abs(static_cast<std::ptrdiff_t>(addr1 - addr2));
    EXPECT_EQ(distance, 64);
}

// [Test 3] Size Deallocation
TEST(SlabManagerTest, SizeDeallocation) {
    mcr::SlabManager manager;

    void* ptr1 = manager.Allocate(40); // It will be routed to 64-byte pool.

    // Free it and request another same size allocation (Route to 64-byte pool again).
    manager.Free(ptr1, 40);
    void* ptr2 = manager.Allocate(40);

    EXPECT_EQ(ptr1, ptr2); // Because of LIFO free list feature, they will use the same address.
}

// [Test 4] Exceeding Maximum Class Size
TEST(SlabManagerTest, ExceedMaxSize) {
    mcr::SlabManager manager;

    void* ptr = manager.Allocate(2048);

    // The max size of class is 1024 bytes. 
    // If the request exceeds the max size, it should return nullptr.
    EXPECT_EQ(ptr, nullptr);
}