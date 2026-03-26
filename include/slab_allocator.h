#ifndef MCR_SLAB_ALLOCATOR_H_

#define MCR_SLAB_ALLOCATOR_H_
#include <cstddef>

namespace mcr
{
    /**
     * @brief A memory allocator consisting of "fixed-size" blocks.
     *
     * Eliminates external fragmentation (The contiguous free block size insufficient for allocation request even if total block size is sufficient).
     * Since all block sizes are the same, any free block could satisfy an allocation request.
     *
     * Contract/Notes:
     *
     * 1. Alignment floor: the effective alignment is `max(user_alignment, sizeof(void*))`.
     * The pool is acquired with that alignment, and each block size is rounded up to it, so every returned block pointer is at least pointer-size aligned.
     *
     * 2. Zero metadata: no per-allocation header is written. `Allocate`/`Free` is O(1) by popping/pushing an embedded singly-linked free list node.
     *
     * 3. Error behavior:
     *
     * - Construction throws `std::invalid_argument` if alignment is zero, not a power of 2, or if the pool cannot hold at least one effective block.
     *
     * - Construction throws `std::bad_alloc` if the OS memory request fails.
     *
     * - `Free(nullptr)` is a no-op.
     *
     * - Passing a non-owned pointer, a non-block pointer, or double-freeing a block is a contract violation (undefined behavior).
     *
     * 4. Thread-safety: not thread-safe. External synchronization is required for any concurrent access.
     *
     * [Ref] OSTEP Chapter 17 (Free-Space Management) - External Fragmentation, Segregated Lists.
     */
    class SlabAllocator
    {
    public:
        /**
         * @brief Construct the allocator & memory pool.
         *
         * Enforces alignment at pool initialization time while preserving O(1) `Allocate`/`Free`.
         *
         * Unaligned accesses could cause performance penalties. (Like multiple accesses to fetch the whole data.)
         *
         * - The effective alignment is `max(requested_alignment, sizeof(void*))`. Block size is then rounded up to that alignment.
         *
         * - No per-allocation header is written; free list is embedded into blocks.
         *
         * [Ref] CSAPP Chapter 3.9.3 (Data Alignment)
         * @param block_size The requested payload size of each block.
         * @param pool_size The total pool size requested from the OS.
         * @param alignment The requested alignment. Must be non-zero and a power of 2. The effective alignment is floored to `sizeof(void*)`.
         * @throws std::invalid_argument If alignment is zero, not a power of 2, or if the pool can't hold at least one effective block.
         * @throws std::bad_alloc If the OS memory allocation fails.
         */
        SlabAllocator(std::size_t block_size, std::size_t pool_size, std::size_t alignment = sizeof(void *));

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
        void *Allocate();

        /**
         * @brief Free an allocated memory block back to the pool.
         *
         * Push the block back to the head of the free list in O(1) time.
         * 
         * Contract:
         * 
         * - `ptr == nullptr` is allowed and is a no-op.
         * 
         * - `ptr` must be a block previously returned by `Allocate()` from this allocator.
         * 
         * - Double free, cross-pool free, or passing a non-block pointer is a contract violation (undefined behavior).
         *
         * @param ptr Pointer to the block to be freed.
         */
        void Free(void *ptr);

        // ---------------------------------------------------------------
        // Disable copy semantics to prevent Double Free and UB when multiple destructors release the same pool.
        // [Ref] Effective C++ Item 14 (Think carefully about copying behavior in resource-managing classes) - Prohibit copying RAII objects.
        SlabAllocator(const SlabAllocator &) = delete;
        SlabAllocator &operator=(const SlabAllocator &) = delete;
        // ---------------------------------------------------------------

    private:
        /**
         * @brief Embedded free list node.
         *
         * [Ref 1] OSTEP Chapter 17.2 (Low-level Mechanisms) - Embedding A Free List.
         *
         * [Ref 2] CSAPP Chapter 9.9.13 (Explicit Free Lists) - LIFO ordering, first-fit placement.
         */
        struct FreeBlock
        {
            FreeBlock *next;
        };

        std::size_t block_size_;
        std::size_t pool_size_;
        std::size_t alignment_;

        /**
         * @brief Pointer to the pool address which requested from OS.
         */
        void *pool_start_;

        /**
         * @brief Store the block which should be allocated first.
         */
        FreeBlock *free_list_head_;
    };
}
// See also: `ADR-0001` (Allocator Contract), `ADR-0003` (Alignment Policy).

#endif