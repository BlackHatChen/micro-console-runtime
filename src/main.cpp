#include <iostream>
#include "SlabAllocator.hpp"

// 測試用物件
struct Particle
{
    float x, y, z; // 12 Bytes
};


int main()
{
    std::cout << "=== Micro Console Runtime: Memory System Test ===" << std::endl;

    // 初始化 Slab Allocator
    // 每個區塊大小：sizeof(Particle)
    // 總區塊數量：假設為 5 個
    size_t blockSize = sizeof(Particle);
    size_t blockCount = 5;
    std::cout << "[Init] Creating SlabAllocator with block size = " << blockSize << ", block count = " << blockCount << std::endl;
    SlabAllocator pool(blockSize, blockCount);

    // 測試連續分配區塊
    std::cout << "\n[Test 1] Sequential Allocation..." << std::endl;
    Particle* p1 = static_cast<Particle*>(pool.allocate());
    Particle* p2 = static_cast<Particle*>(pool.allocate());
    Particle* p3 = static_cast<Particle*>(pool.allocate());
    if (p1 && p2 && p3)
    {
        std::cout << " p1 address: " << p1 << std::endl;
        std::cout << " p2 address: " << p2 << std::endl;
        std::cout << " p3 address: " << p3 << std::endl;

        // 驗證記憶體是否連續（應該相差 blockSize 或對齊後的大小）
        long diff = reinterpret_cast<char*>(p2) - reinterpret_cast<char*>(p1);
        std::cout << " Address Gap (p2 - p1) = " << diff << " bytes" << std::endl;
        if (diff >= (long)sizeof(void*))
        {
            std::cout << " -> PASS: Gap accommodates embedded pointer." << std::endl;
        }
        else
        {
            std::cerr << " -> FAIL: Block size too small for embedded pointer!" << std::endl;
        }
    }
    else
    {
        std::cerr << " -> FAIL: Allocation returned nullptr unexpectedly." << std::endl;
    }
    
    // 測試釋放區塊
    // 最新釋放的區塊會變成下一個被分配的區塊 (LIFO)
    std::cout << "\n[Test 2] LIFO Reusability Test..." << std::endl;
    std::cout << " Deallocating p2 (" << p2 << ")..." << std::endl;
    pool.deallocate(p2);
    std::cout << " Allocating new particle p4..." << std::endl;
    Particle* p4 = reinterpret_cast<Particle*>(pool.allocate());
    std::cout << " p4 address: " << p4 << std::endl;
    if (p2 == p4)
    {
        std::cout << " -> PASS: Memory reused successfully (p4 == p2). No fragmentation" << std::endl;
    }
    else
    {
        std::cout << " -> NOTE: Allocator might not be strictly LIFO, or logic error." << std::endl;
    }
    
    // 測試區塊耗盡
    std::cout << "\n[Test 3] Pool Exhaustion Test..." << std::endl;
    // 現階段已使用 p1, p3, p4 共3個，記憶體總共5個區塊，再分配2個即滿
    Particle* p5 = static_cast<Particle*>(pool.allocate());
    Particle* p6 = static_cast<Particle*>(pool.allocate());
    Particle* p7 = static_cast<Particle*>(pool.allocate()); // 這個會分配失敗
    if (p5 && p6 && !p7)
    {
        std::cout << " -> PASS: Pool correctly reported full (returned nullptr)." << std::endl;
    }
    else
    {
        std::cout << " -> FAIL: Pool overflow check failed." << std::endl;
    }
    
    
    return 0;
}
