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
         * @brief Construct the allocator with a fixed-size blocks and total pool size.
         */
        SlabAllocator(std::size_t block_size, std::size_t pool_size);

        /**
         * @brief Destroy the allocator and free all allocated memory.
         */
        ~SlabAllocator();

        /**
         * @brief Allocate a single fixed-size memory block from the pool.
         * 
         * Guarantees O(1) time complexity by popping a free block from the head of the free list.
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
        void* pool_start_;
        FreeBlock* free_list_head_;
    };
}

#endif