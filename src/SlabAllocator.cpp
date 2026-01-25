#include "SlabAllocator.hpp"
#include <cstdlib> // malloc, free 等方法
#include <new> // std::bad_alloc
#include <algorithm> // std::max
#include <stdexcept> // std::invalid_argument

// 平台差異處理
#if defined(_WIN32) || defined(_WIN64)
    #include <malloc.h> // Windows 專用的 _aligned_malloc
#else
    #include <stdlib.h> // Linux 專用的 posix_memalign
#endif

// 向 OS 申請一大塊記憶體區塊
SlabAllocator::SlabAllocator(size_t blockSize, size_t blockCount, size_t alignment) : m_blockSize(blockSize), m_blockCount(blockCount), m_alignment(alignment) 
{
    // ※ 參數檢查 1
    // 對齊空間至少要是指標的空間大小
    if (m_alignment < sizeof(void*))
    {
        m_alignment = sizeof(void*);
    }

    // ※ 參數檢查 2
    // 確保對齊空間是2的次方
    // 若 x 是2的次方，x & (x-1) 必定為 0
    if ((m_alignment & (m_alignment - 1)) != 0)
    {
        throw std::invalid_argument("Alignment must be a power of 2");
    }
    
    // ※ 區塊大小計算
    // 必須是使用者的需求大小（且至少要能塞下嵌入式指標）
    size_t requiredSize = std::max(m_blockSize, sizeof(void*));

    // 區塊大小的對齊計算
    // m_objectSize = (requiredSize + m_alignment - 1) / m_alignment * m_alignment; 數學計算法
    m_objectSize = (requiredSize + m_alignment - 1) & ~(m_alignment - 1); // 位元操作法 (注：僅生效於對齊值為2的次方)
    
    // 對齊後的總記憶體空間大小
    size_t totalSize = m_objectSize * m_blockCount;

    // 向 OS 申請記憶體區塊
    #if defined(_WIN32) || defined(_WIN64)
        m_memoryPool = _aligned_malloc(totalSize, m_alignment);
        if (!m_memoryPool)
        {
            throw std::bad_alloc();
        }
    #else
        // 將分配好的區塊位址傳給 m_memoryPool，若成功則回傳 0
        if (posix_memalign(&m_memoryPool, m_alignment, totalSize) != 0)
        {
            throw std::bad_alloc(); // 失敗則跳出分配錯誤訊息
        }
    #endif

    // 建立 Free List (直接把下一個空閒的區塊位址寫入當前的區塊，不使用 Linked List 以節省空間)
    m_freeListHead = m_memoryPool; // 初始化 Free List 的 Head 指向 Memory Pool 的第一個區塊
    char* current = static_cast<char*>(m_memoryPool); // 轉換成 char 方便計算位址 (1 Byte)，因為 void pointer 本身不能拿來計算

    for (size_t i = 0; i < m_blockCount - 1; ++i)
    {
        char* next = current + m_objectSize; // 計算下一個區塊的位址
        *reinterpret_cast<void**>(current) = next; // 將下一個區塊的位址寫入當前的區塊空間
        current = next; // 迭代到下一個區塊
    }
    
    *reinterpret_cast<void**>(current) = nullptr; // 最後一個區塊存 nullptr (代表沒有下一個可用區塊)
}

SlabAllocator::~SlabAllocator()
{
    #if defined(_WIN32) || defined(_WIN64)
        _aligned_free(m_memoryPool);
    #else
        free(m_memoryPool);
    #endif
}

void* SlabAllocator::allocate()
{
    // 空間已滿，沒有可用區塊
    if (m_freeListHead == nullptr)
    {
        return nullptr;
    }
    
    void* block = m_freeListHead; // 拿取 Free List 中 Head 指向的區塊，也就是目前欲分配區塊的位址
    m_freeListHead = *reinterpret_cast<void**>(block); // 從欲分配區塊中拿取並更新 Head 指向的下一個可使用區塊位址
    return block;
}

void SlabAllocator::deallocate(void* ptr)
{
    if (ptr == nullptr)
    {
        return;
    }
    
    *reinterpret_cast<void**>(ptr) = m_freeListHead; // 把目前 Head 指向的下一個可使用區塊位址，存入欲釋放的區塊中
    m_freeListHead = ptr; // 並更新 Head 指向的下一個可使用區塊位址，為剛釋放的區塊空間位址
}