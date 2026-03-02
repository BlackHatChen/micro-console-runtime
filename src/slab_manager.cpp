#include "slab_manager.h"
#include <stdexcept>

#if defined(_WIN32) || defined(_WIN64)
    #include <intrin.h>
#endif

namespace mcr {
    SlabManager::SlabManager() {
        // Initialize Segregated Free Lists
        // Sizes: 16, 32, 64, 128, 256, 512, 1024 bytes
        std::size_t current_block_size = kMinClassSize;
        for (std::size_t i = 0; i < kNumClasses; i++) {
            // Pre-allocate 100 blocks for each allocator. 
            std::size_t pool_size = current_block_size * 100;
            // Make alignment equals to block size to prevent False Sharing.
            // [Ref] Effective Modern C++ Item 21 (Prefer std::make_unique and std::make_shared to direct use of new.)
            allocators_[i] = std::make_unique<SlabAllocator>(current_block_size, pool_size, current_block_size);
            current_block_size *= 2;
        }
    }

    std::size_t SlabManager::GetClassIndex(std::size_t size) const {
        // Defensive Checks
        if (size <= kMinClassSize) { // The requested size <= 16: Class 0 allocator.
            return 0;
        }
        if (size > kMaxClassSize) {
            throw std::invalid_argument("Size exceeds maximum managed class size.");
        }

        // O(1) Size Routing (Using CLZ/BSR instructions)
        std::size_t s = size - 1; // Ensure the size wouldn't be pushed into the next class when it's powers of 2.
        unsigned long highest_bit_index = 0;
#if defined(_WIN32) || defined(_WIN64) // MSVC: BSR
    #ifdef _WIN64
        _BitScanReverse64(&highest_bit_index, s);
    #else
        _BitScanReverse(&highest_bit_index, s);
    #endif
#else
        // CLZ: Return the count of the leading zeros.
        // Calculate the index of the highest bit = (64 - 1) - CLZ.
        highest_bit_index = 63 - __builtin_clzll(static_cast<unsigned long long>(s));
#endif

        // Ex. 1: size=17, s=16 (10000 in binary), the index of the highest bit=4, 4 - 3 = 1, Index 1 (32 bytes).
        // Ex. 2: size=33, s=32 (100000 in binary), the index of the highest bit=5, 5 - 3 = 2, Index 2 (64 bytes).
        return static_cast<std::size_t>(highest_bit_index - 3);
    }

    void* SlabManager::Allocate(std::size_t size, std::size_t alignment) {
        if (size > kMaxClassSize) {
            return nullptr;
        }
        
        // Ensure the target block size could satisfy both size and alignment.
        std::size_t target_size = std::max(size, alignment);
        std::size_t class_idx = GetClassIndex(target_size);
        return allocators_[class_idx]->Allocate();
    }

    void SlabManager::Free(void* ptr, std::size_t size) {
        if (!ptr) {
            return;
        }
        
        // Rely on C++14 Sized Deallocation input size to route back to the correct size allocator.
        std::size_t class_idx = GetClassIndex(size);
        allocators_[class_idx]->Free(ptr);
    }
}