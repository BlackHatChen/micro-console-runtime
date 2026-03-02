#ifndef MCR_SLAB_MANAGER_H_

#define MCR_SLAB_MANAGER_H_
#include "slab_allocator.h"
#include <cstddef>
#include <array>
#include <memory>

namespace mcr {
    /**
     * @brief Manage multiple size classes of SlabAllocators.
     * 
     * Implements Segregated Free Lists to handle variable-sized allocations.
     * 
     * [Ref] OSTEP Chapter 17 (Free-Space Management) - Segregated Lists.
     */
    class SlabManager {
    public:
        /**
         * @brief Construct with predefined size classes.
         * 
         * It will initialize multiple SlabAllocators (Ex: 16B, 32B, 64B...).
         */
        SlabManager();

        /**
         * @brief RAII will automatically clean up all managed allocators.
         */
        ~SlabManager() = default;

        /**
         * @brief Allocate memory by routing to the best-fit size class.
         * 
         * Decided by requested memory size or alignment (the maximum one).
         * 
         * @param size The requested memory size.
         * @param alignment The requested alignment (Default to a word size).
         * @return void* pointer to the allocated memory, or nullptr if the pool is exhausted 
         * or the required size exceeds the maximum managed class.
         */
        void* Allocate(std::size_t size, std::size_t alignment = sizeof(void*));

        /**
         * @brief Free memory back to the correct size class pool.
         * 
         * Utilizes C++14 Sized Deallocation semantics to avoid metadata headers.
         * The caller provides the size to allow O(1) route back to the correct pool.
         * 
         * @param ptr Pointer to the memory to free.
         * @param size The size originally requested, for routing back to the correct pool.
         */
        void Free(void* ptr, std::size_t size);

        // Disable copy semantics to prevent Double Free of the entire manager.
        SlabManager(const SlabManager&) = delete;
        SlabManager& operator=(const SlabManager&) = delete;

    private:
        // Define the size classes (Powers of 2, from 16 to 1024 bytes).
        static constexpr std::size_t kNumClasses = 7;
        static constexpr std::size_t kMinClassSize = 16; // Data (8-byte) + Address (8-byte)
        static constexpr std::size_t kMaxClassSize = 1024; // Less than a page size (4KB). The slab allocator is for the small size objects.
        
        // Array of unique_ptr to manage the lifecycle of the allocators.
        // [Ref] Effective C++ Item 13 (Use object to manage resources).
        std::array<std::unique_ptr<SlabAllocator>, kNumClasses> allocators_;

        /**
         * @brief Calculate the size class index for a given size.
         * 
         * Maps size to index: 16->0, 32->1, 64->2, 128->3, 256->4, 512->5, 1024->6
         */
        std::size_t GetClassIndex(std::size_t size) const;
    };
}

#endif