#pragma once // 這個檔案只允許被編譯一次
#include <cstddef> // 為了 size_t (保證存下任何大小的記憶體空間，防止溢位) (32 位元為 4 Bytes，64 位元為 8 Bytes)

class SlabAllocator {
public:
    // 建構子：初始化記憶體池
    SlabAllocator(size_t blockSize, size_t blockCount);

    // 解構子：歸還記憶體
    ~SlabAllocator();

    // 禁止複製操作 (避免重複釋放記憶體區塊)
    SlabAllocator(const SlabAllocator&) = delete;
    SlabAllocator& operator=(const SlabAllocator&) = delete;

    // 分配 & 釋放 (目標時間：O(1))
    void* allocate();
    void deallocate(void* ptr);

private:
    size_t m_blockSize;
    size_t m_blockCount;

    void* m_memoryPool; // 指向向 OS 要來的整個記憶體區塊的開頭
    void* m_freeListHead; // 指向下一個可用空間 (實現 O(1) 分配速度的關鍵)
};
