#ifndef MCR_SLAB_ALLOCATOR_H_

#define MCR_SLAB_ALLOCATOR_H_
#include <cstddef>

namespace mcr
{
    /**
     * @brief A memory allocator consisting of fixed-size blocks.
     *
     * Avoids external fragmentation:
     * 
     * no contiguous free region needs to be larger than one block 
     * because any free block can satisfy a request for one block.
     *
     * Notes:
     *
     * - No per-allocation header is prepended to each block.
     *
     * - Free-list metadata is maintained via an embedded singly-linked free list.
     *
     * - `Allocate()` and `Free()` operate in O(1) time.
     *
     * - Not thread-safe; concurrent use must be synchronized by the caller.
     *
     * - Destroying the allocator invalidates any outstanding pointers returned by `Allocate()`.
     */
    class SlabAllocator
    {
    public:
        /**
         * @brief Construct the allocator and its backing pool.
         * 
         * The effective alignment is `max(requested_alignment, sizeof(void*))`.
         * The final block size is rounded up to that alignment.
         *
         * @param block_size The requested payload size for each block.
         * @param pool_size The requested backing pool size.
         * @param alignment The requested alignment. Must be non-zero and a power of 2.
         * @throws std::invalid_argument If alignment is zero, not a power of 2, or if the pool cannot hold at least one effective block.
         * @throws std::bad_alloc If the backing-pool allocation fails.
         */
        SlabAllocator(std::size_t block_size, std::size_t pool_size, std::size_t alignment = sizeof(void *));

        /**
         * @brief Destroy the allocator and release its backing pool.
         */
        ~SlabAllocator();

        /**
         * @brief Allocate a memory block from the backing pool.
         *
         * @return pointer to the allocated memory, or nullptr if the pool is exhausted.
         */
        void *Allocate();

        /**
         * @brief Return an allocated block to the backing pool.
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
        // Disable copy semantics for the owning allocator.
        SlabAllocator(const SlabAllocator &) = delete;
        SlabAllocator &operator=(const SlabAllocator &) = delete;
        // ---------------------------------------------------------------

    private:
        /**
         * @brief Embedded free list node.
         */
        struct FreeBlock
        {
            FreeBlock *next;
        };

        std::size_t block_size_;
        std::size_t pool_size_;
        std::size_t alignment_;

        /**
         * @brief Start address of the backing pool allocated by the underlying allocator/system.
         */
        void *pool_start_;

        /**
         * @brief Head of the free list; the block to be allocated next.
         */
        FreeBlock *free_list_head_;
    };
}

#endif