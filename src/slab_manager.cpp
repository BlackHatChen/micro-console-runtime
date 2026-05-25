#include "slab_manager.h"
#include <cstddef>
#include <stdexcept>
#include <algorithm>

#if defined(_WIN32) || defined(_WIN64)
#include <intrin.h> // for _BitScanReverse
#endif

namespace mcr
{
    SlabManager::SlabManager()
    {
        std::size_t current_block_size = kMinClassSize; // Start from the smallest managed class size.

        for (std::size_t i = 0; i < kNumClasses; i++)
        {
            std::size_t pool_size = current_block_size * kBlocksPerClass;
            allocators_[i] = std::make_unique<SlabAllocator>(current_block_size, pool_size, current_block_size); // Align each class to its block size.
            current_block_size *= 2;
        }
    }

    std::size_t SlabManager::GetClassIndex(std::size_t size) const
    {
        if (size <= kMinClassSize)
        {
            return 0;
        }
        if (size > kMaxClassSize)
        {
            throw std::invalid_argument("Size exceeds maximum managed class size.");
        }

        // Use `size - 1` so exact powers of 2 stay in their own class.
        // For example, size 32 maps to class 32 instead of class 64.
        std::size_t s = size - 1;
        unsigned long highest_bit_index = 0; // After the bit scan, this is `floor(log2(size - 1))`.
#if defined(_WIN32) || defined(_WIN64)
#ifdef _WIN64
        _BitScanReverse64(&highest_bit_index, s);
#else
        _BitScanReverse(&highest_bit_index, s);
#endif
#else
        highest_bit_index = 63 - __builtin_clzll(static_cast<unsigned long long>(s));
#endif

        // `floor(log2(size - 1)) + 1` gives `ceil(log2(size))` for this size-class mapping.
        // kMinClassSize is 16 = 2^4, so subtract log2(16) = 4 to get the zero-based class index.
        static constexpr unsigned kMinClassLog2 = 4;
        return static_cast<std::size_t>(highest_bit_index + 1 - kMinClassLog2);
    }

    void *SlabManager::Allocate(std::size_t size, std::size_t alignment)
    {
        if (alignment == 0 || (alignment & (alignment - 1)) != 0)
        {
            throw std::invalid_argument("Alignment must be non-zero and a power of 2.");
        }

        std::size_t target_size = std::max(size, alignment); // Ensure the target block size could satisfy both size and alignment.
        if (target_size > kMaxClassSize)
        {
            return nullptr;
        }
        std::size_t class_idx = GetClassIndex(target_size); // Route by `max(size, alignment)`, `Free()` uses the same policy.
        return allocators_[class_idx]->Allocate();
    }

    void SlabManager::Free(void *ptr, std::size_t size, std::size_t alignment)
    {
        if (!ptr)
        {
            return;
        }
        
        std::size_t target_size = std::max(size, alignment);
        std::size_t class_idx = GetClassIndex(target_size); // Route back using the same policy as Allocate().
        allocators_[class_idx]->Free(ptr);
    }
}