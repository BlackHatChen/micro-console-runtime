#pragma once // 這個檔案只允許被編譯一次
#include <cstddef> // 為了 size_t (保證存下任何大小的記憶體空間，防止溢位) (32 位元為 4 Bytes，64 位元為 8 Bytes)

class SlabAllocator {
public:
    /**
     * 建構子：初始化記憶體池
     * @param blockSize 每個區塊的資料所佔空間大小
     * @param blockCount 預先分配的區塊總數
     * @param alignment 對齊邊界，預設是 8 Bytes，也就是 64 位元系統的指標大小
     * （若要配合 Cache Line 使用者可傳入 64 Bytes）
     */
    SlabAllocator(size_t blockSize, size_t blockCount, size_t alignment = sizeof(void*));

    // 解構子：歸還記憶體
    ~SlabAllocator();

    // 禁止複製操作 (避免重複釋放記憶體區塊)
    SlabAllocator(const SlabAllocator&) = delete;
    SlabAllocator& operator=(const SlabAllocator&) = delete;

    // 分配 & 釋放 (目標時間：O(1))
    void* allocate();
    void deallocate(void* ptr);

private:
    size_t m_blockSize; // 資料的區塊大小
    size_t m_blockCount;
    size_t m_alignment;

    size_t m_objectSize; // 真正的區塊大小（包含 padding）

    void* m_memoryPool; // 指向向 OS 要來的整個記憶體區塊的開頭
    void* m_freeListHead; // 指向下一個可用空間 (實現 O(1) 分配速度的關鍵)
};
