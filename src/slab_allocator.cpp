#include "slab_allocator.h"
#include <algorithm> // for std::max
#include <cstdlib>   // for std::malloc and std::free
#include <stdexcept> // for std::bad_alloc, std::invalid_argument

#if defined(_WIN32) || defined(_WIN64)
    #include <malloc.h> // for _aligned_malloc and _aligned_free
#else
    #include <stdlib.h> // for posix_memalign
#endif

namespace mcr {
    SlabAllocator::SlabAllocator(std::size_t block_size, std::size_t pool_size, std::size_t alignment) : alignment_(alignment) {
        // 1. Alignment Validation
        // Ensure alignment is a power of 2.
        if (alignment_ == 0 || (alignment_ & (alignment_ - 1)) != 0) {
            throw std::invalid_argument("Alignment must be a power of 2.");
        }
        // Ensure alignment must be at least a word size.
        alignment_ = std::max(alignment_, sizeof(void*));
        
        // 2. Block Size Adjustment
        // Ensure block size is large enough to hold the embedded FreeBlock pointer.
        block_size = std::max(block_size, sizeof(FreeBlock));
        // Align block size to word boundary (8 bytes in 64-bit systems).
        // Unaligned accesses could cause performance penalties. (Like multiple accesses to fetch the whole data.)
        // [Ref] CSAPP Chapter 3.9.3 (Data Alignment)
        // Round up block size to the nearest multiple of alignment_.
        block_size_ = (block_size + alignment_ - 1) & ~(alignment_ - 1);

        // 3. Calculate how many aligned blocks can fit in the requested pool size.
        std::size_t block_count = pool_size / block_size_;
        if (block_count == 0) {
            throw std::invalid_argument("Pool size must be at least as large as block size.");
        }

        // 4. Pool Size Recalculation
        pool_size_ = block_count * block_size_;

        // 5. Cross Platform Aligned Memory Allocation
        // Handle aligned memory allocation for specific OS API (Windows, Linux, macOS).
#if defined(_WIN32) || defined(_WIN64)
        pool_start_ = _aligned_malloc(pool_size_, alignment_);
        if (!pool_start_) {
            throw std::bad_alloc();
        }
#else
        if (posix_memalign(&pool_start_, alignment_, pool_size_) != 0) {
            throw std::bad_alloc();
        }
#endif

        // 6. Wire Up The Free List
        // Slice the memory pool into fixed-size blocks and link them.
        // (Set last block's next pointer to nullptr.)
        // (We use char* for pointer arithmetic.)
        free_list_head_ = static_cast<FreeBlock*>(pool_start_);
        char* current_byte_ptr = static_cast<char*>(pool_start_);
        for (std::size_t i = 0; i < block_count - 1; i++)
        {
            FreeBlock* current_block = reinterpret_cast<FreeBlock*>(current_byte_ptr);
            FreeBlock* next_block = reinterpret_cast<FreeBlock*>(current_byte_ptr + block_size_);
            current_block->next = next_block;
            current_byte_ptr += block_size_;
        }
        FreeBlock* last_block = reinterpret_cast<FreeBlock*>(current_byte_ptr);
        last_block->next = nullptr;
    }

    SlabAllocator::~SlabAllocator() {
        // Handle aligned memory deallocation for specific OS API (Windows, Linux, macOS).
#if defined(_WIN32) || defined(_WIN64)
        _aligned_free(pool_start_);
#else
        std::free(pool_start_);
#endif
    }

    void* SlabAllocator::Allocate() {
        // If the memory pool is exhausted, return nullptr and do nothing.
        if (free_list_head_ == nullptr) {
            return nullptr;
        }
        
        // O(1) pop a free block from the head of the free list.
        void* allocate_ptr = free_list_head_;
        free_list_head_ = free_list_head_->next;
        return allocate_ptr;
    }

    void SlabAllocator::Free(void* ptr) {
        // If the free pointer is nullptr, do nothing. 
        if (ptr == nullptr) {
            return;
        }

        // O(1) push the block back to the head of the free list.
        FreeBlock* free_block = static_cast<FreeBlock*>(ptr);
        free_block->next = free_list_head_;
        free_list_head_ = free_block;
    }
}