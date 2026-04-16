#ifndef MCR_SLAB_MANAGER_H_

#define MCR_SLAB_MANAGER_H_
#include "slab_allocator.h"
#include <cstddef>
#include <array>
#include <memory>

namespace mcr
{
    /**
     * @brief Manages multiple `SlabAllocator` for power-of-2 size classes.
     *
     * Routes variable-sized allocation requests to segregated size classes.
     *
     * Notes:
     *
     * - Requests are routed to the smallest managed size class that satisfies `max(size, alignment)`.
     * 
     * - `Allocate()` and `Free()` must use the same `(size, alignment)` pair so that deallocation routes back to the same size class.
     * 
     * - Size-class routing is O(1).
     */
    class SlabManager
    {
    public:
        /**
         * @brief Construct the manager and initialize all of the managed per-class allocators.
         * 
         * Initializes one `SlabAllocator` for each size class managed by the current small-object policy.
         */
        SlabManager();

        /**
         * @brief Destroy the manager and release its managed allocators.
         */
        ~SlabManager() = default;

        /**
         * @brief Allocate memory by routing to the smallest satisfying size class.
         *
         * @param size The requested memory size.
         * @param alignment The requested alignment. Must be non-zero and a power of 2. The same alignment must be supplied to `Free()` for symmetric routing.
         * @return Pointer to the allocated memory, or nullptr if the target size class is exhausted or if `max(size, alignment)` exceeds the maximum managed class size.
         * @throws std::invalid_argument If `alignment` is zero or not a power of 2.
         */
        void *Allocate(std::size_t size, std::size_t alignment = sizeof(void *));

        /**
         * @brief Free memory back to the correct size class.
         *
         * `Free()` requires `(size, alignment)` to route back to the correct size class.
         *
         * Contract:
         *
         * - `ptr == nullptr` is allowed and is a no-op.
         *
         * - `size` and `alignment` must match the values used at the allocation site.
         * 
         * - Passing a mismatched `(size, alignment)` pair, a non-owned pointer, a non-block pointer, or double-freeing a block is a contract violation (undefined behavior).
         *
         * @param ptr Pointer to the memory to be freed.
         * @param size The requested size (same value used at the allocation site).
         * @param alignment The requested alignment (same value used at the allocation site).
         */
        void Free(void *ptr, std::size_t size, std::size_t alignment);

        // Disable copy semantics for the manager.
        SlabManager(const SlabManager &) = delete;
        SlabManager &operator=(const SlabManager &) = delete;

    private:
        /**
         * @brief Number of managed size classes.
         */
        static constexpr std::size_t kNumClasses = 7;

        /**
         * @brief Smallest managed size class under the current small-object policy.
         * 
         * Requests below this size are rounded up to the minimum class size.
         */
        static constexpr std::size_t kMinClassSize = 16;

        /**
         * @brief Largest managed size class under the current small-object policy.
         * 
         * Requests above this size fall outside the range handled by `SlabManager`.
         */
        static constexpr std::size_t kMaxClassSize = 1024;

        /**
         * @brief Owns the per-class allocators.
         */
        std::array<std::unique_ptr<SlabAllocator>, kNumClasses> allocators_;

        /**
         * @brief Compute the size class index for a given size.
         *
         * Maps size classes to indices: 16 -> 0, 32 -> 1, 64 -> 2, 128 -> 3, 256 -> 4, 512 -> 5, 1024 -> 6.
         */
        std::size_t GetClassIndex(std::size_t size) const;
    };
}

#endif