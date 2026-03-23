#ifndef MCR_SLAB_ALLOCATOR_H_

#define MCR_SLAB_ALLOCATOR_H_
#include <cstddef>

namespace mcr {
    /**
     * @brief A memory allocator consisting of "fixed-size" blocks.
     * 
     * Eliminates external fragmentation 
     * (The contiguous free block size insufficient for allocation request even if total block size is sufficient).
     * Since all block sizes are the same, any free block could satisfy an allocation request.
     * 
     * Contract/Notes:
     * 
     * - Alignment floor: the effective alignment is `max(user_alignment, sizeof(void*)`).
     * The pool is acquired with that alignment, and each block size is rounded up to it, so every payload is at least pointer-size aligned.
     * 
     * - Zero metadata: no per-allocation header is written. `Allocate`/`Free` is O(1) by popping/pushing an embedded singly-linked free list node.
     * 
     * - Error behavior: `Free(nullptr)` is a no-op. Non power of 2 alignment throws.
     * The caller must free each block exactly once and only blocks allocated by this allocator (no cross-pool free). 
     * 
     * - Thread-safety: not thread-safe. External synchronization is required if used concurrency.
     * 
     * [Ref] OSTEP Chapter 17 (Free-Space Management) - External Fragmentation, Segregated Lists.
     */
    class SlabAllocator {
    public:
        /**
         * @brief Construct the allocator & memory pool.
         * 
         * By enforcing alignment at the pool initialization stage and preserve O(1) `Allocate`/`Free`.
         * 
         * Unaligned accesses could cause performance penalties. (Like multiple accesses to fetch the whole data.)
         * 
         * - Alignment is rounded up with a platform floor (sizeof(void*)), guaranteeing at least pointer-size alignment for every payload.
         * 
         * - No per-allocation header is written; free list is embedded into blocks.
         * 
         * [Ref] CSAPP Chapter 3.9.3 (Data Alignment)
         * @param block_size The size of each memory block.
         * @param pool_size The total size of the memory pool to request from the OS.
         * @param alignment The memory alignment (Must be a power of 2), defaults to a word size.
         */
        SlabAllocator(std::size_t block_size, std::size_t pool_size, std::size_t alignment = sizeof(void*));

        /**
         * @brief Destruct allocator and free all allocated memory.
         */
        ~SlabAllocator();

        /**
         * @brief Allocate a memory block from the pool.
         * 
         * Get a free block address from the head of the free list in O(1) time.
         * 
         * @return pointer to the allocated memory, or nullptr if the pool is exhausted.
         */
        void* Allocate();

        /**
         * @brief Free an allocated memory block back to the pool.
         * 
         * Push the block back to the head of the free list in O(1) time.
         * 
         * @param ptr Pointer to the block to be freed. If ptr is nullptr, do nothing.
         */
        void Free(void* ptr);

        // ---------------------------------------------------------------
        // Disable copy semantics to prevent Double Free and UB when multiple destructors release the same pool.
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

        /**
         * @brief Pointer to the pool address which requested from OS.
         */
        void* pool_start_;

        /**
         * @brief Store the block which should be allocated first.
         */
        FreeBlock* free_list_head_;
    };
}
// See also: `ADR-0001` (Allocator Contract), `ADR-0003` (Alignment Policy).

#endif