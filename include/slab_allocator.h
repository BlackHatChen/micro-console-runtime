#ifndef MCR_SLAB_ALLOCATOR_H_ // Avoid multiple definitions

#define MCR_SLAB_ALLOCATOR_H_
#include <cstddef> // for std::size_t

namespace mcr {
    /**
     * @brief A memory allocator consisting of "fixed-size" blocks for embedded systems.
     * 
     * Eliminates external fragmentation (The contiguous free block size insufficient for allocation request even if total block size is sufficient).
     * Since all block sizes are the same, any free block could satisfy an allocation request.
     * 
     * [Ref] OSTEP Chapter 17 (Free-Space Management) - External Fragmentation, Segregated Lists.
     */
    class SlabAllocator {
    public:
        /**
         * @brief Construct the allocator with a fixed-size blocks, total pool size, and specific alignment.
         * 
         * By enforcing alignment at the pool initialization stage, 
         * we preserve O(1) allocate/free and eliminate the need for per-block metadata (header).
         * 
         * [Ref] CSAPP Chapter 3.9.3 (Data Alignment)
         * @param block_size The requested minimum size of each memory block.
         * @param pool_size The total size of the memory pool to request from the OS.
         * @param alignment The memory alignment (Must be a power of 2), defaults to a word size.
         */
        SlabAllocator(std::size_t block_size, std::size_t pool_size, std::size_t alignment = sizeof(void*));

        /**
         * @brief Destroy the allocator and free all allocated memory.
         */
        ~SlabAllocator();

        /**
         * @brief Allocate a single fixed-size memory block from the pool.
         * 
         * Guarantees O(1) time complexity by popping a free block from the head of the free list.
         * 
         * The returned address is aligned to the `alignment` specified in the constructor.
         * 
         * @return void* pointer to the allocated memory, or nullptr if the pool is exhausted.
         */
        void* Allocate();

        /**
         * @brief Free a previously allocated memory block back to the pool.
         * 
         * Guarantees O(1) time complexity by pushing the block back to the head of the free list.
         * 
         * @param ptr Pointer to the memory block to be freed. If it points to nullptr, then does nothing.
         */
        void Free(void* ptr);

        // ---------------------------------------------------------------
        // Disable copy semantics to prevent Double Free and undefined behavior when multiple destructors release the same pool.
        // [Ref] Effective C++ Item 14 (Think carefully about copying behavior in resource-managing classes) - Prohibit copying RAII objects.
        SlabAllocator(const SlabAllocator&) = delete;
        SlabAllocator& operator=(const SlabAllocator&) = delete;
        // ---------------------------------------------------------------

    private:
        /**
         * @brief Embedded free list node.
         * 
         * [Ref 1] OSTEP Chapter 17.2 (Low-level Mechanisms) - Embedding A Free List.
         * 
         * [Ref 2] CSAPP Chapter 9.9.13 (Explicit Free Lists) - LIFO ordering, first-fit placement.
         */
        struct FreeBlock {
            FreeBlock* next;
        };
        
        std::size_t block_size_;
        std::size_t pool_size_;
        std::size_t alignment_;
        void* pool_start_;
        FreeBlock* free_list_head_;
    };
}

#endif