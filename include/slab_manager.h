#ifndef MCR_SLAB_MANAGER_H_

#define MCR_SLAB_MANAGER_H_
#include "slab_allocator.h"
#include <cstddef>
#include <array>
#include <memory>

namespace mcr
{
    /**
     * @brief Manage multiple size classes of SlabAllocator.
     *
     * Implement Segregated Free Lists to handle variable-sized allocations.
     *
     * Contract:
     *
     * - O(1) routing: size-class index is computed by a bit-scan mapping (CLZ/BSR), and no linear scans over the class list.
     *
     * - Symmetric policy: both `Allocate`/`Free` route by `max(size, alignment)`,
     *   so they land on the same size-class when the same `(size, alignment)` pair is supplied at both allocation and deallocation sites.
     *
     * [Ref] OSTEP Chapter 17 (Free-Space Management) - Segregated Lists.
     */
    class SlabManager
    {
    public:
        /**
         * @brief Construct with predefined general size classes.
         *
         * It will initialize multiple SlabAllocators (Ex: 16, 32, 64 bytes...).
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
         * @param alignment The requested alignment. Must be non-zero and a power of 2. The same alignment must be supplied to `Free()` for symmetric routing.
         * @return Pointer to the allocated memory address, or nullptr if the target size class is exhausted or if `max(size, alignment)` exceeds the maximum managed class size.
         */
        void *Allocate(std::size_t size, std::size_t alignment = sizeof(void *));

        /**
         * @brief Free memory back to the correct size class.
         *
         * Free requires (size, alignment) to route back in O(1) without per-allocation metadata.
         *
         * Contract:
         *
         * - `ptr == nullptr` is allowed and is a no-op.
         *
         * - `size` and `alignment` must match the values used at allocation site.
         *
         * @param ptr Pointer to the memory to be freed.
         * @param size The requested size (Same value used at allocation site).
         * @param alignment The requested alignment (Same value used at allocation site).
         */
        void Free(void *ptr, std::size_t size, std::size_t alignment);

        // Disable copy semantics to prevent Double Free of the entire manager.
        SlabManager(const SlabManager &) = delete;
        SlabManager &operator=(const SlabManager &) = delete;

    private:
        // Define the size classes (Powers of 2, from 16 to 1024 bytes).
        static constexpr std::size_t kNumClasses = 7;

        // Data (8-byte) + Address (8-byte)
        static constexpr std::size_t kMinClassSize = 16;

        // Less than a page size (4KB). The slab allocator is for the small size objects.
        static constexpr std::size_t kMaxClassSize = 1024;

        // Array of unique_ptr to manage the lifecycle of the allocators.
        // [Ref] Effective C++ Item 13 (Use object to manage resources).
        std::array<std::unique_ptr<SlabAllocator>, kNumClasses> allocators_;

        /**
         * @brief Calculate the size class index for a given size.
         *
         * Maps size to index: 16->0, 32->1, 64->2, 128->3, 256->4, 512->5, 1024->6
         *
         * Implementation: bit-scan mapping (CLZ/BSR) on (size-1) to find the highest set bit, then subtract the shift of the 16-byte class.
         */
        std::size_t GetClassIndex(std::size_t size) const;
    };
}

#endif