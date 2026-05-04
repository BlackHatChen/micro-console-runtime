#include "slab_allocator.h"
#include <algorithm>
#include <cstdlib>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

#if defined(_WIN32) || defined(_WIN64)
#include <malloc.h> // for _aligned_malloc and _aligned_free
#else
#include <stdlib.h> // for posix_memalign
#endif

namespace mcr
{
    SlabAllocator::SlabAllocator(std::size_t block_size, std::size_t pool_size, std::size_t alignment) : alignment_(alignment)
    {
        // Validate the requested alignment and derive the effective alignment.
        if (alignment_ == 0 || (alignment_ & (alignment_ - 1)) != 0)
        {
            throw std::invalid_argument("Alignment must be non-zero and a power of 2.");
        }
        alignment_ = std::max(alignment_, sizeof(void *));

        // Adjust the final block size.
        block_size = std::max(block_size, sizeof(FreeBlock));            // Ensure the block is large enough to hold an embedded free-list node.
        block_size_ = (block_size + alignment_ - 1) & ~(alignment_ - 1); // Round the final block size up to the nearest multiple of alignment_.

        // Compute how many whole blocks fit in the requested pool size.
        std::size_t block_count = pool_size / block_size_;
        if (block_count == 0)
        {
            throw std::invalid_argument("Pool size must be able to hold at least one effective block.");
        }

        // Trim the backing-pool size to a whole-block multiple.
        pool_size_ = block_count * block_size_;

        // Allocate the backing pool.
#if defined(_WIN32) || defined(_WIN64)
        pool_start_ = _aligned_malloc(pool_size_, alignment_);
        if (!pool_start_)
        {
            throw std::bad_alloc();
        }
#else
        if (posix_memalign(&pool_start_, alignment_, pool_size_) != 0)
        {
            throw std::bad_alloc();
        }
#endif

        // Link the free list across the pool blocks.
        free_list_head_ = static_cast<FreeBlock *>(pool_start_);
        std::uintptr_t current_byte_ptr = reinterpret_cast<std::uintptr_t>(pool_start_);
        for (std::size_t i = 0; i < block_count - 1; i++)
        {
            FreeBlock *current_block = reinterpret_cast<FreeBlock *>(current_byte_ptr);
            FreeBlock *next_block = reinterpret_cast<FreeBlock *>(current_byte_ptr + block_size_);
            current_block->next = next_block;
            current_byte_ptr += block_size_;
        }
        FreeBlock *last_block = reinterpret_cast<FreeBlock *>(current_byte_ptr);
        last_block->next = nullptr; // Terminate the free list.
    }

    SlabAllocator::~SlabAllocator()
    {
        // Release the backing pool.
#if defined(_WIN32) || defined(_WIN64)
        _aligned_free(pool_start_);
#else
        std::free(pool_start_);
#endif
    }

    void *SlabAllocator::Allocate()
    {
        // If the allocator is exhausted, return nullptr.
        if (!free_list_head_)
        {
            return nullptr;
        }

        // Pop the head block from the free list.
        void *allocate_ptr = free_list_head_;
        free_list_head_ = free_list_head_->next;
        return allocate_ptr;
    }

    void SlabAllocator::Free(void *ptr)
    {
        // If ptr is nullptr, do nothing.
        if (!ptr)
        {
            return;
        }

        // Push the block back to the free-list head.
        FreeBlock *free_block = static_cast<FreeBlock *>(ptr);
        free_block->next = free_list_head_;
        free_list_head_ = free_block;
    }
}