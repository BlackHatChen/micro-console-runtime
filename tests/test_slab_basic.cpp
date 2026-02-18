#include <iostream>
#include <cassert>
#include "slab_allocator.h"

struct TestObj {
    int id; // 4 bytes
    float value; // 4 bytes
    float a, b, c; // 3 * 4 bytes = 12 bytes
}; // The size of TestObj is 20 bytes (Without padding).

int main() {
    std::cout << "=== MCR Slab Allocator Test ===" << std::endl;
    std::cout <<"Size of TestObj:" << sizeof(TestObj) << " bytes" << std::endl;

    // Create a slab allocator and memory pool.
    // The block size (20 bytes) will be aligned to 24 bytes (multiple of 8 bytes).
    // With a pool size of 72 bytes, it could be sliced into 3 blocks (72 / 24 = 3).
    mcr::SlabAllocator allocator(sizeof(TestObj), 72);

    // [Test 1] Allocate 3 blocks (Expect to succeed).
    std::cout << "[Test 1] Allocating 3 blocks..." << std::endl;
    void* ptr1 = allocator.Allocate();
    void* ptr2 = allocator.Allocate();
    void* ptr3 = allocator.Allocate();
    std::cout << "Ptr 1:" << ptr1 << std::endl;
    std::cout << "Ptr 2:" << ptr2 << std::endl;
    std::cout << "Ptr 3:" << ptr3 << std::endl;
    assert(ptr1 != nullptr && ptr2 != nullptr && ptr3 != nullptr);
    std::cout << "Test 1 Passed: Successfully allocated 3 blocks." << std::endl;

    // [Test 2] Out Of Memory.
    std::cout << "[Test 2] Allocating 4th block (Expected to OOM)..." << std::endl;
    void* ptr4 = allocator.Allocate();
    if (ptr4 == nullptr) {
        std::cout << "Test 2 Passed: Allocation failed (Due to the memory pool is exhausted)." << std::endl;
    } else {
        std::cerr << "Test 2 Failed: The allocation should be failed but got " << ptr4 << std::endl;
        return -1;
    }
    
    // [Test 3] Free and Reuse (LIFO feature).
    std::cout << "[Test 3] Freeing ptr2 and re-allocating..." << std::endl;
    allocator.Free(ptr2);
    void* ptr_test = allocator.Allocate();
    std::cout << "New ptr:" << ptr_test << std::endl;
    if (ptr2 == ptr_test) {
        std::cout << "Test 3 passed: The new allocator reused the same block as ptr2." << std::endl;
    } else {
        std::cerr << "Test 3 failed: Expected to reuse the same block as ptr2." << std::endl;
        return -1;
    }

    // [Test 4] Free nullptr (Expect to do nothing and not crash).
    std::cout << "[Test 4] Freeing nullptr (Expected to do nothing)..." << std::endl;
    allocator.Free(nullptr);
    std::cout << "Test 4 passed: Freeing nullptr did not cause any issues." << std::endl;

    std::cout << "=== All tests passed successfully! ===" << std::endl;
    return 0;
}