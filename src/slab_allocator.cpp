#include "slab_allocator.h"
#include <algorithm> // for std::max
#include <cstdlib>   // for std::malloc and std::free
#include <stdexcept> // for std::bad_alloc, std::invalid_argument

namespace mcr {
    SlabAllocator::SlabAllocator(std::size_t block_size, std::size_t pool_size) {
        // -----------------------------------------------------------
        // Minimum Block Size Requirement:
        // Ensure block size is large enough to hold FreeBlock pointer.
        // (Minimum size is 8 bytes on 64-bit systems)
        // -----------------------------------------------------------
        block_size = std::max(block_size, sizeof(FreeBlock));

        // -----------------------------------------------------------
        // Memory Block Size Alignment:
        // Align block size to word boundary (8 bytes in 64-bit systems).
        // Unaligned accesses could cause performance penalties.
        // (Like multiple accesses to fetch the whole data.)
        // [Ref] CSAPP Chapter 3.9.3 (Data Alignment)
        // -----------------------------------------------------------
        const std::size_t ALIGNMENT = sizeof(void*);
        // Round up block size to the nearest multiple of ALIGNMENT.
        block_size_ = (block_size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);

        // Calculate how many aligned blocks can fit in the requested pool size.
        std::size_t block_count = pool_size / block_size_;
        if (block_count == 0) {
            throw std::invalid_argument("Pool size must be at least as large as block size.");
        }

        // Recalculate actual pool size based on the number of aligned blocks.
        pool_size_ = block_count * block_size_;

        // Allocate the raw contiguous memory block from OS.
        pool_start_ = std::malloc(pool_size_);
        if (!pool_start_) {
            throw std::bad_alloc();
        }

        // ----------------------------------------------------------
        // Wire Up The Free List:
        // Slice the memory pool into fixed-size blocks and link them.
        // (Set last block's next pointer to nullptr.)
        // (We use char* for pointer arithmetic.)
        // ----------------------------------------------------------
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
        // Return the entire memory pool back to the OS.
        std::free(pool_start_);
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